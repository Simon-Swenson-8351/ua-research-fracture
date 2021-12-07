#include <fstream>

#include <prob_cpp/prob_sample.h>

#include "config.hpp"
#include "sample.hpp"
#include "sample_vector_adapter.hpp"

namespace fracture
{
namespace block_2d
{
namespace driver_inference_graddesc
{

static void run_sample(const cfg::Arguments_inference_mh & args)
{
    using namespace fracture::block_2d;

    std::ofstream log_file;
    log_file.open(std::to_string(args.chain_idx_) + ".log");

    Sample data_sample;
    if(args.data_archive_ver_ >= 20191031)
    {
        Data_record_20191031 data_record;
        load_data_record(args.data_folder_, args.dataset_idx_, data_record);
        data_sample = data_record.sample_;
    }
    else if(args.data_archive_ver_ < 20191031)
    {
        load_sample(args.data_folder_, args.dataset_idx_, data_sample);
    }

    Sample cur_sample(data_sample);
    cur_sample.forward_sample_hidden_rvs();
    log_file << "data log probability: " << cur_sample.log_prob() << "\n";
    log_file.flush();

    Inference_record_20191031 results;
    results.rng_seed_ = args.rng_seed_;

    double exploration_rate = std::pow(args.stds_multiplier_, args.stds_exp_);

    size_t i = 0;
    Sample_vector_adapter sva;
    while(true)
    {
        Sample new_sample(cur_sample);
        kjb::Vector grad = sample_log_gradient(cur_sample);
        kjb::Vector grad_norm = grad / grad.magnitude();
        for(int dimension = 0; dimension < grad.get_length(); dimension++)
        {
            try
            {
                sva.set(&new_sample, dimension, sva.get(&cur_sample, dimension) + exploration_rate * grad_norm[dimension]);
            }
            catch(const prob::No_support_exception &e)
            {
                // do not save the new, partially-set sample.
                new_sample = cur_sample;
                break;
            }
        }
        double ratio = std::exp(new_sample.log_prob() - cur_sample.log_prob());
        if(ratio < 1.0)
        {
            // No longer improving. Reached the local optimum.
            break;
        }

        if(!(i % 10000))
        {
            log_file << "iteration :" << i << " | grad mag :" << grad.magnitude() << " | ratio :" << ratio << " | log prob: " << new_sample.log_prob() << "\n";
            log_file.flush();
        }

        cur_sample = new_sample;
        i++;
    }

    log_file << "chain #: " << args.chain_idx_ << " | iterations: " << i << " | log prob: " << cur_sample.log_prob() << "\n";
    log_file.flush();

    results.samples_.push_back(new Sample(cur_sample));

    std::vector<unsigned> flex_vars = util::setup_mh_flex_vars(args.stds_exp_, args.chain_idx_, 0);
    save_inference_record(
            args.data_folder_,
            args.dataset_idx_,
            util::IT_METROPOLIS,
            flex_vars,
            results);
}

}
}
}

int main(int argc, char *argv[])
{
    using namespace fracture;
    using namespace cfg;
    using namespace block_2d::driver_inference_graddesc;

    Arguments_inference_mh args(argc, argv);

    kjb::seed_sampling_rand(args.rng_seed_);

    run_sample(args);
}
