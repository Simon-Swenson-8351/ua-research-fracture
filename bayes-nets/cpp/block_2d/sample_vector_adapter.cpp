#include "sample_vector_adapter.hpp"
#include "util.hpp"

namespace fracture { namespace block_2d {

double Sample_vector_adapter::get(const Sample * s, size_t idx) const
{
    if(!s) throw std::exception(); //util::err_str(__FILE__, __LINE__);
    switch(idx)
    {
    case RI_C_T:
        return s->get_camera().get_camera_top();
        break;
    case RI_INIT_X:
        return s->get_initial_block_rvs().get_initial_x();
        break;
    case RI_INIT_Y:
        return s->get_initial_block_rvs().get_initial_y();
        break;
    case RI_INIT_W:
        return s->get_initial_block_rvs().get_initial_width();
        break;
    case RI_INIT_H:
        return s->get_initial_block_rvs().get_initial_height();
        break;
    case RI_FRAC_LOC:
        return s->get_fracture_rvs().get_fracture_location();
        break;
    case RI_R_X_MOM:
        return s->get_fracture_rvs().get_right_x_momentum();
        break;
    case RI_L_ANG_MOM:
        return s->get_fracture_rvs().get_left_angular_momentum();
        break;
    default:
        throw util::Unhandled_enum_value_exception(); //util::err_str(__FILE__, __LINE__);
    }
}

void Sample_vector_adapter::set(Sample * s, size_t idx, double val) const
{
    if(!s) throw std::exception(); //util::err_str(__FILE__, __LINE__);
    switch(idx)
    {
    case RI_C_T:
        return s->set_camera_top(val);
        break;
    case RI_INIT_X:
        return s->set_block_initial_x(val);
        break;
    case RI_INIT_Y:
        return s->set_block_initial_y(val);
        break;
    case RI_INIT_W:
        return s->set_block_initial_width(val);
        break;
    case RI_INIT_H:
        return s->set_block_initial_height(val);
        break;
    case RI_FRAC_LOC:
        return s->set_fracture_location(val);
        break;
    case RI_R_X_MOM:
        return s->set_right_x_momentum(val);
        break;
    case RI_L_ANG_MOM:
        return s->set_left_angular_momentum(val);
        break;
    default:
        throw util::Unhandled_enum_value_exception(); //util::err_str(__FILE__, __LINE__);
    }
}

double sample_log_prob(const Sample & s)
{
    return s.log_prob();
}

kjb::Vector sample_log_gradient(const Sample & s)
{
    Sample_vector_adapter csva;
    // TODO figure out how to define the deltas
    std::vector<double> deltas(csva.size(&s), 0.0000000000001);
    return kjb::gradient_ffd<
            double(*)(const Sample &),
            Sample,
            Sample_vector_adapter>(
                    sample_log_prob,
                    s,
                    deltas,
                    csva
            );
}

}}
