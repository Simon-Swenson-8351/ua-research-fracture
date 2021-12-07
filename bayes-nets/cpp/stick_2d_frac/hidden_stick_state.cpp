#include <cmath>

#include "hidden_stick_state.hpp"

namespace stick_2d_frac { namespace sample {

Hidden_stick_state::Static::Static() : state_transition_(7, 7)
{
    state_transition_(SV_X_POSITION, SV_X_POSITION) = 1;
    state_transition_(SV_X_POSITION, SV_X_VELOCITY) = 1;

    state_transition_(SV_Y_POSITION, SV_Y_POSITION) = 1;
    state_transition_(SV_Y_POSITION, SV_Y_VELOCITY) = 1;

    state_transition_(SV_X_VELOCITY, SV_X_VELOCITY) = 1;

    state_transition_(SV_Y_VELOCITY, SV_Y_VELOCITY) = 1;
    state_transition_(SV_Y_VELOCITY, SV_Y_ACCELERATION) = 1;

    state_transition_(SV_Y_ACCELERATION, SV_Y_ACCELERATION) = 1;

    state_transition_(SV_ANGLE, SV_ANGLE) = 1;
    state_transition_(SV_ANGLE, SV_ANGULAR_VELOCITY) = 1;

    state_transition_(SV_ANGULAR_VELOCITY, SV_ANGULAR_VELOCITY) = 1;
}

Hidden_stick_state::Hidden_stick_state(
        const Hidden_stick_state &prev_stick_state,
        const Stick_length_info & local_endpoints_homo,
        const Camera & c,
        int num_steps) :
    data_(7, 0.0)
{
    kjb::Matrix t = kjb::create_identity_matrix(7);
    // Matrix exponentiation. We should use square and multiply algorithm for
    // better efficiency, but I think that since we must calculate the value
    // for each time step, sequential is okay for now.
    for(int i(0); i < num_steps; i++) {
        t = t * STATIC.get_state_transition();
    }
    data_ = t * prev_stick_state.data_;
    update_data_dependents(local_endpoints_homo.get_local_endpoints_homo(), c);
}

Hidden_stick_state::Hidden_stick_state(
        const Hidden_stick_state & parent_prev_state,
        const Stick_length_info & parent_len,
        const Camera & c,
        double frac_loc,
        Fragment_side fs)
{
    std::unique_ptr<kjb::Vector> ad(parent_prev_state.calculate_new_fracture_addend());
    Stick_length_info my_len(0.0);
    double offset;
    switch(fs)
    {
    case FS_LEFT:
        my_len = Stick_length_info(parent_len.get_length() - frac_loc);
        offset = -0.5 * my_len.get_length();
        break;
    case FS_RIGHT:
        my_len = Stick_length_info(frac_loc);
        offset = 0.5 * my_len.get_length();
        break;
    default:
        throw "unknown fracture side";
    }
    data_ = STATIC.get_state_transition() * (parent_prev_state.data_ + offset * (*ad));
    update_data_dependents(my_len.get_local_endpoints_homo(), c);
}

void Hidden_stick_state::update_data_dependents(const kjb::Matrix & local_endpoints_homo, const Camera & c)
{
    kjb::Matrix world_transformation_matrix(3,3);
    world_transformation_matrix(0, 0) = std::cos(data_[SV_ANGLE]); world_transformation_matrix(0, 1) = -std::sin(data_[SV_ANGLE]); world_transformation_matrix(0, 2) = data_[SV_X_POSITION];
    world_transformation_matrix(1, 0) = std::sin(data_[SV_ANGLE]); world_transformation_matrix(1, 1) =  std::cos(data_[SV_ANGLE]); world_transformation_matrix(1, 2) = data_[SV_Y_POSITION];
    world_transformation_matrix(2, 0) =                       0.0; world_transformation_matrix(2, 1) =                        0.0; world_transformation_matrix(2, 2) =                  1.0;
    world_endpoints_homo_ = world_transformation_matrix * local_endpoints_homo;
    image_endpoints_homo_ = c.get_camera_matrix() * world_endpoints_homo_;
}

std::unique_ptr<kjb::Vector> Hidden_stick_state::calculate_new_fracture_addend() const {
    double d[7];
    d[SV_X_POSITION] = std::cos(data_[SV_ANGLE]);
    d[SV_Y_POSITION] = std::sin(data_[SV_ANGLE]);
    d[SV_X_VELOCITY] = std::cos(data_[SV_ANGLE] + data_[SV_ANGULAR_VELOCITY]) - std::cos(data_[SV_ANGLE]);
    d[SV_Y_VELOCITY] = std::sin(data_[SV_ANGLE] + data_[SV_ANGULAR_VELOCITY]) - std::sin(data_[SV_ANGLE]);
    d[SV_Y_ACCELERATION] = 0;
    d[SV_ANGLE] = 0;
    d[SV_ANGULAR_VELOCITY] = 0;
    return std::unique_ptr<kjb::Vector>(new kjb::Vector(7, d));
}

}}
