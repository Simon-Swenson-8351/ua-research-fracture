#include "continuous_sample_vector_adapter.hpp"

namespace stick_2d_frac {

// Here's what we're working with:
// c_t: top of the camera
// initial stick length
// for each non-root, non-leaf node, fracture location
// p_x: initial x position
// p_y: initial y position
// v_x
// v_y
// angle
// angular_velocity
// for each active stick for each image, the four values representing the observed
//     stick end-points.
// Let's do the first few addresses first, since they're static.

double Continuous_sample_vector_adapter::get(const sample::Continuous_sample * s, size_t idx) const
{
    switch(idx)
    {
    case 0:
        return s->get_camera().get_camera_top();
    case 1:
        return s->get_stick_tree().get_length_info().get_length();
    case 2:
        return s->get_stick_tree().get_hidden_stick_state(0).get_state_element(sample::Hidden_stick_state::SV_X_POSITION);
    case 3:
        return s->get_stick_tree().get_hidden_stick_state(0).get_state_element(sample::Hidden_stick_state::SV_Y_POSITION);
    case 4:
        return s->get_stick_tree().get_hidden_stick_state(0).get_state_element(sample::Hidden_stick_state::SV_X_VELOCITY);
    case 5:
        return s->get_stick_tree().get_hidden_stick_state(0).get_state_element(sample::Hidden_stick_state::SV_Y_VELOCITY);
    case 6:
        return s->get_stick_tree().get_hidden_stick_state(0).get_state_element(sample::Hidden_stick_state::SV_ANGLE);
    case 7:
        return s->get_stick_tree().get_hidden_stick_state(0).get_state_element(sample::Hidden_stick_state::SV_ANGULAR_VELOCITY);
    default:
        idx -= 8; // Need something to point a reference at.
        return s->get_stick_tree().get_nonleaf_node_by_index(idx)->get_left()->get_length_info().get_length();
    }
}

void Continuous_sample_vector_adapter::set(sample::Continuous_sample * s, size_t idx, double val) const
{
    switch(idx)
    {
    case 0:
        s->get_camera().set_camera_top(val);
        break;
    case 1:
        s->set_initial_length(val);
        break;
    case 2:
        s->set_initial_state_element(sample::Hidden_stick_state::SV_X_POSITION, val);
        break;
    case 3:
        s->set_initial_state_element(sample::Hidden_stick_state::SV_Y_POSITION, val);
        break;
    case 4:
        s->set_initial_state_element(sample::Hidden_stick_state::SV_X_VELOCITY, val);
        break;
    case 5:
        s->set_initial_state_element(sample::Hidden_stick_state::SV_Y_VELOCITY, val);
        break;
    case 6:
        s->set_initial_state_element(sample::Hidden_stick_state::SV_ANGLE, val);
        break;
    case 7:
        s->set_initial_state_element(sample::Hidden_stick_state::SV_ANGULAR_VELOCITY, val);
        break;
    default:
        idx -= 8;
        s->set_fracture_location(idx, val);
        break;
    }
}

size_t Continuous_sample_vector_adapter::size(const sample::Continuous_sample * s) const
{
    return 8 + s->get_stick_tree().get_num_nonleaf_nodes();
}

}
