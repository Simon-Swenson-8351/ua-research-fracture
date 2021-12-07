#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <chrono>

#include <boost/program_options.hpp>

#include <i_cpp/i_image.h>

#include "util.hpp"

namespace fracture {
namespace cfg {

class Arguments_data_gen
{
public:
    static const std::vector<std::string> NUM_IMS_OPTION;
    static const std::vector<std::string> IM_W_OPTION;
    static const std::vector<std::string> IM_H_OPTION;
    static const std::vector<std::string> CAM_FPS_OPTION;
    static const std::vector<std::string> DATASET_IDX_OPTION;
    static const std::vector<std::string> RNG_SEED_OPTION;
    static const std::vector<std::string> DATA_FOLDER_OPTION;

    static const unsigned NUM_IMS_DEF;
    static const unsigned IM_W_DEF;
    static const unsigned IM_H_DEF;
    static const double CAM_FPS_DEF;
    static const unsigned DATASET_IDX_DEF;
    static const unsigned RNG_SEED_DEF;
    static const std::string DATA_FOLDER_DEF;

    Arguments_data_gen();
    Arguments_data_gen(int argc, const char * const * const argv);

    void parse(int argc, const char * const * const argv);

    unsigned num_ims_;
    unsigned im_w_;
    unsigned im_h_;
    double cam_fps_;
    unsigned dataset_idx_;
    unsigned rng_seed_;
    std::string data_folder_;
};

class Arguments_inference_mh
{
public:
    static const std::vector<std::string> STDS_MULTIPLIER_OPTION;
    static const std::vector<std::string> STDS_EXP_OPTION;
    static const std::vector<std::string> CHAIN_IDX_OPTION;
    static const std::vector<std::string> CHAIN_LEN_OPTION;
    static const std::vector<std::string> SAMPLE_VELOCITY_OPT;
    static const std::vector<std::string> CONSTRAIN_ANG_VEL_OPT;

    static const double STDS_MULTIPLIER_DEF;
    static const unsigned STDS_EXP_DEF;
    static const unsigned CHAIN_IDX_DEF;
    static const unsigned CHAIN_LEN_DEF;
    static const bool SAMPLE_VELOCITY_DEF;
    static const bool CONSTRAIN_ANG_VEL_DEF;

    Arguments_inference_mh();
    Arguments_inference_mh(int argc, const char * const * const argv);

    void parse(int argc, const char * const * const argv);

    unsigned dataset_idx_;
    double stds_multiplier_;
    unsigned stds_exp_;
    unsigned chain_idx_;
    unsigned chain_len_;
    unsigned rng_seed_;
    std::string data_folder_;
    unsigned data_archive_ver_;

    // whether to sample velocity (true) or momentum (false) for the new proposals
    // (decouples momentum and block volume)
    bool sample_velocity_;

    // whether to reject new samples with angular velocities > 2 * pi. Removes such
    // local minima from the search space.
    bool constrain_ang_vel_;
};

class Arguments_aggregator
{
public:
    class Aggregation_exception : std::exception {};
    enum Aggregation_type
    {
        AT_MAX,
        AT_MIN,
        AT_MEAN,
        AT_FIRST,
        AT_MEDIAN,
        AT_LAST,

        // DO NOT CREATE ENTRIES BELOW HERE
        AT_COUNT
    };
    static const std::string AT_STRS[AT_COUNT];
    static const Aggregation_type AT_DEFAULT;

    // This class bears some explaining.
    // Essentially, we want to be able to represent a single index
    // or a range of indices, using the same command line option, and
    // be able to figure out which one the user wants.
    // Let's consider the case of a possible aggreagtion over a particular
    // chain's samples. We could have no aggregation, in which case the user
    // will enter a particular index:
    //      -l 50
    // We could also have aggregation:
    //      -l 0-9999
    // This will aggregate the first 10000 samples in the chain using the
    // default aggregation method, AT_DEFAULT. What if the user wants
    // a different aggregation method, however? They can specify it in
    // the following way:
    //      -l 0-9999;mean
    // the valid strings for aggregation methods (like "mean") are found in
    // the AT_STRS constant.
    // If the user wants to only aggregate every fifth sample, he or she can
    // specify:
    //      -l 0-9999;;5
    // The number after the semicolon designates the "stride" of the
    // aggregator.
    // Finally, the user might to aggregate over multiple bins and get
    // a result per each bin rather than obtain only one aggregate
    // sample over the whole range. In this case, the user must
    // follow the range indicator with a binning flag and a bin size:
    //      -l 0-9999;mean;5;1000
    // This will bin every thousand samples into one aggregate value.
    // Any of the values found in the "middle" of the string (aggregation
    // type and stride
    class Possible_aggregation
    {
    public:
        Possible_aggregation(unsigned idx) :
                aggregate_(false)
        {
            data_.no_aggregation_idx_ = idx;
        }
        Possible_aggregation(
                unsigned start_idx,
                unsigned stop_idx,
                Aggregation_type at = AT_DEFAULT,
                unsigned stride = 1,
                unsigned bin_size = 1) :

                aggregate_(true)
        {
            data_.aggregation_range_.type_ = at;
            data_.aggregation_range_.start_idx_ = start_idx;
            data_.aggregation_range_.stop_idx_ = stop_idx;
            data_.aggregation_range_.stride_ = stride;
            data_.aggregation_range_.bin_size_ = bin_size;
        }

