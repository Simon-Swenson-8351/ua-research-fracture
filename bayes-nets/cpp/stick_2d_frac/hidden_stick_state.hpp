#ifndef STICK_STATE_HPP
#define STICK_STATE_HPP

#include <vector>
#include <memory>

#include <boost/serialization/access.hpp>

#include <m_cpp/m_matrix.h>

#include "stick_camera.hpp"
#include "stick_length_info.hpp"

namespace stick_2d_frac { namespace sample {

class Hidden_stick_state {
    friend class boost::serialization::access;
public:
    enum State_variable {
        SV_X_POSITION = 0,
        SV_Y_POSITION = 1,
        SV_X_VELOCITY = 2,
        SV_Y_VELOCITY = 3,
        SV_Y_ACCELERATION = 4,
        SV_ANGLE = 5,
        SV_ANGULAR_VELOCITY = 6
    };
    enum Fragment_side {
        FS_LEFT,
        FS_RIGHT
    };

    // To get around the fact that C++ doesn't have good static initialization support.
    // Couldn't initialize a kjb::Matrix as a one-liner with a multi-dimensional array,
    // so we have to do this instead.
    class Static {
    public:
        Static();
        const kjb::Matrix & get_state_transition() const { return state_transition_; }
    private:
        kjb::Matrix state_transition_;
    };
    Static STATIC;

    Hidden_stick_state(
            const kjb::Vector & data,
            const Stick_length_info & local_endpoints_homo,
            const Camera & c) :
        data_(data)
    {
        update_data_dependents(local_endpoints_homo.get_local_endpoints_homo(), c);
    }
    // Initializes a HiddenStickState from a given HiddenStickState, after num_steps
    // frames, assuming no fractures had happened.
    Hidden_stick_state(
            const Hidden_stick_state & prev_stick_state,
            const Stick_length_info & local_endpoints_homo,
            const Camera & c,
            int num_steps = 1);
    // Initializes a HiddenStickState after a fracture of prev_stick_state happens at the given frac_loc.
    // Our state depends on whether we're the left or right fragment of this new fracture.
    Hidden_stick_state(
            const Hidden_stick_state & parent_prev_state,
            const Stick_length_info & parent_len,
            const Camera & c,
            double frac_loc,
            Fragment_side fs);

    double get_state_element(State_variable sv) const { return data_[sv]; }
    const kjb::Vector & get_state() const { return data_; }
    const kjb::Matrix & get_world_endpoints_homo() const { return world_endpoints_homo_; }
    const kjb::Matrix & get_image_endpoints_homo() const { return image_endpoints_homo_; }

    void set_state_element(
            State_variable sv,
            double val,
            const kjb::Matrix & local_endpoints_homo,
            const Camera & c)
    {
        data_[sv] = val;
        switch(sv) {
        case SV_X_POSITION:
        case SV_Y_POSITION:
        case SV_ANGLE:
            update_data_dependents(local_endpoints_homo, c);
            break;
        case SV_X_VELOCITY:
        case SV_Y_VELOCITY:
        case SV_Y_ACCELERATION:
        case SV_ANGULAR_VELOCITY:
            break;
        default:
            throw "unknown state variable";
        }
    }
    void set_data(
            const kjb::Vector & d,
            const kjb::Matrix & local_endpoints_homo,
            const Camera & c)
    {
        data_ = d;
        update_data_dependents(local_endpoints_homo, c);
    }

    // This is called on object construction, whenever the data_ vector is set to a
    // new value, or when certain values in the data_ vector are updated to also
    // update the end point matrices.
    // It can also be called manually if, for example, the stick's length, and thus its local
    // endpoints, has changed.
    void update_data_dependents(const kjb::Matrix & local_endpoints_homo, const Camera & c);


    double log_prior(const Camera & c, bool initial_state = true) const
    {
        if(initial_state)
        {
            return kjb::pdf(*prob::new_state_0_p_x_dist(Camera::CAMERA_LEFT, c.get_camera_right()), get_state_element(SV_X_POSITION))
                    + kjb::pdf(*prob::new_state_0_p_y_dist(c.get_camera_top(), Camera::CAMERA_BOTTOM), get_state_element(SV_Y_POSITION));
        }
        else {
            return 0.0;
        }
    }

    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & data_ & world_endpoints_homo_ & image_endpoints_homo_;
    }
private:

    // Returns a newly-created vector that represents a modifier to
    // the parent's stick state (for fracture).
    std::unique_ptr<kjb::Vector> calculate_new_fracture_addend() const;

    // Represented internally as a 3x3 matrix, where the row denotes the derivative level:
    // position, velocity or acceleration and the column denotes the dimension: x, y,
    // angular.
    kjb::Vector data_;
    kjb::Matrix world_endpoints_homo_;
    kjb::Matrix image_endpoints_homo_;
};

}}

#endif // STICK_STATE_HPP
