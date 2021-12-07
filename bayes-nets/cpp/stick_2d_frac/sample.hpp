#ifndef SAMPLE_HPP
#define SAMPLE_HPP

#include <vector>
#include <memory>

#include <boost/serialization/access.hpp>

#include <prob_cpp/prob_sample.h>

#include "discrete_sample.hpp"
#include "continuous_sample.hpp"

namespace stick_2d_frac { namespace sample {

class Sample {
    friend class boost::serialization::access;
public:
    // Constructs a new Sample using the forward sampling strategy.
    Sample(unsigned num_ims, unsigned im_w, unsigned im_h, double cam_fps) :
        ds_(num_ims),
        cs_(ds_, im_w, im_h, cam_fps) { }

    // Returns a reference to the DiscreteSample object owned by this sample.
    const Discrete_sample & get_discrete_sample() const { return ds_; }

    // Returns a reference to the ContinuousSample object owned by this sample.
    const Continuous_sample & get_continuous_sample() const { return cs_; }

    double log_prior() const { return get_discrete_sample().log_prior() + get_continuous_sample().log_prior(); }
    double log_likelihood() const { return get_continuous_sample().log_likelihood(); }
    double log_posterior() const { return log_prior() + log_likelihood(); }

    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & ds_ & cs_;
    }
private:
    Discrete_sample ds_;
    Continuous_sample cs_;
};
double log_posterior(const Sample & s) { return s.log_posterior(); }

}}

#endif // SAMPLE_HPP
