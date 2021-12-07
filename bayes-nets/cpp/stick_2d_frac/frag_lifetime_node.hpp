#ifndef FRAC_LIFETIME_NODE_HPP
#define FRAC_LIFETIME_NODE_HPP

#include <memory>

#include <boost/serialization/access.hpp>

#include "stick_tree_structure.hpp"

namespace stick_2d_frac { namespace sample {

class Frag_lifetime_node {
    friend class boost::serialization::access;
public:
    // Forward sample
    Frag_lifetime_node(const Stick_tree_structure & structure, unsigned subtree_start_time, unsigned subtree_end_time);
    ~Frag_lifetime_node();

    unsigned get_start_time() const { return start_t_; }
    unsigned get_end_time() const { return end_t_; }
    unsigned get_lifetime() const { return lifetime_; }
    bool is_leaf() const { return !l_; }
    // May return null if we're a leaf.
    const Frag_lifetime_node * get_left() const { return l_; }
    const Frag_lifetime_node * get_right() const { return r_; }

    double log_prior() const;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & l_ & r_ & start_t_ & end_t_ & lifetime_;
    }
private:
    // May be nullptr if we're a leaf.
    Frag_lifetime_node * l_;
    Frag_lifetime_node * r_;

    unsigned start_t_;
    unsigned end_t_;
    unsigned lifetime_;
};

}}

#endif // FRAC_LIFETIME_NODE_HPP
