#include <boost/filesystem/path.hpp>

#include "config.hpp"
#include "sample.hpp"
#include "sample_vector_adapter.hpp"
#include "util.hpp"

namespace fracture
{
namespace block_2d
{
namespace driver_csv_hidden_rvs
{

void print_hidden_rvs_csv_record(std::ofstream &f, const Sample &s)
{
    Sample_vector_adapter sva;
    for(unsigned col = 0; col < sva.size(&s); col++)
    {
        if(col != 0) f << ",";
        f << "\"" << sva.get(&s, col) << "\"";
    }
}


void save_csv_hidden_rvs(
        const cfg::Arguments_aggregator &args,
        unsigned dataset_idx,
        unsigned explr_rate,
        unsigned chain_idx,
        const std::vector<Sample *> &chain,
        const Sample &ground_truth)
{
    using namespace fracture;
    using namespace util;
    using namespace block_2d;
    boost::filesystem::path sample_instance_path = get_sample_path(args.in_folder_, dataset_idx);
    std::ostringstream fname;
    fname << pad_unsigned(dataset_idx) << "_" << util::INFERENCE_TYPE_STR[util::IT_METROPOLIS] << "_" << pad_unsigned(explr_rate) << "_" << pad_unsigned(chain_idx) << "_hidden_rvs.csv";
    sample_instance_path /= fname.str();
    std::ofstream f(
            sample_instance_path.string(),
            std::ios_base::out | std::ios_base::trunc
    );

    Sample_vector_adapter sva;

    f << "\"Iteration\",";
    for(size_t inference_or_gt = 0; inference_or_gt < 2; inference_or_gt++)
    {
        for(size_t col = 0; col < sva.size(&ground_truth); col++)
        {
            if(col > 0) f << ",";
            f << "\"" << Rvs_idx_str[col] << "\"";
        }
        f << ",\"Log Probability\"";
        if(inference_or_gt == 0) f << ",\"\",";
    }
    f << "\n";
    for(size_t chain_sample_idx = 0; chain_sample_idx < chain.size(); chain_sample_idx++)
    {
        if(chain_sample_idx != 0 && chain[chain_sample_idx] == chain[chain_sample_idx-1]) continue;
        f << "\"" << chain_sample_idx << "\",";

        print_hidden_rvs_csv_record(f, *chain[chain_sample_idx]);
        f << ",\"" << chain[chain_sample_idx]->log_prob() << "\"";

        // put the ground truth somewhere on the spreadsheet
        if(chain_sample_idx == 0)
        {
            f << ",\"\",";
            print_hidden_rvs_csv_record(f, ground_truth);
            f << ",\"" << ground_truth.log_prob() << "\"";
        }
        f << "\n";
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
    using namespace driver_csv_hidden_rvs;
    // TODO implement a separate argument parser
    cfg::Arguments_aggregator args(argc, argv);

    assert(!args.dataset_.aggregate_);
    assert(!args.exploration_rate_.aggregate_);
    assert(!args.chain_.aggregate_);

    Sample ground_truth;

    if(args.data_archive_ver_ >= 20191031)
    {
        Data_record_20191031 gtdr;
        load_data_record(args.ground_truth_folder_, args.dataset_.data_.no_aggregation_idx_, gtdr);
        ground_truth = gtdr.sample_;
        std::cout << gtdr.rng_seed_ << "\n";
    }
    else
    {
        load_sample(args.ground_truth_folder_, args.dataset_.data_.no_aggregation_idx_, ground_truth);
    }

    double ground_truth_log_prob = ground_truth.log_prob();

    std::vector<Sample *> results;
    std::vector<unsigned> flex_vars = util::setup_mh_flex_vars(
            args.exploration_rate_.data_.no_aggregation_idx_,
            args.chain_.data_.no_aggregation_idx_);
    if(args.inference_archive_ver_ >= 20191031)
    {
        Inference_record_20191031 ir;
        load_inference_record(
                args.in_folder_,
                args.dataset_.data_.no_aggregation_idx_,
                util::IT_METROPOLIS,
                flex_vars,
                ir);
        results = ir.samples_;
    }
    else
    {
        load_sample_vector(
                args.in_folder_,
                args.dataset_.data_.no_aggregation_idx_,
                util::IT_METROPOLIS,
                flex_vars,
                results);
    }
    save_csv_hidden_rvs(
                args,
                args.dataset_.data_.no_aggregation_idx_,
                args.exploration_rate_.data_.no_aggregation_idx_,
                args.chain_.data_.no_aggregation_idx_,
                results,
                ground_truth);
}
