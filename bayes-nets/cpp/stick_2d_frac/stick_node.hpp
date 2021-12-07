#ifndef FRAC_LOC_NODE_HPP
#define FRAC_LOC_NODE_HPP

#include <memory>

#include <boost/serialization/access.hpp>

#include "prob.hpp"
#include "stick_tree_structure.hpp"
#include "frag_lifetime_node.hpp"
#include "hidden_stick_state.hpp"
#include "observed_stick_state.hpp"
#include "stick_length_info.hpp"

namespace stick_2d_frac { namespace sample {

class Stick_node {
    friend class boost::serialization::access;
public:
    Stick_node(
            const Stick_tree_structure & tree_structure,
            const Frag_lifetime_node & frac_tree,
            const Stick_length_info & stick_len,
            const Hidden_stick_state & init_state,
            const Camera & c,
            const kjb::Normal_distribution & observed_image_endpoints_offset_dist);
    ~Stick_node();

    const Stick_length_info & get_length_info() const { return stick_len_; }
    // state_index must be the difference between a given time stamp and
    // this node's start time. This class cannot calculate this on its own,
    // since it does not know the start time on account of that being part of
    // a Frag_lifetime_node.
    const Hidden_stick_state & get_hidden_stick_state(unsigned state_index) const { return hss_[state_index]; }
    const Observed_stick_state & get_observed_stick_state(unsigned state_index) const { return oss_[state_index]; }
    bool is_leaf() const { return !l_; }
    // May return null if this is a leaf
    const Stick_node * get_left() const { return l_; }
    Stick_node * get_left() { return l_; }
    const Stick_node * get_right() const { return r_;}
    Stick_node * get_right() { return r_;}

    // need the Frag_lifetime_node to calculate the array index from the timestamp.
    void get_all_active_hidden_states(const Frag_lifetime_node & fln, unsigned timestamp, std::vector<Hidden_stick_state> & out) const;
    void get_all_active_observed_states(const Frag_lifetime_node & fln, unsigned timestamp, std::vector<Observed_stick_state> & out) const;

    // Basically a wrapper around get_all_active_hidden_states that also returns non-homogeneous matrices, ready to be drawn.
    void get_all_active_hidden_lines(const Frag_lifetime_node & fln, unsigned timestamp, std::vector<kjb::Matrix> & out) const;
    void get_all_active_observed_lines(const Frag_lifetime_node & fln, unsigned timestamp, std::vector<kjb::Matrix> & out) const;

    size_t get_num_nonleaf_nodes() const;
    size_t get_total_number_of_states() const;

    const Stick_node * get_nonleaf_node_by_index(size_t & idx) const;
    Stick_node * get_nonleaf_node_by_index(size_t & idx)
    {
        return const_cast<Stick_node *>(const_cast<const Stick_node *>(this)->get_nonleaf_node_by_index(idx));
    }

    // Also propagate the changes to all hidden states in the stick tree.
    // ONLY CALL THIS ON THE ROOT
    void set_length(double val, const Camera & c);
    // ONLY CALL THIS ON THE ROOT
    void set_initial_state_element(Hidden_stick_state::State_variable sv, double val, const Camera & c);
    void set_fracture_location(size_t idx, double val, const Camera & c);

    double log_prior(const Camera & cam, bool is_root = true) const;
    double log_likelihood(const kjb::Normal_distribution & offset_dist) const;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int)
    {
        ar & stick_len_ & hss_ & oss_ & l_ & r_;
    }
private:
    // Either internal length, initial state, or both will be set, depending on
    // if null was passed for the arguments or not. If both are null, this is
    // a no-op.
    void propagate_hidden_variables(const Camera & c, const Stick_length_info * my_length = nullptr, const Hidden_stick_state * initial_state = nullptr);
    Stick_length_info stick_len_;
    std::vector<Hidden_stick_state> hss_;
    std::vector<Observed_stick_state> oss_;
    // Both of these will be null if we're a leaf.
    Stick_node * l_;
    Stick_node * r_;
};

}}

#endif // FRAC_LOC_NODE_HPP
