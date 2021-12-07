#include <vector>

#include <prob_cpp/prob_sample.h>

#include "config.hpp"
#include "sample.hpp"

int main(int argc, char *argv[])
{
    using namespace fracture;
    using namespace block_2d;

    cfg::Arguments_data_gen args;

    args.data_folder_ = "./test_archive_tmp_data";
    args.rng_seed_ = 42;
    size_t num_samples = 64;
    size_t consecutive_samples = 4;

    unsigned cur_rng_seed = args.rng_seed_;

    // Inference archive
    Inference_record_20191031 inference_record_out;
    inference_record_out.rng_seed_ = cur_rng_seed;
    inference_record_out.samples_.reserve(num_samples * consecutive_samples);

    for(size_t i = 0; i < num_samples; i++)
    {
        // Data archive
        kjb::seed_sampling_rand(++cur_rng_seed);
        Data_record_20191031 data_record_out;
        data_record_out.rng_seed_ = cur_rng_seed;
        data_record_out.sample_ = Sample(args.num_ims_, args.im_w_, args.im_h_, args.cam_fps_);
        save_data_record(args.data_folder_, i, data_record_out);
        Data_record_20191031 data_record_in;
        load_data_record(args.data_folder_, i, data_record_in);
        assert(data_record_out.rng_seed_ == data_record_in.rng_seed_);
        assert(data_record_out.sample_ == data_record_in.sample_);
        assert(data_record_out.sample_.log_prob() == data_record_in.sample_.log_prob());

        Sample *s_ptr = new Sample(data_record_out.sample_);
        for(size_t j = 0; j < consecutive_samples; j++)
        {
            inference_record_out.samples_.push_back(s_ptr);
        }
    }

    std::vector<unsigned> flex_vars(0);
    save_inference_record(args.data_folder_, 0, util::IT_METROPOLIS, flex_vars, inference_record_out);
    Inference_record_20191031 inference_record_in;
    load_inference_record(args.data_folder_, 0, util::IT_METROPOLIS, flex_vars, inference_record_in);
    assert(inference_record_out.rng_seed_ == inference_record_in.rng_seed_);
    for(size_t i = 0; i < num_samples * consecutive_samples; i++)
    {
        assert(*inference_record_out.samples_[i] == *inference_record_in.samples_[i]);
        assert(inference_record_out.samples_[i]->log_prob() == inference_record_in.samples_[i]->log_prob());
    }

    return 0;
}
