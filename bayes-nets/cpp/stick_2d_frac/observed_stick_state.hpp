#ifndef OBSERVED_STICK_STATE_HPP
#define OBSERVED_STICK_STATE_HPP

#include <memory>

#include <boost/serialization/access.hpp>

#include <m_cpp/m_matrix.h>
#include <prob_cpp/prob_distribution.h>

#include "hidden_stick_state.hpp"
#include "stick_camera.hpp"

namespace stick_2d_frac { namespace sample {

class Observed_stick_state {
    friend class boost::serialization::access;
public:
    enum Endpoint_index {
        EI_FIRST = 0,
        EI_SECOND = 1
    };

    // Forward sampling from a given hidden state to produce the observed
    // endpoints.
    // camera_height and camera_width are in world coordinates, used to
    // determine the value for the Gaussian noise added to each end point.
    Observed_stick_state(
            const kjb::Matrix & hidden_image_endpoints_homo,
            const kjb::Normal_distribution & offset_dist);
            // Can't make offset_dist a constant, since it depends on image_width and image_height.
            // Thus, we just construct one distribution and share it across all observed stick states.

    const kjb::Matrix & get_image_endpoints_homo() const { return image_endpoints_homo_; }

    double log_likelihood(
            const Hidden_stick_state & hss,
            const kjb::Normal_distribution & offset_dist) const;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & image_endpoints_homo_;
    }
private:
    // Represented in homogeneous coordinates as a 3x2 matrix in world
    // coordinates.
    kjb::Matrix image_endpoints_homo_;
};

}}

#endif // OBSERVED_STICK_STATE_HPP
