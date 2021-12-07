#include <vector>
#include <thread>

#include <boost/archive/text_iarchive.hpp>

#include "config.hpp"
#include "sample.hpp"

/**
 *  The idea behind this driver is that we essentially have a multidimensional array of data,
 *  parameterized by hyperparameters of the model, such as the chain length, the number of
 *  chains, and the standard deviations and by different data sets, themselves.
 *  We want to define a method whereby we can select a single dimension of the data,
 *  a method of aggregation, and be able to collapse that dimension down. We want to collapse
 *  the dimensions down, one at a time, using as much parallel processing as possible
 *  to ensure that the this is done in a reasonable amount of time.
 */

namespace fracture
{
namespace block_2d
{
namespace driver_aggregator
{

// TODO figure out a better way to do this. Aggregation only makes sense over a metric.
// It makes less sense over samples. What's an average sample? Does it average all rvs?

//// assumes that same-pointers can only occur in series.
//// duplicate pointers basically comes from the Metropolis-Hastings rejections.
//void free_chain_samples(std::vector<Sample *> & chain)
//{
//    Sample *prev = nullptr;
//    for(Sample *cur : chain)
//    {
//        if(cur != prev)
//        {
//            prev = cur;
//            delete cur;
//        }
//    }
//}

//// If the at is AT_MEAN, the caller is responsible for properly freeing the memory.
//Sample *aggregate_bin(
//        const std::vector<Sample *> &chain,
//        const cfg::Arguments_aggregator::Aggregation_type &at,
//        unsigned start,
//        unsigned guard,
//        unsigned stride)
//{
//    switch(at)
//    {
//    case cfg::Arguments_aggregator::AT_FIRST:
//        return chain[start];
//    case cfg::Arguments_aggregator::AT_MEDIAN:
//        return chain[(start + guard) / 2];
//    case cfg::Arguments_aggregator::AT_LAST:
//        return chain[guard - 1];
//    }
//    Sample *r = nullptr;
//    double r_log_prob;
//    for(unsigned cur_elem = start; cur_elem < guard; cur_elem += stride)
//    {
//        Sample *cur = chain[cur_elem];
//        double considered_log_prob;
//        if(cur_elem == start)
//        {
//            switch(at)
//            {
//            case cfg::Arguments_aggregator::AT_MAX:
//            case cfg::Arguments_aggregator::AT_MIN:
//                r = cur;
//                break;
//            case cfg::Arguments_aggregator::AT_MEAN:
//                // In case we attempt to add values, but subsequent samples
//                // in the input vector also point to the same value.
//                // Would mess up results.
//                r = new Sample(*cur);
//                break;
//            }
//            r_log_prob = r->log_prob();
//        }
//        else
//        {
//            switch(at)
//            {
//            case cfg::Arguments_aggregator::AT_MAX:
//                considered_log_prob = cur->log_prob();
//                if(considered_log_prob > r_log_prob)
//                {
//                    r = cur;
//                    r_log_prob = considered_log_prob;
//                }
//                break;
//            case cfg::Arguments_aggregator::AT_MIN:
//                considered_log_prob = cur->log_prob();
//                if(considered_log_prob < r_log_prob)
//                {
//                    r = cur;
//                    r_log_prob = considered_log_prob;
//                }
//                break;
//            case cfg::Arguments_aggregator::AT_MEAN:
//                r->add_rvs(*cur);
//                break;
//            default:
//                throw util::Unhandled_enum_value_exception(); //util::err_str(__FILE__, __LINE__);
//            }
//        }
//    }
//    if(at == cfg::Arguments_aggregator::AT_MEAN)
//        r->div_rvs(guard - start);
//}

//void aggregate_vector(
//        const std::vector<Sample *> &in,
//        const cfg::Arguments_aggregator::Possible_aggregation &aggregation_options,
//        std::vector<Sample *> &out)
//{
//    assert(aggregation_options.aggregate_);
//    unsigned num_elems = (aggregation_options.data_.aggregation_range_.stop_idx_ + 1 - aggregation_options.data_.aggregation_range_.start_idx_)
//            / aggregation_options.data_.aggregation_range_.stride_;
//    unsigned num_bins = unsigned(std::ceil(num_elems
//            / double(aggregation_options.data_.aggregation_range_.bin_size_)));
//    out.reserve(num_bins);
//    for(unsigned cur_bin = 0; cur_bin < num_bins; cur_bin++)
//    {
//        unsigned start = cur_bin * aggregation_options.data_.aggregation_range_.bin_size_;
//        unsigned guard = std::min((cur_bin + 1) * aggregation_options.data_.aggregation_range_.bin_size_, num_elems);
//        out.push_back(aggregate_bin(
//                in,
//                aggregation_options.data_.aggregation_range_.type_,
//                start,
//                guard,
//                aggregation_options.data_.aggregation_range_.stride_));
//    }
//}

//// Each inner vector represents a single MCMC chain.
//// Our goal is to aggregate along the other dimension (along the outer vector).
//// precondition: all inner vectors have the same size.
//void aggregate_vector_of_vectors(
//        const std::vector<std::vector<Sample *>> &in,
//        const cfg::Arguments_aggregator::Possible_aggregation &aggregation_options,
//        std::vector<Sample *> &out)
//{
//    assert(aggregation_options.aggregate_);
//    out.reserve(in[0].size());
//    for(unsigned sample_idx = 0; sample_idx < in[0].size(); sample_idx++)
//    {
//        std::vector<Sample *> cur_sample_in_vec;
//        for(unsigned i = 0; i < in.size(); i++)
//        {
//            cur_sample_in_vec.push_back(in[i][sample_idx]);
//        }
//        std::vector<Sample *> cur_sample_out_vec;
//        aggregate_vector(cur_sample_in_vec, aggregation_options, cur_sample_out_vec);
//        assert(cur_sample_out_vec.size() == 1);
//        out.push_back(cur_sample_out_vec[0]);
//    }
//}

//void perform_chain_sample_aggregation(const cfg::Arguments_aggregator &args)
//{
//    std::vector<unsigned> flex_vars = util::setup_flex_vars(
//            args.exploration_rate_.data_.no_aggregation_idx_,
//            args.chain_.data_.no_aggregation_idx_);
//    std::vector<Sample *> chain;
//    load_inference_record(
//            args.in_folder_,
//            args.dataset_.data_.no_aggregation_idx_,
//            util::IT_METROPOLIS,
//            flex_vars,
//            chain);
//    std::vector<Sample *> aggregated;
//    aggregate_vector(
//            chain,
//            args.chain_sample_,
//            aggregated);
//    save_inference_record(
//            args.out_folder_,
//            args.dataset_.data_.no_aggregation_idx_,
//            util::IT_METROPOLIS,
//            flex_vars,
//            aggregated);
//    if(args.chain_sample_.data_.aggregation_range_.type_ == cfg::Arguments_aggregator::AT_MEAN)
//        free_chain_samples(aggregated);
//    free_chain_samples(chain);
//}

//void perform_chain_aggregation(const cfg::Arguments_aggregator &args)
//{
//    std::vector<unsigned> fv;
//    std::vector<std::vector<Sample *>> chains;
//    for(
//            unsigned chain_cur = args.chain_.data_.aggregation_range_.start_idx_;
//            chain_cur <= args.chain_.data_.aggregation_range_.stop_idx_;
//            chain_cur += args.chain_.data_.aggregation_range_.stride_)
//    {
//        fv = util::setup_flex_vars(
//                args.exploration_rate_.data_.no_aggregation_idx_,
//                chain_cur);
//        chains.push_back(std::vector<Sample *>());
//        load_inference_record(
//                args.in_folder_,
//                args.dataset_.data_.no_aggregation_idx_,
//                util::IT_METROPOLIS,
//                fv,
//                chains[chains.size() - 1]);
//    }

//    std::vector<Sample *> out;
//    aggregate_vector_of_vectors(chains, args.chain_, out);

//    fv = util::setup_flex_vars(
//            args.exploration_rate_.data_.no_aggregation_idx_,
//            args.chain_.data_.aggregation_range_.start_idx_);
//    save_inference_record(
//            args.out_folder_,
//            args.dataset_.data_.no_aggregation_idx_,
//            util::IT_METROPOLIS,
//            fv,
//            out);
//}


//void perform_exploration_rate_aggregation(cfg::Arguments_aggregator &args)
//{
//    std::vector<unsigned> fv;
//    std::vector<std::vector<Sample *>> chains;
//    for(
//            unsigned expl_rate_cur = args.exploration_rate_.data_.aggregation_range_.start_idx_;
//            expl_rate_cur <= args.exploration_rate_.data_.aggregation_range_.stop_idx_;
//            expl_rate_cur += args.exploration_rate_.data_.aggregation_range_.stride_)
//    {
//        fv = util::setup_flex_vars(
//                expl_rate_cur,
//                args.chain_.data_.no_aggregation_idx_);
//        chains.push_back(std::vector<Sample *>());
//        load_inference_record(
//                args.in_folder_,
//                args.dataset_.data_.no_aggregation_idx_,
//                util::IT_METROPOLIS,
//                fv,
//                chains[chains.size() - 1]);
//    }

//    std::vector<Sample *> out;
//    aggregate_vector_of_vectors(chains, args.exploration_rate_, out);

//    fv = util::setup_flex_vars(
//            args.exploration_rate_.data_.aggregation_range_.start_idx_,
//            args.chain_.data_.no_aggregation_idx_);
//    save_inference_record(
//            args.out_folder_,
//            args.dataset_.data_.no_aggregation_idx_,
//            util::IT_METROPOLIS,
//            fv,
//            out);
//}

//void perform_dataset_aggregation(cfg::Arguments_aggregator &args)
//{
//    std::vector<unsigned> fv = util::setup_flex_vars(
//            args.exploration_rate_.data_.no_aggregation_idx_,
//            args.chain_.data_.no_aggregation_idx_);
//    std::vector<std::vector<Sample *>> chains;
//    for(
//            unsigned dataset_cur = args.dataset_.data_.aggregation_range_.start_idx_;
//            dataset_cur <= args.dataset_.data_.aggregation_range_.stop_idx_;
//            dataset_cur += args.dataset_.data_.aggregation_range_.stride_)
//    {
//        chains.push_back(std::vector<Sample *>());
//        load_inference_record(
//                args.in_folder_,
//                dataset_cur,
//                util::IT_METROPOLIS,
//                fv,
//                chains[chains.size() - 1]);
//    }
//    std::vector<Sample *> out;
//    aggregate_vector_of_vectors(chains, args.dataset_, out);
//    save_inference_record(
//            args.out_folder_,
//            args.dataset_.data_.aggregation_range_.start_idx_,
//            util::IT_METROPOLIS,
//            fv,
//            out);
//}

}
}
}


int main(int argc, char *argv[])
{
//    using namespace fracture;
//    using namespace fracture::block_2d::driver_aggregator;
//    cfg::Arguments_aggregator args(argc, argv);
//    if(args.dataset_.aggregate_)
//    {
//        perform_dataset_aggregation(args);
//    }
//    else if(args.exploration_rate_.aggregate_)
//    {
//        perform_exploration_rate_aggregation(args);
//    }
//    else if(args.chain_.aggregate_)
//    {
//        perform_chain_aggregation(args);
//    }
//    else if(args.chain_sample_.aggregate_)
//    {
//        perform_chain_sample_aggregation(args);
//    }
}
