#ifndef DISCRETE_SAMPLE_HPP
#define DISCRETE_SAMPLE_HPP

#include <boost/serialization/access.hpp>

#include "stick_tree_structure.hpp"
#include "frag_lifetime_node.hpp"

namespace stick_2d_frac { namespace sample {

class Discrete_sample {
    friend class boost::serialization::access;
public:
    // Uses forward sampling to initialize the object
    Discrete_sample(unsigned num_ims) :
        // Let's leave things simpler and stick with a set depth for now.
        tree_structure_(3),
        // Use the structure to sample the stick fracture times.
        frag_lifetime_tree_(tree_structure_, 0, num_ims) {}

    const Stick_tree_structure & get_stick_tree_structure() const { return tree_structure_; }
    const Frag_lifetime_node & get_frac_lifetime_tree() const { return frag_lifetime_tree_; }

    double log_prior() const { return get_frac_lifetime_tree().log_prior(); }

    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & tree_structure_ & frag_lifetime_tree_;
    }
private:
    // These are both recursive structures, so they can't be nested easily. Just use
    // smart pointers in this case.
    Stick_tree_structure tree_structure_;
    Frag_lifetime_node frag_lifetime_tree_;
};

}}

#endif // DISCRETE_SAMPLE_HPP
