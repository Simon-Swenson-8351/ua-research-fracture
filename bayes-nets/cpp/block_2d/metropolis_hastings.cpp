#include <boost/math/constants/constants.hpp>

#include <prob_cpp/prob_sample.h>

#include "metropolis_hastings.hpp"
#include "config.hpp"
#include "util.hpp"

namespace fracture { namespace block_2d {

static const double STD_MULTIPLIER = 0.01;

// Basically, use the expected value of the std of the associated hidden variables,
// then multiply that by the STD_MULTIPLIER.
const std::vector<double> Metropolis_hastings_resampler::DEFAULT_STDS = {
    // c_t
    STD_MULTIPLIER * Camera::C_T_STD,
    // init_x
    STD_MULTIPLIER * Initial_block_rvs::calc_init_x_std(Camera::CAMERA_LEFT, (4.0 / 3.0) * Camera::C_T_MEAN),
    // init_y
    STD_MULTIPLIER * Initial_block_rvs::calc_init_y_std(Camera::C_T_MEAN, Camera::CAMERA_BOTTOM),
    // init_width
    STD_MULTIPLIER * Initial_block_rvs::INIT_WIDTH_STD,
    // init_height
    STD_MULTIPLIER * Initial_block_rvs::INIT_HEIGHT_STD,
    // frac_loc
    STD_MULTIPLIER * Fracture_rvs::calc_frac_loc_std(Initial_block_rvs::INIT_WIDTH_MEAN),
    // right_x_momentum
    STD_MULTIPLIER * Fracture_rvs::R_X_MOMENTUM_STD,
    // left_angular_momentum
    STD_MULTIPLIER * Fracture_rvs::L_ANGULAR_MOMENTUM_STD
};

Metropolis_hastings_resampler::Inference_resample_result Metropolis_hastings_resampler::resample_once()
{
    Inference_resample_result r;

    static kjb::Uniform_distribution acceptance_test = kjb::Uniform_distribution();
    if(cur_iter_ >= num_resamples_) throw util::Index_oob_exception(); //util::err_str(__FILE__, __LINE__);

    Sample *new_sample = new Sample(*cur_sample_);
    double uniform = kjb::sample(acceptance_test);
    double log_uniform = std::log(uniform);

    for(unsigned i = 0; i < sva_.size(new_sample); i++)
    {
        // The new sampled value could violate our priors. In that case, an exception is
        // thrown.
        try
        {
            switch(i)
            {
            case RI_L_ANG_MOM:
                if(!try_resample_l_ang_mom(*new_sample)) goto cleanup_rejected;
                break;
            case RI_R_X_MOM:
                if(!try_resample_r_x_mom(*new_sample)) goto cleanup_rejected;
                break;
            default:
                sva_.set(new_sample, i, sva_.get(new_sample, i) + kjb::sample(resample_dists_[i]));
                break;
            }
        }
        catch(const prob::No_support_exception &e)
        {
            goto cleanup_rejected;
        }
    }
    double new_log_prob;
    // If some of the calculated variables are cached rather than calculated at call time, they may
    // not be recalculated until here. Thus, our priors may actually be violated in this call to
    // log_prob.
    try
    {
        new_log_prob = new_sample->log_prob();
    }
    catch(const prob::No_support_exception &e)
    {
        goto cleanup_rejected;
    }

    // Acceptance is basically how much worse our new sample x', is, which
    // can be represented as a ratio, p(x')/p(x).
    // This gives us a number between 0-1. Lower numbers indicate worse
    // samples, so we want to test a uniform random number generator, and
    // only accept if that value is < the ratio.
    // In log space, this becomes a ~ Uniform(0,1) < log(p(x')) - log(p(x)).
    if(new_log_prob > cur_log_prob_
            || log_uniform < (1/as_->get_temperature(cur_iter_)) * (new_log_prob - cur_log_prob_))
    {
        if(new_log_prob < cur_log_prob_)
        {
            std::cout << "---WARNINIG: NEW SAMPLE WITH LOWER PROBABILITY---" << "\n";
            std::cout << "             uniform: " << uniform << "\n";
            std::cout << "        log(uniform): " << log_uniform << "\n";
            std::cout << "           log(prev): " << cur_log_prob_ << "\n";
            std::cout << "            log(cur): " << new_log_prob << "\n";
            std::cout << "log(cur) - log(prev): " << (new_log_prob - cur_log_prob_) << "\n";
        }
        cur_sample_ = new_sample;
        cur_log_prob_ = new_log_prob;
        r = IRR_ACCEPTED;
        goto cleanup_common;
    }
    else
    {
        goto cleanup_rejected;
    }
    goto cleanup_common;
cleanup_rejected:
    delete new_sample;
    r = IRR_REJECTED;
cleanup_common:
    saved_samples_.samples_.push_back(cur_sample_);
    cur_iter_++;
    return r;
}

bool Metropolis_hastings_resampler::try_resample_l_ang_mom(Sample & s)
{
    // do the resample
    double left_ang_vel_in_seconds_new;
    double left_ang_vel_in_frames_new;
    double left_ang_mom_new;
    kjb::Normal_distribution left_ang_vel_dist;
    switch(mrs_)
    {
    case MRS_MOMENTUM:
        sva_.set(
                    &s,
                    RI_L_ANG_MOM,
                    sva_.get(&s, RI_L_ANG_MOM) + kjb::sample(resample_dists_[RI_L_ANG_MOM])
        );
        left_ang_vel_in_frames_new = s.get_left_block().get_state(1).get_hidden_state().get(SV_ANGULAR_VELOCITY);
        left_ang_vel_in_seconds_new = left_ang_vel_in_frames_new * s.get_camera().get_frames_per_second();
        break;
    case MRS_VELOCITY:
        // at this point, resample_dists_[mrs_] should be in terms of velocity already (see constructor).
        left_ang_vel_in_frames_new = s.get_left_block().get_state(1).get_hidden_state().get(SV_ANGULAR_VELOCITY) + kjb::sample(resample_dists_[RI_L_ANG_MOM]);
        left_ang_vel_in_seconds_new = left_ang_vel_in_frames_new * s.get_camera().get_frames_per_second();
        left_ang_mom_new = left_ang_vel_in_seconds_new * s.get_left_block().get_local_geometry().get_volume();
        sva_.set(&s, RI_L_ANG_MOM, left_ang_mom_new);
        break;
    default:
        throw util::Unhandled_enum_value_exception();
        break;
    }

    // check if new sample should be rejected
    if(constrain_ang_vel_)
    {
        double right_ang_vel_in_frames_new = s.get_right_block().get_state(1).get_hidden_state().get(SV_ANGULAR_VELOCITY);
        using namespace boost::math::constants;
        // for the very bad local optimum (20191114)
        // discovered that the block was oscillating at a rate of slightly higher than pi.
        // blocks are symmetric vertically and horizontally, so it may make more sense that
        // the minimum is at pi rather than 2pi. however, the end point correspondences are
        // given, so not exactly sure why this is a local minimum. every other frame would
        // have much worse probability. If we don't want it to get close to that pi condition,
        // try cutting it off at a slightly lower value.
        if(left_ang_vel_in_frames_new > 0.5 * pi<double>() || right_ang_vel_in_frames_new < -0.5 * pi<double>())
        {
            return false;
        }
    }
    return true;
}

bool Metropolis_hastings_resampler::try_resample_r_x_mom(Sample & s)
{
    double right_x_vel_in_frames_new;
    double right_x_vel_in_seconds_new;
    double right_x_mom_new;
    kjb::Normal_distribution right_x_vel_dist;
    switch(mrs_)
    {
    case MRS_MOMENTUM:
        sva_.set(&s, RI_R_X_MOM, sva_.get(&s, RI_R_X_MOM) + kjb::sample(resample_dists_[RI_R_X_MOM]));
        break;
    case MRS_VELOCITY:
        right_x_vel_in_frames_new = s.get_right_block().get_state(1).get_hidden_state().get(SV_X_VELOCITY) + kjb::sample(right_x_vel_dist);
        right_x_vel_in_seconds_new = right_x_vel_in_frames_new * s.get_camera().get_frames_per_second();
        right_x_mom_new = right_x_vel_in_seconds_new * s.get_right_block().get_local_geometry().get_volume();
        sva_.set(&s, RI_R_X_MOM, right_x_mom_new);
        break;
    default:
        throw util::Unhandled_enum_value_exception();
        break;
    }
    return true;
}

}}
