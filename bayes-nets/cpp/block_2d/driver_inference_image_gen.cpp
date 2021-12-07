#include "boost/archive/archive_exception.hpp"

#include "config.hpp"
#include "sample.hpp"
#include "util.hpp"

int main(int argc, char *argv[])
{
    using namespace fracture;
    using namespace block_2d;
    using namespace cfg;
    using namespace util;

    // TODO make its own argument class
    Arguments_aggregator args(argc, argv);

    assert(!args.dataset_.aggregate_);
    assert(!args.exploration_rate_.aggregate_);
    assert(!args.chain_.aggregate_);
    assert(!args.chain_sample_.aggregate_);

    std::vector<unsigned> chain_flex_vars = util::setup_mh_flex_vars(
            args.exploration_rate_.data_.no_aggregation_idx_,
            args.chain_.data_.no_aggregation_idx_,
            0);

    Sample s;

    if(args.inference_archive_ver_ >= 20191031)
    {
        Inference_record_20191031 ir;
        load_inference_record(
                args.in_folder_,
                args.dataset_.data_.no_aggregation_idx_,
                util::IT_METROPOLIS,
                chain_flex_vars,
                ir);
        s = *ir.samples_[args.chain_sample_.data_.no_aggregation_idx_];
    }
    else
    {
        std::vector<Sample *> vec;
        load_sample_vector(
                args.in_folder_,
                args.dataset_.data_.no_aggregation_idx_,
                util::IT_METROPOLIS,
                chain_flex_vars,
                vec);
        s = *vec[args.chain_sample_.data_.no_aggregation_idx_];
    }

    std::vector<unsigned> im_flex_vars = util::setup_mh_flex_vars(
                args.exploration_rate_.data_.no_aggregation_idx_,
                args.chain_.data_.no_aggregation_idx_,
                args.chain_sample_.data_.no_aggregation_idx_);

    for(unsigned i = 0; i < s.get_num_ims(); i++)
    {
        util::save_center_of_mass_image(
                s.get_camera().get_image_width(),
                s.get_camera().get_image_height(),
                s.get_image_center_of_mass(i),
                util::get_inference_image_path(
                        args.in_folder_,
                        args.dataset_.data_.no_aggregation_idx_,
                        util::IT_METROPOLIS,
                        im_flex_vars,
                        i,
                        IIT_CENTER_OF_MASS));
        util::save_endpoint_image(
                s.get_camera().get_image_width(),
                s.get_camera().get_image_height(),
                s.get_image_polygon_actual(i),
                util::get_inference_image_path(
                        args.in_folder_,
                        args.dataset_.data_.no_aggregation_idx_,
                        util::IT_METROPOLIS,
                        im_flex_vars,
                        i,
                        IIT_GEOM));
    }
}
