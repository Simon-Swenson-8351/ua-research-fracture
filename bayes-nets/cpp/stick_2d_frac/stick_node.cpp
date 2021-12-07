#include <vector>

#include <prob_cpp/prob_sample.h>

#include "stick_node.hpp"
#include "hidden_stick_state.hpp"
#include "prob.hpp"

namespace stick_2d_frac { namespace sample {

Stick_node::Stick_node(
        const Stick_tree_structure & tree_structure,
        const Frag_lifetime_node & frac_tree,
        const Stick_length_info & stick_len,
        const Hidden_stick_state & init_state,
        const Camera & c,
        const kjb::Normal_distribution & observed_image_endpoints_offset_dist):
    stick_len_(stick_len)
{
    hss_.reserve(size_t(frac_tree.get_lifetime()));
    oss_.reserve(size_t(frac_tree.get_lifetime()));
    hss_.push_back(init_state);
    oss_.push_back(Observed_stick_state(hss_[0].get_image_endpoints_homo(), observed_image_endpoints_offset_dist));
    for(unsigned i = 1; i < frac_tree.get_lifetime(); i++) {
        hss_.push_back(Hidden_stick_state(hss_[i - 1], stick_len_, c, 1));
        oss_.push_back(Observed_stick_state(hss_[i].get_image_endpoints_homo(), observed_image_endpoints_offset_dist));
    }
    if(tree_structure.get_left())
    {

        double fr_loc(prob::sample(*prob::new_frac_loc_i_dist(stick_len.get_length())));
        // Use the constructor of Hidden_stick_state for when a fracture happens
        l_ = new Stick_node
        (
            *(tree_structure.get_left()),
            *(frac_tree.get_left()),
            fr_loc,
            Hidden_stick_state
            (
                hss_[hss_.size() - 1],
                stick_len,
                c,
                fr_loc,
                Hidden_stick_state::FS_LEFT
            ),
            c,
            observed_image_endpoints_offset_dist
        );
        r_ = new Stick_node
        (
            *(tree_structure.get_right()),
            *(frac_tree.get_right()),
            fr_loc,
            Hidden_stick_state
            (
                hss_[hss_.size() - 1],
                stick_len,
                c,
                fr_loc,
                Hidden_stick_state::FS_RIGHT
            ),
            c,
            observed_image_endpoints_offset_dist
        );
    }
    else
    {
        l_ = nullptr;
        r_ = nullptr;
    }
}

Stick_node::~Stick_node() {
    if(l_) {
        delete l_;
        delete r_;
    }
}

void Stick_node::get_all_active_hidden_states(const Frag_lifetime_node & fln, unsigned timestamp, std::vector<Hidden_stick_state> & out) const
{
    if(fln.get_end_time() > timestamp && fln.get_start_time() <= timestamp)
    {
        out.push_back(hss_[timestamp - fln.get_start_time()]);
    }
    else if(fln.get_end_time() <= timestamp && l_)
    {
        l_->get_all_active_hidden_states(*(fln.get_left()), timestamp, out);
        r_->get_all_active_hidden_states(*(fln.get_right()), timestamp, out);
    }
}

void Stick_node::get_all_active_observed_states(const Frag_lifetime_node & fln, unsigned timestamp, std::vector<Observed_stick_state> & out) const
{
    if(fln.get_end_time() > timestamp && fln.get_start_time() <= timestamp)
    {
        out.push_back(oss_[timestamp - fln.get_start_time()]);
    }
    else if(fln.get_end_time() <= timestamp && l_)
    {
        l_->get_all_active_observed_states(*(fln.get_left()), timestamp, out);
        r_->get_all_active_observed_states(*(fln.get_right()), timestamp, out);
    }
}

void Stick_node::get_all_active_hidden_lines(const Frag_lifetime_node & fln, unsigned timestamp, std::vector<kjb::Matrix> & out) const
{
    std::vector<Hidden_stick_state> temp_vec;
    get_all_active_hidden_states(fln, timestamp, temp_vec);
    for(const Hidden_stick_state & hss : temp_vec)
    {
        out.push_back(*util::homo_col_vecs_to_non_homo_col_vecs(hss.get_image_endpoints_homo()));
    }
}

void Stick_node::get_all_active_observed_lines(const Frag_lifetime_node & fln, unsigned timestamp, std::vector<kjb::Matrix> & out) const
{
    std::vector<Observed_stick_state> temp_vec;
    get_all_active_observed_states(fln, timestamp, temp_vec);
    for(const Observed_stick_state & oss : temp_vec)
    {
        out.push_back(*util::homo_col_vecs_to_non_homo_col_vecs(oss.get_image_endpoints_homo()));
    }
}

size_t Stick_node::get_num_nonleaf_nodes() const
{
    if(!l_)
    {
        return 0;
    }
    return 1 + l_->get_num_nonleaf_nodes() + r_->get_num_nonleaf_nodes();
}

size_t Stick_node::get_total_number_of_states() const
{
    size_t result(hss_.size());
    if(l_)
    {
        result += l_->get_total_number_of_states() + r_->get_total_number_of_states();
    }
    return result;
}

const Stick_node * Stick_node::get_nonleaf_node_by_index(size_t & idx) const
{
    if(!l_)
        return nullptr;
    if(idx == 0)
    {
        return this;
    }
    idx -= 1;
    const Stick_node * result(l_->get_nonleaf_node_by_index(idx));
    if(!result)
        result = r_->get_nonleaf_node_by_index(idx);
    return result;
}

void Stick_node::set_length(double val, const Camera & c) {
    Stick_length_info temp(val);
    propagate_hidden_variables(c, &temp, nullptr);
}

void Stick_node::set_initial_state_element(
        Hidden_stick_state::State_variable sv,
        double val,
        const Camera & c)
{
    hss_[0].set_state_element(sv, val, stick_len_.get_local_endpoints_homo(), c);
    kjb::Vector temp_vec(hss_[0].get_state());
    temp_vec[sv] = val;
    Hidden_stick_state temp_hss(hss_[0]);

    propagate_hidden_variables(c, nullptr, &temp_hss);
}

void Stick_node::set_fracture_location(size_t idx, double val, const Camera & c)
{
    Stick_node *parent = get_nonleaf_node_by_index(idx);
    Stick_length_info sli_l(val);
    Stick_length_info sli_r(stick_len_.get_length() - val);
    Hidden_stick_state hss_l(parent->hss_[hss_.size() - 1], stick_len_, c, val, Hidden_stick_state::FS_LEFT);
    Hidden_stick_state hss_r(parent->hss_[hss_.size() - 1], stick_len_, c, val, Hidden_stick_state::FS_RIGHT);
    parent->get_left()->propagate_hidden_variables(c, &sli_l, &hss_l);
    parent->get_right()->propagate_hidden_variables(c, &sli_r, &hss_r);
}

double Stick_node::log_prior(const Camera & cam, bool is_root) const
{
    double result(0.0);
    if(is_root)
    {
        // Our length is conditioned on the camera
        // TODO instead of creating a new distribution, maybe save it in the class object?
        result += prob::pdf(*prob::new_len_0_dist(cam.get_camera_width(), cam.get_camera_height()), stick_len_.get_length());

        // Don't forget to do the prior for the initial state variables
        result+= hss_[0].log_prior(cam);
    }
    if(l_)
    {
        // Just have sticks handle their fracture location random variables as well.
        result += prob::pdf(*prob::new_frac_loc_i_dist(stick_len_.get_length()), l_->get_length_info().get_length());
        result += l_->log_prior(cam, false) + r_->log_prior(cam, false);
    }
    return result;
}

double Stick_node::log_likelihood(const kjb::Normal_distribution & offset_dist) const
{
    double result(0.0);
    assert(oss_.size() == hss_.size());
    for(size_t i = 0; i < oss_.size(); i++)
    {
        result += oss_[i].log_likelihood(hss_[i], offset_dist);
    }
    if(l_)
    {
        result += l_->log_likelihood(offset_dist) + r_->log_likelihood(offset_dist);
    }
    return result;
}

void Stick_node::propagate_hidden_variables(const Camera & c, const Stick_length_info * my_length, const Hidden_stick_state * initial_state)
{
    if(initial_state)
    {
        hss_[0] = *initial_state;
    }
    if(my_length)
    {
        stick_len_ = *my_length;
    }
    for(size_t i = 1; i < hss_.size(); i++)
    {
        hss_[i] = Hidden_stick_state(hss_[i - 1], stick_len_, c);
    }
    if(l_)
    {
        Hidden_stick_state hss_l(hss_[hss_.size() - 1], stick_len_, c, l_->get_length_info().get_length(), Hidden_stick_state::FS_LEFT);
        Hidden_stick_state hss_r(hss_[hss_.size() - 1], stick_len_, c, l_->get_length_info().get_length(), Hidden_stick_state::FS_RIGHT);
        l_->propagate_hidden_variables(c, nullptr, &hss_l);
        if(my_length)
        {
            Stick_length_info sli_r(stick_len_.get_length() - l_->get_length_info().get_length());
            r_->propagate_hidden_variables(c, &sli_r, &hss_r);
        }
        else
        {
            r_->propagate_hidden_variables(c, nullptr, &hss_r);
        }
    }
}

}}
