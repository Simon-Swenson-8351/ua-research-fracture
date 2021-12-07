#include "config.hpp"
#include "metropolis_hastings.hpp"
#include "sample.hpp"

int main(int argc, char *argv[])
{
    using namespace fracture;
    using namespace block_2d;

    cfg::Arguments_inference_mh args;

    args.rng_seed_ = 42;
    size_t num_data_samples = 64;
    args.chain_len_ = 64;
    args.stds_multiplier_ = 1.5;
    args.stds_exp_ = 5;
    size_t num_ims = 30;
    size_t im_w = 640;
    size_t im_h = 480;
    double c_fps = 30.0;

    kjb::seed_sampling_rand(args.rng_seed_);

    std::vector<double> stds = Metropolis_hastings_resampler::DEFAULT_STDS;
    double multiplier = std::pow(args.stds_multiplier_, double(args.stds_exp_));
    for(double &std : stds)
    {
        std *= multiplier;
    }
    for(size_t data_idx = 0; data_idx < num_data_samples; data_idx++)
    {
        Sample data(num_ims, im_w, im_h, c_fps);
        Sample init_inference_sample(data);
        init_inference_sample.forward_sample_hidden_rvs();
        Annealing_schedule *as = new No_annealing_schedule();
        Metropolis_hastings_resampler mhr(
                args.rng_seed_,
                args.chain_len_,
                init_inference_sample,
                stds,
                as
        );
        mhr.resample_once();
        mhr.resample(3);
        mhr.resample_all();
        assert(!mhr.still_resampling());
        double prev_log_prob = double(-INFINITY);
        for(Sample *s : mhr.get_saved_samples().samples_)
        {
            double cur_log_prob = s->log_prob();
            assert(cur_log_prob >= prev_log_prob);
            if(cur_log_prob > prev_log_prob) prev_log_prob = cur_log_prob;
        }
        delete as;
    }
}
