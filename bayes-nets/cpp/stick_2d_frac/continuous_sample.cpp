#include <memory>

#include <m_cpp/m_vector.h>

#include "continuous_sample.hpp"
#include "prob.hpp"
#include "hidden_stick_state.hpp"

namespace stick_2d_frac { namespace sample {

void Continuous_sample::set_initial_length(double val) {
    stick_tree_.set_length(val, cam_);
}

void Continuous_sample::set_fracture_location(size_t idx, double val)
{
    stick_tree_.get_nonleaf_node_by_index(idx)->set_fracture_location(idx, val, cam_);
}

void Continuous_sample::set_initial_state_element(sample::Hidden_stick_state::State_variable sv, double val)
{
    stick_tree_.set_initial_state_element(sv, val, cam_);
}

double Continuous_sample::sample_len_0(const Camera & c)
{
    return prob::sample(*(prob::new_len_0_dist(c.get_camera_width(), c.get_camera_height())));
}

std::unique_ptr<Hidden_stick_state> Continuous_sample::sample_s_0(const Stick_length_info & sli, const Camera & c)
{
    // Use a vector instead of an array initialization here so we can use
    // the enum instead of raw indices.
    double temp_array[7] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    // For some reason the constructor kjb::Vector(int, double) was being interpreted as
    // the template constructor with InputIterator_
    kjb::Vector temp(7, temp_array);
    temp.zero_out();
    temp[Hidden_stick_state::SV_X_POSITION] = kjb::sample(
                *(prob::new_state_0_p_x_dist(
                      Camera::CAMERA_LEFT,
                      c.get_camera_right())));
    temp[Hidden_stick_state::SV_Y_POSITION] = kjb::sample(
                *(prob::new_state_0_p_y_dist(
                      c.get_camera_top(),
                      Camera::CAMERA_BOTTOM)));
    temp[Hidden_stick_state::SV_X_VELOCITY] = kjb::sample(prob::STATE_0_V_X_IN_M_S_DIST) / c.get_frames_per_second();
    temp[Hidden_stick_state::SV_Y_VELOCITY] = kjb::sample(prob::STATE_0_V_Y_IN_M_S_DIST) / c.get_frames_per_second();
    temp[Hidden_stick_state::SV_Y_ACCELERATION] = A_Y_IN_M_S_SQ / (c.get_frames_per_second() * c.get_frames_per_second());
    temp[Hidden_stick_state::SV_ANGLE] = kjb::sample(prob::STATE_0_ANG_DIST);
    temp[Hidden_stick_state::SV_ANGULAR_VELOCITY] = kjb::sample(prob::STATE_0_ANG_VEL_IN_R_S_DIST) / c.get_frames_per_second();
    return std::unique_ptr<Hidden_stick_state>(new Hidden_stick_state(temp, sli, c));
}

}}
