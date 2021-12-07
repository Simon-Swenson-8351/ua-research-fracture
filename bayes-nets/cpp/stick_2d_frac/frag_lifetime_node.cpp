#include <prob_cpp/prob_sample.h>

#include "frag_lifetime_node.hpp"

namespace stick_2d_frac { namespace sample {

// Additional fractures tend to happen soon after the initial fracture.
static const kjb::Geometric_distribution frac_time_i_dist(0.5);
Frag_lifetime_node::Frag_lifetime_node(
        const Stick_tree_structure & structure,
        unsigned subtree_start_time,
        unsigned subtree_end_time):
    start_t_(subtree_start_time)
{
    if(subtree_start_time >= subtree_end_time)
        throw "subtree_start_time >= subtree_end_time";
    if(structure.is_leaf())
    {
        end_t_ = subtree_end_time;
        l_ = nullptr;
        r_ = nullptr;
    }
    else
    {
        // The underlying implementation of kjb::Geometric_distribution is
        // boost::math::Geometric_distribution, which does not use integers
        // for its underlying calculations. We need to cast this to an int.
        // This may very rarely overflow, since geometric distributions are
        // unbounded, but I think this chance is so astronomically small
        // that we don't need to worry about.

        // The reason we add 1.0 here is that end_t_ is excluded, so, for the
        // stick to have a life time of at least one frame, we need to ensure
        // that end_t_ is at least 1.
        end_t_ = subtree_start_time + unsigned(kjb::sample(frac_time_i_dist) + 1);
        l_ = new Frag_lifetime_node(
                *structure.get_left(),
                end_t_,
                subtree_end_time);
        r_ = new Frag_lifetime_node(
                *structure.get_right(),
                end_t_,
                subtree_end_time);
    }
    lifetime_ = end_t_ - start_t_;
}

Frag_lifetime_node::~Frag_lifetime_node()
{
    if(l_) {
        delete l_;
        delete r_;
    }
}

double Frag_lifetime_node::log_prior() const
{
    if(!l_)
    {
        return 0; // No fracture time for leaves, so don't modify the result.
    }
    else
    {
        return kjb::pdf(frac_time_i_dist, double(lifetime_)) + l_->log_prior() + r_->log_prior();
    }
}

}}
