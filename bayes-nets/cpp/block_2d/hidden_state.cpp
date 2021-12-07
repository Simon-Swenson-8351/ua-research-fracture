#include <m_cpp/m_matrix_d.h>
// For who knows why m_matrix_d.impl.h is not included in m_matrix_d.h
#include <m_cpp/m_matrix_d.impl.h>

#include "hidden_state.hpp"

namespace fracture { namespace block_2d {

static kjb::Matrix_d<SV_COUNT, SV_COUNT> init_trans_matrix()
{
    kjb::Matrix_d<7,7> r = kjb::Matrix_d<7,7>(0.0);
    r(SV_X_POSITION, SV_X_POSITION) = 1.0;
    r(SV_X_POSITION, SV_X_VELOCITY) = 1.0;

    r(SV_Y_POSITION, SV_Y_POSITION) = 1.0;
    r(SV_Y_POSITION, SV_Y_VELOCITY) = 1.0;

    r(SV_X_VELOCITY, SV_X_VELOCITY) = 1.0;

    r(SV_Y_VELOCITY, SV_Y_VELOCITY) = 1.0;
    r(SV_Y_VELOCITY, SV_Y_ACCELERATION) = 1.0;

    r(SV_Y_ACCELERATION, SV_Y_ACCELERATION) = 1.0;

    r(SV_ANGLE, SV_ANGLE) = 1.0;
    r(SV_ANGLE, SV_ANGULAR_VELOCITY) = 1.0;

    r(SV_ANGULAR_VELOCITY, SV_ANGULAR_VELOCITY) = 1.0;
    return r;
}

const kjb::Matrix_d<SV_COUNT, SV_COUNT> Hidden_state::STATE_TRANS_MATRIX = init_trans_matrix();

bool Hidden_state::operator==(const Hidden_state &other) const
{
    return (state_vec_ == other.state_vec_)
            && (local_to_world_trans_ == other.local_to_world_trans_)
            && (world_polygon_ == other.world_polygon_)
            && (world_center_of_mass_ == other.world_center_of_mass_)
            && (image_polygon_ == other.image_polygon_)
            && (image_center_of_mass_ == other.image_center_of_mass_);
}

void Hidden_state::recalculate_initial_parent_state_values(const Camera &c, const Initial_block_rvs & ibr, const Block_geom &my_geom)
{
    state_vec_.fill(0.0);
    state_vec_[SV_X_POSITION] = ibr.get_initial_x();
    state_vec_[SV_Y_POSITION] = ibr.get_initial_y();
    state_vec_[SV_Y_ACCELERATION] = GRAVITY / (c.get_frames_per_second() * c.get_frames_per_second());
    init_instance_vars(c, my_geom);
}

void Hidden_state::recalculate_no_fracture_values(const Camera &c, const Block_geom &my_geom, const Hidden_state &prev_state)
{
    state_vec_ = STATE_TRANS_MATRIX * prev_state.state_vec_;
    init_instance_vars(c, my_geom);
}

void Hidden_state::recalculate_fracture_values(const Camera &c, const Block_geom &parent_geom, const Hidden_state &parent_state, const Block_geom &my_geom, const Fracture_rvs &fr, Block_geom::Fragment_side fs)
{
    double x_mom;
    double ang_mom;
    switch(fs)
    {
    case Block_geom::FS_LEFT:
        x_mom = fr.get_left_x_momentum();
        ang_mom = fr.get_left_angular_momentum();
        break;
    case Block_geom::FS_RIGHT:
        x_mom = fr.get_right_x_momentum();
        ang_mom = fr.get_right_angular_momentum();
        break;
    default:
         throw util::Unhandled_enum_value_exception(); //util::err_str(__FILE__, __LINE__);
    }
    state_vec_ = parent_state.state_vec_;
    state_vec_[SV_X_POSITION] += my_geom.fracture_x_offset(parent_geom, fs);
    state_vec_[SV_X_VELOCITY] += my_geom.momentum_to_velocity(x_mom) / c.get_frames_per_second();
    state_vec_[SV_ANGULAR_VELOCITY] += my_geom.momentum_to_velocity(ang_mom) / c.get_frames_per_second();
    state_vec_ = STATE_TRANS_MATRIX * state_vec_;
    init_instance_vars(c, my_geom);
}

}}
