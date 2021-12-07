#include <boost/filesystem/operations.hpp>

#include "i_cpp/i_image.h"

#include "config.hpp"
#include "util.hpp"

int main(int argc, char *argv[])
{
    using namespace fracture;
    using namespace cfg;

    Arguments_image_combiner args(argc, argv);

    std::vector<unsigned> flex_vars = util::setup_mh_flex_vars(args.exploration_rate_, args.chain_idx_, args.chain_sample_idx_);

    std::vector<kjb::Image::Pixel_type> colors(5);
    colors[0] = args.ground_truth_polygons_color_;
    colors[1] = args.ground_truth_centers_of_mass_color_;
    colors[2] = args.observation_polygons_color_;
    colors[3] = args.inference_polygons_color_;
    colors[4] = args.inference_centers_of_mass_color_;

    for(unsigned i = 0; ; i++)
    {
        boost::filesystem::path ground_truth_polygons_path = util::get_data_image_path(args.in_folder_, args.dataset_idx_, i, util::DIT_ACTUAL_GEOM);
        // Check if we've iterated over every image
        if(!boost::filesystem::exists(ground_truth_polygons_path)) break;
        kjb::Image ground_truth_polygons_im(ground_truth_polygons_path.string());
        kjb::Image ground_truth_centers_of_mass_im(util::get_data_image_path(
                args.in_folder_,
                args.dataset_idx_,
                i,
                util::DIT_CENTER_OF_MASS).string());
        kjb::Image observation_polygons_im(util::get_data_image_path(
                args.in_folder_,
                args.dataset_idx_,
                i,
                util::DIT_OBSERVED_GEOM).string());
        kjb::Image inference_centers_of_mass_im(util::get_inference_image_path(
                args.in_folder_,
                args.dataset_idx_,
                util::IT_METROPOLIS,
                flex_vars,
                i,
                util::IIT_CENTER_OF_MASS).string());
        kjb::Image inference_polygons_im(util::get_inference_image_path(
                args.in_folder_,
                args.dataset_idx_,
                util::IT_METROPOLIS,
                flex_vars,
                i,
                util::IIT_GEOM).string());
        std::vector<const kjb::Image *> images(5);
        images[0] = &ground_truth_polygons_im;
        images[1] = &ground_truth_centers_of_mass_im;
        images[2] = &observation_polygons_im;
        images[3] = &inference_polygons_im;
        images[4] = &inference_centers_of_mass_im;

        kjb::Image out_im(ground_truth_polygons_im.get_num_rows(), ground_truth_polygons_im.get_num_cols(), cfg::COLOR_BACKGROUND.r, cfg::COLOR_BACKGROUND.g, cfg::COLOR_BACKGROUND.b);
        for(size_t cur_im_idx = 0; cur_im_idx < images.size(); cur_im_idx++)
        {
            util::transfer_image_contents(out_im, *images[cur_im_idx], colors[cur_im_idx]);
        }
        out_im.write(util::get_inference_image_path(
                args.in_folder_,
                args.dataset_idx_,
                util::IT_METROPOLIS,
                flex_vars,
                i,
                util::IIT_COMBINED).string());
    }
}
