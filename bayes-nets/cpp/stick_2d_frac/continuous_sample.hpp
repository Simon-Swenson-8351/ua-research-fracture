#ifndef CONTINUOUS_SAMPLE_HPP
#define CONTINUOUS_SAMPLE_HPP

#include <memory>
#include <numeric>

#include <boost/serialization/access.hpp>

#include <m_cpp/m_matrix.h>

#include "prob.hpp"
#include "discrete_sample.hpp"
#include "stick_node.hpp"
#include "stick_camera.hpp"
#include "hidden_stick_state.hpp"
#include "stick_length_info.hpp"

namespace stick_2d_frac { namespace sample {

class Continuous_sample {
    friend class boost::serialization::access;
    friend class Continuous_sample_vector_adapter;
public:
    static constexpr double A_Y_IN_M_S_SQ = -9.8;

    // Creates new continuous variables using forward sampling from the given
    // DiscreteSample.
    Continuous_sample(const Discrete_sample &ds, unsigned im_w, unsigned im_h, double cam_fps) :
        cam_(im_w, im_h, cam_fps),
        initial_len_(sample_len_0(cam_)),
        stick_tree_(
                ds.get_stick_tree_structure(),
                ds.get_frac_lifetime_tree(),
                initial_len_,
                *sample_s_0(initial_len_, cam_),
                cam_,
                *(prob::new_noise_offset_dist(im_w, im_h)))
    {
    }

    const Camera & get_camera() const { return cam_; }
    Camera & get_camera() { return cam_; }
    const Stick_node & get_stick_tree() const { return stick_tree_; }
    Stick_node & get_stick_tree() { return stick_tree_; }

    void set_initial_length(double val);
    void set_fracture_location(size_t idx, double val);
    void set_initial_state_element(sample::Hidden_stick_state::State_variable sv, double val);

    double log_prior() const { return cam_.log_prior() + stick_tree_.log_prior(cam_); }
    double log_likelihood() const
    {
        std::unique_ptr<kjb::Normal_distribution> offset_dist(prob::new_noise_offset_dist(cam_.get_image_width(), cam_.get_image_height()));
        return stick_tree_.log_likelihood(*offset_dist);
    }
    double log_posterior() const
    {
        return log_prior() + log_likelihood();
    }

    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & cam_ & initial_len_ & stick_tree_;
    }
private:
    // These two are used in the constructor to allow us to use the initialization list for stick_tree_, other-
    // wise, my IDE was complaining.
    static double sample_len_0(const Camera & c);
    static std::unique_ptr<Hidden_stick_state> sample_s_0(const Stick_length_info & length_endpoints, const Camera & c);

    Camera cam_;
    // I would have preferred to not have these as locals, but
    // I needed some temps to initialize stick_tree_, otherwise
    // it was complaining about not being initialized in the
    // initialization list.
    Stick_length_info initial_len_;

    Stick_node stick_tree_;
};

}}

#endif // CONTINUOUS_SAMPLE_HPP
