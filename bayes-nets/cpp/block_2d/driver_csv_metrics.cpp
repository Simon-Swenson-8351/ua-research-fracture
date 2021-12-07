// Idea here is to grab the best sample from each chain in terms of log probability,
// then look at the values of their 8 hidden variables.

#include <boost/filesystem/path.hpp>

#include "config.hpp"
#include "sample.hpp"
#include "util.hpp"
#include "sample_vector_adapter.hpp"

namespace fracture
{
namespace block_2d
{
namespace driver_csv_metrics
{

void save_chain_differences(
        const cfg::Arguments_aggregator &args,
        unsigned data_idx,
        unsigned expl_rate,
        const std::vector<Sample *> &chains)
{
    using namespace fracture::util;
    boost::filesystem::path sample_instance_path = get_sample_path(args.in_folder_, data_idx);
    std::ostringstream fname;
    fname << pad_unsigned(data_idx) << "_" << util::INFERENCE_TYPE_STR[util::IT_METROPOLIS] << "_" << pad_unsigned(expl_rate) << "_chain_comparison.csv";
    sample_instance_path /= fname.str();
    std::ofstream f(
            sample_instance_path.string(),
            std::ios_base::out | std::ios_base::trunc
    );

    f << "\"Chain\"";
    block_2d::Sample_vector_adapter sva;
    for(size_t hidden_rvs_idx = 0; hidden_rvs_idx < sva.size(chains[0]); hidden_rvs_idx++)
    {
        f << ",\"" << block_2d::Rvs_idx_str[hidden_rvs_idx] << "\"";
    }
    f<<",\"True Log Probability\"\n";

    for(size_t chain_idx = 0; chain_idx < chains.size(); chain_idx++)
    {
        f << "\"" << chain_idx << "\"";
        for(size_t hidden_rvs_idx = 0; hidden_rvs_idx < sva.size(chains[0]); hidden_rvs_idx++)
        {
            f << ",\"" << sva.get(chains[chain_idx], hidden_rvs_idx) << "\"";
        }
        f << ",\"" << chains[chain_idx]->log_prob() << "\"\n";
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
    using namespace driver_csv_metrics;
    // TODO implement a separate argument parser
    cfg::Arguments_aggregator args(argc, argv);

    Sample ground_truth;

    if(args.data_archive_ver_ >= 20191031)
    {
        Data_record_20191031 gtdr;
        load_data_record(args.ground_truth_folder_, args.dataset_.data_.no_aggregation_idx_, gtdr);
        ground_truth = gtdr.sample_;
    }
    else
    {
        load_sample(args.ground_truth_folder_, args.dataset_.data_.no_aggregation_idx_, ground_truth);
    }

    std::vector<Sample *> results;
    for(
            unsigned chain_idx = args.chain_.data_.aggregation_range_.start_idx_;
            chain_idx <= args.chain_.data_.aggregation_range_.stop_idx_;
            chain_idx += args.chain_.data_.aggregation_range_.stride_)
    {
        std::vector<unsigned> flex_vars = util::setup_mh_flex_vars(
                args.exploration_rate_.data_.no_aggregation_idx_,
                chain_idx);
        std::vector<Sample *> inference_chain;
        if(args.inference_archive_ver_ >= 20191031)
        {
            Inference_record_20191031 ir;
            load_inference_record(
                    args.in_folder_,
                    args.dataset_.data_.no_aggregation_idx_,
                    util::IT_METROPOLIS,
                    flex_vars,
                    ir);
            inference_chain = ir.samples_;
        }
        else
        {
            load_sample_vector(
                    args.in_folder_,
                    args.dataset_.data_.no_aggregation_idx_,
                    util::IT_METROPOLIS,
                    flex_vars,
                    inference_chain);
        }
        results.push_back(inference_chain[inference_chain.size() - 1]);
    }
    // We want to keep all other things constant and only look at the chains, in this case,
    // to get a sense of whether the samples are getting stuck in local minima.
    // This becomes one table in a CSV.
    save_chain_differences(
                args,
                args.dataset_.data_.no_aggregation_idx_,
                args.exploration_rate_.data_.no_aggregation_idx_,
                results);
}
