#ifndef HIDDEN_STATE_HPP
#define HIDDEN_STATE_HPP

#include <boost/serialization/access.hpp>

#include <m_cpp/m_matrix_d.h>
#include <m_cpp/m_vector_d.h>

#include "camera.hpp"
#include "block_geom.hpp"
#include "fracture_rvs.hpp"

namespace fracture { namespace block_2d {

// types
enum State_variable
{
    SV_X_POSITION,
    SV_Y_POSITION,
    SV_X_VELOCITY,
    SV_Y_VELOCITY,
    SV_Y_ACCELERATION,
    SV_ANGLE,
    SV_ANGULAR_VELOCITY,

    // DO NOT ADD ENTRIES BELOW THIS LINE
    SV_COUNT
};

class Hidden_state
{
    friend class boost::serialization::access;
public:
    static constexpr double GRAVITY = -9.8;
    static const int STATE_VARIABLE_SIZE = 7;
    static const kjb::Matrix_d<SV_COUNT, SV_COUNT> STATE_TRANS_MATRIX;

    // required for serialization/deserialization
    Hidden_state() {}

    // Used to construct the root fragment's initial state
    Hidden_state(const Camera & c, const Block_geom & bg, double x, double y) :
            state_vec_(0.0)
    {
        state_vec_[SV_X_POSITION] = x;
        state_vec_[SV_Y_POSITION] = y;
        state_vec_[SV_Y_ACCELERATION] = GRAVITY / (c.get_frames_per_second() * c.get_frames_per_second());
        init_instance_vars(c, bg);
    }

    // Used to construct a state from the deterministic simulation. No fracture, 
    // just simulate one frame
    Hidden_state(const Camera & c, const Block_geom & bg, const Hidden_state & prev_state) :
            state_vec_(STATE_TRANS_MATRIX * prev_state.state_vec_)
    {
        init_instance_vars(c, bg);
    }

    // Nudges the state by the given offsets, then simulates deterministically 
    // for one frame.
    Hidden_state(const Camera & c, const Block_geom & bg, const Hidden_state & prev_state, double x_offset, double x_vel_offset, double ang_vel_offset) :
            state_vec_(prev_state.state_vec_)
    {
        state_vec_[SV_X_POSITION] += x_offset;
        state_vec_[SV_X_VELOCITY] += x_vel_offset;
        state_vec_[SV_ANGULAR_VELOCITY] += ang_vel_offset;
        state_vec_ = STATE_TRANS_MATRIX * state_vec_;
        init_instance_vars(c, bg);
    }

    double get(State_variable sv) const { return state_vec_[sv]; }

    const kjb::Matrix_d<3,3> & get_local_to_world_trans() const { return local_to_world_trans_; }
    const kjb::Matrix_d<3,4> & get_world_polygon() const { return world_polygon_; }
    const kjb::Vector_d<3> & get_world_center_of_mass() const { return world_center_of_mass_; }
    const kjb::Matrix_d<3,4> & get_image_polygon() const { return image_polygon_; }
    const kjb::Vector_d<3> & get_image_center_of_mass() const { return image_center_of_mass_; }

    void update_projection(const Camera & c)
    {
        init_image_endpoints_actual(c);
    }

    bool operator==(const Hidden_state &other) const;

    void recalculate_initial_parent_state_values(const Camera &c, const Initial_block_rvs & ibr, const Block_geom &my_geom);
    void recalculate_no_fracture_values(const Camera &c, const Block_geom &my_geom, const Hidden_state &prev_state);
    void recalculate_fracture_values(const Camera &c, const Block_geom &parent_geom, const Hidden_state &parent_state, const Block_geom &my_geom, const Fracture_rvs &fr, Block_geom::Fragment_side fs);
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    }
private:

    // Requires a valid state_vec_ to be set.
    void init_instance_vars(const Camera & c, const Block_geom & bg)
    {
        init_local_to_world_trans();
        init_world_endpoints(bg);
        init_world_center_of_mass();
        init_image_endpoints_actual(c);
        init_image_center_of_mass(c);
    }

    // Assumes state_vec_ has been properly initialized
    void init_local_to_world_trans()
    {
        local_to_world_trans_(0, 0) = std::cos(state_vec_[SV_ANGLE]); local_to_world_trans_(0, 1) = -std::sin(state_vec_[SV_ANGLE]); local_to_world_trans_(0, 2) = state_vec_[SV_X_POSITION];
        local_to_world_trans_(1, 0) = std::sin(state_vec_[SV_ANGLE]); local_to_world_trans_(1, 1) =  std::cos(state_vec_[SV_ANGLE]); local_to_world_trans_(1, 2) = state_vec_[SV_Y_POSITION];
        local_to_world_trans_(2, 0) =                            0.0; local_to_world_trans_(2, 1) =                             0.0; local_to_world_trans_(2, 2) =                       1.0;
    }

    // Assumes local_to_world_trans_ has been properly initialized
    void init_world_endpoints(const Block_geom & bg)
    {
        world_polygon_ = local_to_world_trans_ * bg.get_local_endpoints();
    }

    void init_world_center_of_mass()
    {
        world_center_of_mass_[0] = state_vec_[SV_X_POSITION];
        world_center_of_mass_[1] = state_vec_[SV_Y_POSITION];
        world_center_of_mass_[2] = 1.0;
    }

    // Assumes world_endpoints_ has been properly initialized
    void init_image_endpoints_actual(const Camera & c)
    {
        image_polygon_ = c.get_camera_matrix() * world_polygon_;
    }

    void init_image_center_of_mass(const Camera & c)
    {
        image_center_of_mass_ = c.get_camera_matrix() * world_center_of_mass_;
    }

    // calculated from the initial block rvs for the parent's state
    // calculated from previous state vector if no fracture.
    // calculated from the parent's previous state vector, the parent's geometry,
    // the fracture momentum sample, our fragment side, and the camera's fps.
    kjb::Vector_d<SV_COUNT> state_vec_;
    // calculated from state vector
    kjb::Matrix_d<3,3> local_to_world_trans_;
    // calculated from local geometry and local-world transformation
    kjb::Matrix_d<3,4> world_polygon_;
    // calculated from state_vec_ and local-world transformation
    kjb::Vector_d<3> world_center_of_mass_;
    // calculated from world polygon and camera matrix
    kjb::Matrix_d<3,4> image_polygon_;
    // calculated from world center of mass and camera matrix
    kjb::Vector_d<3> image_center_of_mass_;
};

}}

#endif // HIDDEN_STATE_HPP
