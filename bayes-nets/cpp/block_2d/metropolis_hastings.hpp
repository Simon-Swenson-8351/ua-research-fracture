#ifndef METROPOLIS_HASTINGS_HPP
#define METROPOLIS_HASTINGS_HPP

#include <vector>
#include <memory>

#include <prob_cpp/prob_distribution.h>

#include "sample.hpp"
#include "sample_vector_adapter.hpp"
#include "config.hpp"
#include "annealing_schedule.hpp"

namespace fracture
{
namespace block_2d
{

class Metropolis_hastings_resampler
{
// types
public:
    enum Inference_resample_result
    {
        IRR_ACCEPTED,
        IRR_REJECTED,

        // ADD NEW ELEMENTS ABOVE THIS
        IRR_COUNT
    };
    enum Movement_resampling_strategy
    {
        MRS_MOMENTUM,
        MRS_VELOCITY,

        // ADD NEW ELEMENTS ABOVE THIS
        MRS_COUNT
    };

public:
    static const std::vector<double> DEFAULT_STDS;

    Metropolis_hastings_resampler(
            unsigned rng_seed,
            unsigned num_resamples,
            const Sample & initial_sample,
            const std::vector<double> & resample_stds,
            Annealing_schedule * const annealing_schedule,
            enum Movement_resampling_strategy mrs = MRS_MOMENTUM,
            bool constrain_ang_vel = false) :

            num_resamples_(num_resamples),
            cur_iter_(0),
            cur_sample_(new Sample(initial_sample)),
            cur_log_prob_(cur_sample_->log_prob()),
            as_(annealing_schedule),
            mrs_(mrs),
            constrain_ang_vel_(constrain_ang_vel)
    {
        saved_samples_.rng_seed_ = rng_seed;
        saved_samples_.samples_.reserve(num_resamples_ + 1);
        saved_samples_.samples_.push_back(cur_sample_);
        for(unsigned i = 0; i < RI_COUNT; i++)
        {
            if(mrs == MRS_VELOCITY && i == RI_L_ANG_MOM)
            {
                // convert to velocity in m/s: divide by ev of volume (ev of fracture loc * ev of height)
                // convert to velocity in m/frame: divide by fps
                // expected value of velocity std =
                // expected value of momentum std / (expected value of fracture location * expected value of height * camera fps)
                //double volume_term = Fracture_rvs::calc_frac_loc_mean(Initial_block_rvs::INIT_WIDTH_MEAN)
                //        * Initial_block_rvs::INIT_HEIGHT_MEAN;
                // value of sample #4 at local minimum
                double volume_term = 0.0235 * 0.0369;
                resample_dists_[i] = kjb::Normal_distribution(
                        0.0,
                        resample_stds[i] / (volume_term * cfg::Arguments_data_gen::CAM_FPS_DEF));
            }
            else if(mrs == MRS_VELOCITY && i == RI_R_X_MOM)
            {
                double volume_term = Fracture_rvs::calc_frac_loc_mean(Initial_block_rvs::INIT_WIDTH_MEAN)
                        * Initial_block_rvs::INIT_HEIGHT_MEAN;
                // value of sample #4 at local minimum
                // double volume_term = 0.0235 * 0.0369;
                resample_dists_[i] = kjb::Normal_distribution(
                        0.0,
                        resample_stds[i] / (volume_term * cfg::Arguments_data_gen::CAM_FPS_DEF));
            }
            else
            {
                resample_dists_[i] = kjb::Normal_distribution(0.0, resample_stds[i]);
            }
        }
    }

    ~Metropolis_hastings_resampler()
    {
        Sample *prev = nullptr;
        for(Sample *s : saved_samples_.samples_)
        {
            if(s != prev)
            {
                prev = s;
                delete s;
            }
        }
        delete as_;
    }

    Inference_record_20191031 &get_saved_samples() { return saved_samples_; }

    unsigned get_num_resamples() { return num_resamples_; }
    // Ideally, metropolis hastings wouldn't need to know about what we are doing with the samples
    // as they are generated. The shared pointers here betray that we are doing some data collation
    // of the samples, and the samples have a high rejection rate. This saves a lot of allocations.
    const Sample *get_cur_sample() const {return cur_sample_;}
    Sample *get_cur_sample() {return cur_sample_;}
    double get_cur_log_prob() const {return cur_log_prob_;}
    double get_acceptance_rate() const;

    bool still_resampling() { return cur_iter_ < num_resamples_; }

    Inference_resample_result resample_once();
    void resample(unsigned num_resamples)
    {
        for(unsigned i = 0; i < num_resamples; i++)
        {
            resample_once();
        }
    }

    void resample_all()
    {
        resample(num_resamples_ - cur_iter_);
    }
// private methods
private:
    bool try_resample_l_ang_mom(Sample & s);
    bool try_resample_r_x_mom(Sample & s);
// members
private:
    unsigned num_resamples_;
    kjb::Normal_distribution resample_dists_[RI_COUNT];
    unsigned cur_iter_;
    Sample *cur_sample_;
    Inference_record_20191031 saved_samples_;
    double cur_log_prob_;
    Sample_vector_adapter sva_;

    Annealing_schedule *as_;
    enum Movement_resampling_strategy mrs_;
    bool constrain_ang_vel_;
};

}
}

#endif // METROPOLIS_HASTINGS_HPP