        bool aggregate_;
        union
        {
            unsigned no_aggregation_idx_;
            struct
            {
                Aggregation_type type_;
                unsigned start_idx_;
                unsigned stop_idx_;
                unsigned stride_;
                unsigned bin_size_;
            } aggregation_range_;
        } data_;
    };

    static const std::vector<std::string> GROUND_TRUTH_FOLDER_OPT;
    static const std::vector<std::string> OUTPUT_FOLDER_OPT;
    // TODO this should be moved to its own separate arguments class
    static const std::vector<std::string> DATA_ARCHIVE_VER_OPT;
    static const std::vector<std::string> INFERENCE_ARCHIVE_VER_OPT;



    // TODO ADD OPTIONS FOR THESE
    static const std::string OUTPUT_FOLDER_DEF;
    static const Possible_aggregation DATASET_DEF;
    static const Possible_aggregation EXPLORATION_RATE_DEF;
    static const Possible_aggregation CHAIN_DEF;
    static const Possible_aggregation CHAIN_SAMPLE_DEF;
    static const unsigned DATA_ARCHIVE_VER_DEF;
    static const unsigned INFERENCE_ARCHIVE_VER_DEF;

    Arguments_aggregator();
    Arguments_aggregator(int argc, const char * const * const argv);

    void parse(int argc, const char * const * const argv);

    std::string ground_truth_folder_;
    std::string in_folder_;
    std::string out_folder_;
    Possible_aggregation dataset_;
    Possible_aggregation exploration_rate_;
    Possible_aggregation chain_;
    Possible_aggregation chain_sample_;
    unsigned data_archive_ver_;
    unsigned inference_archive_ver_;

private:
    static Possible_aggregation parse_idx_or_range(int argc, const char * const * const argv);
};

class Arguments_image_combiner
{
public:

    Arguments_image_combiner();
    Arguments_image_combiner(int argc, const char * const * const argv);

    void parse(int argc, const char * const * const argv);

    std::string in_folder_;

    unsigned dataset_idx_;

    kjb::Image::Pixel_type ground_truth_polygons_color_;
    kjb::Image::Pixel_type ground_truth_centers_of_mass_color_;
    kjb::Image::Pixel_type observation_polygons_color_;

    unsigned exploration_rate_;
    unsigned chain_idx_;
    unsigned chain_sample_idx_;

    kjb::Image::Pixel_type inference_polygons_color_;
    kjb::Image::Pixel_type inference_centers_of_mass_color_;
};

const kjb::Image::Pixel_type COLOR_GROUND_TRUTH_POLYGONS =
{
    .r = 0.0,
    .g = 255.0,
    .b = 0.0,
    .extra = {
        .alpha = 255.0
    }
};
const kjb::Image::Pixel_type COLOR_GROUND_TRUTH_CENTERS_OF_MASS =
{
    .r = 0.0,
    .g = 127.0,
    .b = 0.0,
    .extra = {
        .alpha = 255.0
    }
};
const kjb::Image::Pixel_type COLOR_OBSERVATION_POLYGONS =
{
    .r = 255.0,
    .g = 0.0,
    .b = 0.0,
    .extra = {
        .alpha = 255.0
    }
};
const kjb::Image::Pixel_type COLOR_INFERENCE_POLYGONS =
{
    .r = 0.0,
    .g = 127.0,
    .b = 255.0,
    .extra = {
        .alpha = 255.0
    }
};
const kjb::Image::Pixel_type COLOR_INFERENCE_CENTERS_OF_MASS =
{
    .r = 0.0,
    .g = 63.0,
    .b = 127.0,
    .extra = {
        .alpha = 255.0
    }
};

const kjb::Image::Pixel_type COLOR_FOREGROUND =
{
    .r = 255.0,
    .g = 255.0,
    .b = 255.0,
    .extra = {
        .alpha = 255.0
    }
};

const kjb::Image::Pixel_type COLOR_BACKGROUND =
{
    .r = 0.0,
    .g = 0.0,
    .b = 0.0,
    .extra = {
        .alpha = 255.0
    }
};

void parse_data_gen_args(int argc, const char * const * const argv, boost::program_options::variables_map &vm);

}

namespace util
{

template<>
inline cfg::Arguments_aggregator::Aggregation_type sto<cfg::Arguments_aggregator::Aggregation_type>(
        const std::string &s,
        size_t *idx)
{
    for(size_t cur_agg_type_idx = 0; cur_agg_type_idx < cfg::Arguments_aggregator::AT_COUNT; cur_agg_type_idx++)
    {
        if(s.find(cfg::Arguments_aggregator::AT_STRS[cur_agg_type_idx]) == 0)
        {
            if(idx)
            {
                *idx = cfg::Arguments_aggregator::AT_STRS[cur_agg_type_idx].length();
            }
            return cfg::Arguments_aggregator::Aggregation_type(cur_agg_type_idx);
        }
    }
}

}
}

std::istringstream &operator>>(std::istringstream &ss, fracture::cfg::Arguments_aggregator::Aggregation_type &o);

#endif // CONFIG_HPP
