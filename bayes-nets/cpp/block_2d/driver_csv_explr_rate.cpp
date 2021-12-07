#include <boost/filesystem/path.hpp>

#include "config.hpp"
#include "sample.hpp"

namespace fracture
{
namespace block_2d
{
namespace driver_csv_explr_rate
{

void save_std_differences(
        const cfg::Arguments_aggregator &args,
        const std::vector<std::vector<double>> &chain_log_probs,
        double ground_truth_log_prob)
{
    using namespace fracture::util;
    boost::filesystem::path sample_instance_path = get_sample_path(args.in_folder_, args.dataset_.data_.no_aggregation_idx_);
    std::ostringstream fname;
    fname << pad_unsigned(args.dataset_.data_.no_aggregation_idx_)
          << "_" << util::INFERENCE_TYPE_STR[util::IT_METROPOLIS]
          << "_expl_rate_comparison.csv";
    sample_instance_path /= fname.str();
    std::ofstream f(
            sample_instance_path.string(),
            std::ios_base::out | std::ios_base::trunc
    );
    size_t i;
    size_t j;
    for(
            i = 0, j = args.exploration_rate_.data_.aggregation_range_.start_idx_;
            i < chain_log_probs.size();
            i++, j += args.exploration_rate_.data_.aggregation_range_.stride_)
    {
        if(i!=0) f << ",";
        f << "\"Expl. Rate " << j << " Mean\"";
    }
    f << ",\"True Log Probability\"\n";
    for(size_t j = 0; j < chain_log_probs[0].size(); j++)
    {
        for(size_t i = 0; i < chain_log_probs.size(); i++)
        {
            if(i!=0) f << ",";
            f << "\"" << chain_log_probs[i][j] << "\"";
        }
        f<<",\""<<ground_truth_log_prob<<"\"\n";
    }
}


}
}
}

int main(int argc, char *argv[])
{
    using namespace fracture;
    using namespace cfg;
    using namespace block_2d;
    using namespace driver_csv_explr_rate;
    // TODO implement a separate argument parser
    cfg::Arguments_aggregator args(argc, argv);

    Data_record_20191031 ground_truth;
    load_data_record(args.ground_truth_folder_, args.dataset_.data_.no_aggregation_idx_, ground_truth);
    double ground_truth_log_prob = ground_truth.sample_.log_prob();

    std::vector<Inference_record_20191031> results;
    std::vector<std::vector<std::pair<double, double>>> explr_rate_stats;
    for(
            unsigned explr_rate_idx = args.exploration_rate_.data_.aggregation_range_.start_idx_;
            explr_rate_idx <= args.exploration_rate_.data_.aggregation_range_.stop_idx_;
            explr_rate_idx += args.exploration_rate_.data_.aggregation_range_.stride_)
    {
        std::vector<unsigned> flex_vars = util::setup_mh_flex_vars(
                explr_rate_idx,
                args.chain_.data_.no_aggregation_idx_);
        Inference_record_20191031 ir;
        load_inference_record(
                args.in_folder_,
                args.dataset_.data_.no_aggregation_idx_,
                util::IT_METROPOLIS,
                flex_vars,
                ir);
        results.push_back(ir);
        // We want to aggregate multiple chains here, then look at the trends between different
        // standard deviation values.
        std::vector<std::vector<double>> cached_log_probs;
        // TODO fix this
        //cache_log_probs(results, cached_log_probs);

        // We want to keep all other things constant and only look at the chains, in this case,
        // to get a sense of whether the samples are getting stuck in local minima.
            // This becomes one table in a CSV.
        save_std_differences(
                    args,
                    cached_log_probs,
                    ground_truth_log_prob);
    }
}
