#include <fstream>
#include <vector>
#include <chrono>
#include <thread>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/filesystem/path.hpp>

#include "config.hpp"
#include "util.hpp"
#include "sample.hpp"
#include "metropolis_hastings.hpp"

namespace fracture
{
namespace block_2d
{
namespace driver_inference_mh
{

static void init_mh_stds(std::vector<double> &vec, double multiplier, unsigned multiplier_exp)
{
    multiplier = std::pow(multiplier, double(multiplier_exp));
    for(double &elem : vec)
    {
        elem *= multiplier;
    }
}


void draw_and_save_image(const cfg::Arguments_inference_mh & args, const Sample & s, const std::vector<unsigned> & flex_vars, unsigned im_idx)
{
    kjb::Image im_com(int(s.get_camera().get_image_height()), int(s.get_camera().get_image_width()), cfg::COLOR_BACKGROUND.r, cfg::COLOR_BACKGROUND.g, cfg::COLOR_BACKGROUND.b);
    kjb::Image im_inf_geom(int(s.get_camera().get_image_height()), int(s.get_camera().get_image_width()), cfg::COLOR_BACKGROUND.r, cfg::COLOR_BACKGROUND.g, cfg::COLOR_BACKGROUND.b);
    std::vector<const kjb::Vector_d<3> *> centers_of_mass = s.get_image_center_of_mass(im_idx);
    for(const kjb::Vector_d<3> *pt : centers_of_mass)
    {
        im_com.draw_point((*pt)[0] / (*pt)[2], (*pt)[1] / (*pt)[2], 1, cfg::COLOR_FOREGROUND);
    }
    std::vector<const kjb::Matrix_d<3,4> *> polygons = s.get_image_polygon_actual(im_idx);
    for(const kjb::Matrix_d<3,4> *block : polygons)
    {
        util::draw_poly_edges(im_inf_geom, util::static_matrix_to_dynamic_matrix<3,4>(*block), cfg::COLOR_FOREGROUND);
    }
    im_com.write(util::get_inference_image_path(
            args.data_folder_,
            args.dataset_idx_,
            util::IT_METROPOLIS,
            flex_vars,
            im_idx,
            util::IIT_CENTER_OF_MASS).string());
    im_inf_geom.write(util::get_inference_image_path(
            args.data_folder_,
            args.dataset_idx_,
            util::IT_METROPOLIS,
            flex_vars,
            im_idx,
            util::IIT_GEOM).string());
}

void draw_and_save_images(const cfg::Arguments_inference_mh & args, const Sample & s, const std::vector<unsigned> & flex_vars)
{
    for(unsigned j = 0; j < s.get_num_ims(); j++)
        draw_and_save_image(args, s, flex_vars, j);
}

static void run_sample(const cfg::Arguments_inference_mh & args)
{
    using namespace fracture::block_2d;
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

    std::vector<double> stds = Metropolis_hastings_resampler::DEFAULT_STDS;
    init_mh_stds(stds, args.stds_multiplier_, args.stds_exp_);

    block_2d::Sample mh_init_sample(data_sample);
    mh_init_sample.forward_sample_hidden_rvs();
    // 20191217 experiment: clamp width, height to the known good values to see if we still get local
    // optima.
    mh_init_sample.set_block_initial_width(data_sample.get_initial_block_rvs().get_initial_width());
    mh_init_sample.set_block_initial_height(data_sample.get_initial_block_rvs().get_initial_height());
    // Whether to enable annealing or not. At some point, this should be made a command
    // line parameter or config file variable.
    // start at a temperature of a million. set an alpha such that the temperature is 1 after
    // a million iterations.
    //Annealing_schedule *as = new Traditional_annealing_schedule(1e6, 0.999986);
    Annealing_schedule *as = new No_annealing_schedule();
    block_2d::Metropolis_hastings_resampler mhr(
            args.rng_seed_,
            args.chain_len_,
            mh_init_sample,
            stds,
            as,
            args.sample_velocity_ ? Metropolis_hastings_resampler::MRS_VELOCITY : Metropolis_hastings_resampler::MRS_MOMENTUM,
            args.constrain_ang_vel_);
    std::vector<unsigned> flex_vars = util::setup_mh_flex_vars(args.stds_exp_, args.chain_idx_, 0);
    while(mhr.still_resampling()) mhr.resample_once();
    // Only save the last sample. Otherwise, with annealing, the number of saved samples was too long.
    // The last sample is a good heuristic for the best probability in the chain.
    Inference_record_20191031 to_save;
    to_save.rng_seed_ = mhr.get_saved_samples().rng_seed_;
    to_save.samples_.push_back(mhr.get_saved_samples().samples_[mhr.get_saved_samples().samples_.size() - 1]);
    save_inference_record(
            args.data_folder_,
            args.dataset_idx_,
            util::IT_METROPOLIS,
            flex_vars,
            to_save);
    delete as;
}

}
}
}

int main(int argc, char *argv[])
{
    using namespace fracture;
    using namespace cfg;
    using namespace fracture::block_2d::driver_inference_mh;

    Arguments_inference_mh args(argc, argv);

    kjb::seed_sampling_rand(args.rng_seed_);

    run_sample(args);
}
