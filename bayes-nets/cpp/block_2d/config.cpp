#include "config.hpp"
#include "util.hpp"

namespace fracture {
namespace cfg {

const std::vector<std::string> Arguments_data_gen::NUM_IMS_OPTION     = {"-n", "--num-ims"};
const std::vector<std::string> Arguments_data_gen::IM_W_OPTION        = {"-w", "--width"};
const std::vector<std::string> Arguments_data_gen::IM_H_OPTION        = {"-h", "--height"};
const std::vector<std::string> Arguments_data_gen::CAM_FPS_OPTION     = {"-f", "--fps"};
const std::vector<std::string> Arguments_data_gen::DATASET_IDX_OPTION = {"-i", "--index"};
const std::vector<std::string> Arguments_data_gen::RNG_SEED_OPTION    = {"-s", "--seed"};
const std::vector<std::string> Arguments_data_gen::DATA_FOLDER_OPTION = {"-d", "--data-folder"};
const unsigned Arguments_data_gen::NUM_IMS_DEF        = 30;
const unsigned Arguments_data_gen::IM_W_DEF           = 640;
const unsigned Arguments_data_gen::IM_H_DEF           = 480;
const double Arguments_data_gen::CAM_FPS_DEF          = 30.0;
const unsigned Arguments_data_gen::DATASET_IDX_DEF    = 0;
const unsigned Arguments_data_gen::RNG_SEED_DEF       = unsigned(util::get_sys_time_nano());
const std::string Arguments_data_gen::DATA_FOLDER_DEF = "./data/";

Arguments_data_gen::Arguments_data_gen() :
        num_ims_(NUM_IMS_DEF),
        im_w_(IM_W_DEF),
        im_h_(IM_H_DEF),
        cam_fps_(CAM_FPS_DEF),
        dataset_idx_(DATASET_IDX_DEF),
        rng_seed_(RNG_SEED_DEF),
        data_folder_(DATA_FOLDER_DEF)
{}

Arguments_data_gen::Arguments_data_gen(int argc, const char * const * const argv) :
        Arguments_data_gen()
{
    parse(argc, argv);
}

void Arguments_data_gen::parse(int argc, const char * const * const argv)
{
    for(int i = 0; i < argc; i++)
    {
        if(NUM_IMS_OPTION[0] == argv[i] || NUM_IMS_OPTION[1] == argv[i])
        {
            num_ims_ = unsigned(std::stoul(argv[i + 1]));
        }
        else if(IM_W_OPTION[0] == argv[i] || IM_W_OPTION[1] == argv[i])
        {
            im_w_ = unsigned(std::stoul(argv[i + 1]));
        }
        else if(IM_H_OPTION[0] == argv[i] || IM_H_OPTION[1] == argv[i])
        {
            im_h_ = unsigned(std::stoul(argv[i + 1]));
        }
        else if(CAM_FPS_OPTION[0] == argv[i] || CAM_FPS_OPTION[1] == argv[i])
        {
            cam_fps_ = std::stod(argv[i + 1]);
        }
        else if(DATASET_IDX_OPTION[0] == argv[i] || DATASET_IDX_OPTION[1] == argv[i])
        {
            dataset_idx_ = unsigned(std::stoul(argv[i + 1]));
        }
        else if(RNG_SEED_OPTION[0] == argv[i] || RNG_SEED_OPTION[1] == argv[i])
        {
            rng_seed_ = unsigned(std::stoul(argv[i + 1]));
        }
        else if(DATA_FOLDER_OPTION[0] == argv[i] || DATA_FOLDER_OPTION[1] == argv[i])
        {
            data_folder_ = argv[i + 1];
        }
        else
        {
            continue;
        }
        i++;
    }
}

const std::vector<std::string> Arguments_inference_mh::STDS_MULTIPLIER_OPTION = {"-m", "--multiplier"};
const std::vector<std::string> Arguments_inference_mh::STDS_EXP_OPTION = {"-e", "--exponent"};
const std::vector<std::string> Arguments_inference_mh::CHAIN_IDX_OPTION = { "-c", "--chain-idx"};
const std::vector<std::string> Arguments_inference_mh::CHAIN_LEN_OPTION = {"-l", "--length"};
const std::vector<std::string> Arguments_inference_mh::SAMPLE_VELOCITY_OPT = {"-V", "--sample-velocities"};
const std::vector<std::string> Arguments_inference_mh::CONSTRAIN_ANG_VEL_OPT = {"-C", "--constrain-angular-velocities"};
const double Arguments_inference_mh::STDS_MULTIPLIER_DEF = 1.5;
const unsigned Arguments_inference_mh::STDS_EXP_DEF = 0;
const unsigned Arguments_inference_mh::CHAIN_IDX_DEF = 0;
const unsigned Arguments_inference_mh::CHAIN_LEN_DEF = 1000000;
// default is dumb mode
const bool Arguments_inference_mh::SAMPLE_VELOCITY_DEF = false;
const bool Arguments_inference_mh::CONSTRAIN_ANG_VEL_DEF = false;

Arguments_inference_mh::Arguments_inference_mh() :
    dataset_idx_(Arguments_data_gen::DATASET_IDX_DEF),
    stds_multiplier_(STDS_MULTIPLIER_DEF),
    stds_exp_(STDS_EXP_DEF),
    chain_idx_(CHAIN_IDX_DEF),
    chain_len_(CHAIN_LEN_DEF),
    rng_seed_(Arguments_data_gen::RNG_SEED_DEF),
    data_folder_(Arguments_data_gen::DATA_FOLDER_DEF),
    data_archive_ver_(Arguments_aggregator::DATA_ARCHIVE_VER_DEF),
    sample_velocity_(SAMPLE_VELOCITY_DEF),
    constrain_ang_vel_(CONSTRAIN_ANG_VEL_DEF)
{}

Arguments_inference_mh::Arguments_inference_mh(int argc, const char * const * const argv) :
        Arguments_inference_mh()
{
    parse(argc, argv);
}

void Arguments_inference_mh::parse(int argc, const char * const * const argv)
{
    for(int i = 0; i < argc; i++)
    {
        if(Arguments_data_gen::DATASET_IDX_OPTION[0] == argv[i] || Arguments_data_gen::DATASET_IDX_OPTION[1] == argv[i])
        {
            dataset_idx_ = unsigned(std::stoul(argv[i + 1]));
        }
        else if(STDS_MULTIPLIER_OPTION[0] == argv[i] || STDS_MULTIPLIER_OPTION[1] == argv[i])
        {
            stds_multiplier_ = std::stod(argv[i + 1]);
        }
        else if(STDS_EXP_OPTION[0] == argv[i] || STDS_EXP_OPTION[1] == argv[i])
        {
            stds_exp_ = unsigned(std::stoul(argv[i + 1]));
        }
        else if(CHAIN_IDX_OPTION[0] == argv[i] || CHAIN_IDX_OPTION[1] == argv[i])
        {
            chain_idx_ = unsigned(std::stoul(argv[i + 1]));
        }
        else if(CHAIN_LEN_OPTION[0] == argv[i] || CHAIN_LEN_OPTION[1] == argv[i])
        {
            chain_len_ = unsigned(std::stoul(argv[i + 1]));
        }
        else if(Arguments_data_gen::RNG_SEED_OPTION[0] == argv[i] || Arguments_data_gen::RNG_SEED_OPTION[1] == argv[i])
        {
            rng_seed_ = unsigned(std::stoul(argv[i + 1]));
        }
        else if(Arguments_data_gen::DATA_FOLDER_OPTION[0] == argv[i] || Arguments_data_gen::DATA_FOLDER_OPTION[1] == argv[i])
        {
            data_folder_ = argv[i + 1];
        }
        else if(Arguments_aggregator::DATA_ARCHIVE_VER_OPT[0] == argv[i] || Arguments_aggregator::DATA_ARCHIVE_VER_OPT[1] == argv[i])
        {
            data_archive_ver_ = unsigned(std::stoul(argv[i + 1]));
        }
        else if(SAMPLE_VELOCITY_OPT[0] == argv[i] || SAMPLE_VELOCITY_OPT[1] == argv[i])
        {
            sample_velocity_ = true;
            continue;
        }
        else if(CONSTRAIN_ANG_VEL_OPT[0] == argv[i] || CONSTRAIN_ANG_VEL_OPT[1] == argv[i])
        {
            constrain_ang_vel_ = true;
            continue;
        }
        else
        {
            continue;
        }
        i++;
    }
}

const std::string Arguments_aggregator::AT_STRS[AT_COUNT] = {
    "max",
    "min",
    "mean",
    "first",
    "med",
    "last",
};
const Arguments_aggregator::Aggregation_type Arguments_aggregator::AT_DEFAULT = AT_MEDIAN;

const std::vector<std::string> Arguments_aggregator::GROUND_TRUTH_FOLDER_OPT = {"-g", "--ground-truth-folder"};
const std::vector<std::string> Arguments_aggregator::OUTPUT_FOLDER_OPT {"-o", "--output-folder"};
const std::vector<std::string> Arguments_aggregator::DATA_ARCHIVE_VER_OPT = {"-v", "--data-archive-version"};
const std::vector<std::string> Arguments_aggregator::INFERENCE_ARCHIVE_VER_OPT = {"-r", "--inference-archive-version"};

const std::string Arguments_aggregator::OUTPUT_FOLDER_DEF;
const Arguments_aggregator::Possible_aggregation Arguments_aggregator::DATASET_DEF(
        Arguments_data_gen::DATASET_IDX_DEF);
const Arguments_aggregator::Possible_aggregation Arguments_aggregator::EXPLORATION_RATE_DEF(
        Arguments_inference_mh::STDS_EXP_DEF);
const Arguments_aggregator::Possible_aggregation Arguments_aggregator::CHAIN_DEF(
        Arguments_inference_mh::CHAIN_IDX_DEF);
const Arguments_aggregator::Possible_aggregation Arguments_aggregator::CHAIN_SAMPLE_DEF(0);
const unsigned Arguments_aggregator::DATA_ARCHIVE_VER_DEF = 20191031;
const unsigned Arguments_aggregator::INFERENCE_ARCHIVE_VER_DEF = 20191031;

Arguments_aggregator::Arguments_aggregator() :
        in_folder_(Arguments_data_gen::DATA_FOLDER_DEF),
        out_folder_(OUTPUT_FOLDER_DEF),
        dataset_(DATASET_DEF),
        exploration_rate_(EXPLORATION_RATE_DEF),
        chain_(CHAIN_DEF),
        chain_sample_(CHAIN_SAMPLE_DEF),
        data_archive_ver_(DATA_ARCHIVE_VER_DEF),
        inference_archive_ver_(INFERENCE_ARCHIVE_VER_DEF)
{}

Arguments_aggregator::Arguments_aggregator(int argc, const char * const * const argv):
        Arguments_aggregator()
{
    parse(argc, argv);
}

void Arguments_aggregator::parse(int argc, const char * const * const argv)
{
    unsigned aggregation_ct = 0;
    for(int i = 0; i < argc; i++)
    {
        if(Arguments_data_gen::DATA_FOLDER_OPTION[0] == argv[i] || Arguments_data_gen::DATA_FOLDER_OPTION[1] == argv[i])
        {
            in_folder_ = argv[i + 1];
        }
        else if(OUTPUT_FOLDER_OPT[0] == argv[i] || OUTPUT_FOLDER_OPT[1] == argv[i])
        {
            out_folder_ = argv[i + 1];
        }
        else if(GROUND_TRUTH_FOLDER_OPT[0] == argv[i] || GROUND_TRUTH_FOLDER_OPT[1] == argv[i])
        {
            ground_truth_folder_ = argv[i + 1];
        }
        else if(Arguments_data_gen::DATASET_IDX_OPTION[0] == argv[i] || Arguments_data_gen::DATASET_IDX_OPTION[1] == argv[i])
        {
            dataset_ = parse_idx_or_range(argc - i, argv + i + 1);
            if(dataset_.aggregate_)
                aggregation_ct++;
        }
        else if(Arguments_inference_mh::STDS_EXP_OPTION[0] == argv[i] || Arguments_inference_mh::STDS_EXP_OPTION[0] == argv[i])
        {
            exploration_rate_ = parse_idx_or_range(argc - i, argv + i + 1);
            if(exploration_rate_.aggregate_)
                aggregation_ct++;
        }
        else if(Arguments_inference_mh::CHAIN_IDX_OPTION[0] == argv[i] || Arguments_inference_mh::CHAIN_IDX_OPTION[1] == argv[i])
        {
            chain_ = parse_idx_or_range(argc - i, argv + i + 1);
            if(chain_.aggregate_)
                aggregation_ct++;
        }
        else if(Arguments_inference_mh::CHAIN_LEN_OPTION[0] == argv[i] || Arguments_inference_mh::CHAIN_LEN_OPTION[1] == argv[i])
        {
            chain_sample_ = parse_idx_or_range(argc - i, argv + i + 1);
            if(chain_sample_.aggregate_)
                aggregation_ct++;
        }
        else if(DATA_ARCHIVE_VER_OPT[0] == argv[i] || DATA_ARCHIVE_VER_OPT[1] == argv[i])
        {
            data_archive_ver_ = unsigned(std::stoul(argv[i + 1]));
        }
        else if(INFERENCE_ARCHIVE_VER_OPT[0] == argv[i] || INFERENCE_ARCHIVE_VER_OPT[1] == argv[i])
        {
            inference_archive_ver_ = unsigned(std::stoul(argv[i + 1]));
        }
        else
        {
            continue;
        }
        i++;
    }
    if(aggregation_ct > 1) throw Aggregation_exception(); //util::err_str(__FILE__, __LINE__);
}

// Figures out whether the current option is empty or not (basically, if it
// starts with ';' or it's EOF, it's empty). Then, parses the value and consumes it
// from the string.
// side-effect: ss will have tokens consumed if an option was found.
template<typename T>
bool consume_aggregation_suboption(std::istringstream &ss, T & t)
{
    bool r = false;
    std::char_traits<char>::int_type peek = ss.peek();
    if(peek != ';' && peek != std::char_traits<char>::eof() && peek != 0)
    {
        ss >> t;
        r = true;
    }
    peek = ss.peek();
    if(peek != std::char_traits<char>::eof())
    {
        char semic;
        ss >> semic;
        assert(semic == ';');
    }
    return r;
}

static Arguments_aggregator::Possible_aggregation consume_all_aggregation_suboptions(std::string &s)
{
    std::istringstream ss(s);
    std::pair<unsigned,unsigned> start_end;
    Arguments_aggregator::Aggregation_type at = Arguments_aggregator::AT_DEFAULT;
    unsigned stride = 1;
    consume_aggregation_suboption(ss, start_end);
    // add one since "end" is included
    unsigned bin_size = start_end.second + 1 - start_end.first;
    consume_aggregation_suboption<Arguments_aggregator::Aggregation_type>(ss, at);
    consume_aggregation_suboption<unsigned>(ss, stride);
    consume_aggregation_suboption<unsigned>(ss, bin_size);
    return Arguments_aggregator::Possible_aggregation(
            start_end.first,
            start_end.second,
            at,
            stride,
            bin_size);
}

// Note that argv is assumed to be offset already such that the first entry is the index or range specifier.
Arguments_aggregator::Possible_aggregation Arguments_aggregator::parse_idx_or_range(
        int remaining_argc,
        const char * const * const remaining_argv)
{
    std::string cur_str(remaining_argv[0]);
    size_t split_char_idx;
    if((split_char_idx = cur_str.find('-')) != std::string::npos)
    {
        return consume_all_aggregation_suboptions(cur_str);
    }
    else
    {
        return Arguments_aggregator::Possible_aggregation(unsigned(std::stoul(cur_str)));
    }
}

Arguments_image_combiner::Arguments_image_combiner() :
        in_folder_(Arguments_data_gen::DATA_FOLDER_DEF),
        dataset_idx_(Arguments_data_gen::DATASET_IDX_DEF),
        ground_truth_polygons_color_(COLOR_GROUND_TRUTH_POLYGONS),
        ground_truth_centers_of_mass_color_(COLOR_GROUND_TRUTH_CENTERS_OF_MASS),
        observation_polygons_color_(COLOR_OBSERVATION_POLYGONS),
        exploration_rate_(Arguments_inference_mh::STDS_EXP_DEF),
        chain_idx_(Arguments_inference_mh::CHAIN_IDX_DEF),
        chain_sample_idx_(Arguments_inference_mh::CHAIN_LEN_DEF),
        inference_polygons_color_(COLOR_INFERENCE_POLYGONS),
        inference_centers_of_mass_color_(COLOR_INFERENCE_CENTERS_OF_MASS)
{}

Arguments_image_combiner::Arguments_image_combiner(int argc, const char * const * const argv):
        Arguments_image_combiner()
{
    parse(argc, argv);
}

void Arguments_image_combiner::parse(int argc, const char *const *const argv)
{
    unsigned aggregation_ct = 0;
    for(int i = 0; i < argc; i++)
    {
        if(Arguments_data_gen::DATA_FOLDER_OPTION[0] == argv[i] || Arguments_data_gen::DATA_FOLDER_OPTION[1] == argv[i])
        {
            in_folder_ = argv[i + 1];
        }
        else if(Arguments_data_gen::DATASET_IDX_OPTION[0] == argv[i] || Arguments_data_gen::DATASET_IDX_OPTION[0] == argv[i])
        {
            dataset_idx_ = unsigned(std::stoul(argv[i + 1]));
        }
        else if(Arguments_inference_mh::STDS_EXP_OPTION[0] == argv[i] || Arguments_inference_mh::STDS_EXP_OPTION[1] == argv[i])
        {
            exploration_rate_ = unsigned(std::stoul(argv[i + 1]));
        }
        else if(Arguments_inference_mh::CHAIN_IDX_OPTION[0] == argv[i] || Arguments_inference_mh::CHAIN_IDX_OPTION[1] == argv[i])
        {
            chain_idx_ = unsigned(std::stoul(argv[i + 1]));
        }
        else if(Arguments_inference_mh::CHAIN_LEN_OPTION[0] == argv[i] || Arguments_inference_mh::CHAIN_LEN_OPTION[1] == argv[i])
        {
            chain_sample_idx_ = unsigned(std::stoul(argv[i + 1]));
        }
        else
        {
            continue;
        }
        i++;
    }
}

}
}

// global namespace for overloaded operators
std::istringstream &operator>>(std::istringstream &ss, fracture::cfg::Arguments_aggregator::Aggregation_type &o)
{
    std::string s_tot(ss.str());
    std::string s(s_tot.substr(ss.tellg(), s_tot.length() - ss.tellg()));
    for(size_t cur_agg_type_idx = 0; cur_agg_type_idx < fracture::cfg::Arguments_aggregator::AT_COUNT; cur_agg_type_idx++)
    {
        if(s.find(fracture::cfg::Arguments_aggregator::AT_STRS[cur_agg_type_idx]) == 0)
        {
            o = fracture::cfg::Arguments_aggregator::Aggregation_type(cur_agg_type_idx);
            ss.ignore(std::streamsize(fracture::cfg::Arguments_aggregator::AT_STRS[cur_agg_type_idx].length()));
            break;
        }
    }
}
