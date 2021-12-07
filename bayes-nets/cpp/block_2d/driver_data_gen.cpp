#include "config.hpp"
#include "sample.hpp"

int main(int argc, char *argv[])
{
    using namespace fracture;
    using namespace block_2d;

    cfg::Arguments_data_gen args;
    args.parse(argc, argv);

    kjb::seed_sampling_rand(args.rng_seed_);

    Sample s(args.num_ims_, args.im_w_, args.im_h_, args.cam_fps_);

    Data_record_20191031 r;
    r.rng_seed_ = args.rng_seed_;
    r.sample_ = s;

    save_data_record(args.data_folder_, args.dataset_idx_, r);

    return 0;
}
