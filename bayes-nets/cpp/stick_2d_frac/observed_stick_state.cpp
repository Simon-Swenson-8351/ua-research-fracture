#include <prob_cpp/prob_sample.h>

#include "observed_stick_state.hpp"

namespace stick_2d_frac { namespace sample {

Observed_stick_state::Observed_stick_state(
        const kjb::Matrix & hidden_image_endpoints_homo,
        const kjb::Normal_distribution & offset_dist) :
    image_endpoints_homo_(hidden_image_endpoints_homo)
{
    image_endpoints_homo_(0, 0) += (kjb::sample(offset_dist) * image_endpoints_homo_(2, 0)); image_endpoints_homo_(0, 1) += (kjb::sample(offset_dist) * image_endpoints_homo_(2, 1));
    image_endpoints_homo_(1, 0) += (kjb::sample(offset_dist) * image_endpoints_homo_(2, 0)); image_endpoints_homo_(1, 1) += (kjb::sample(offset_dist) * image_endpoints_homo_(2, 1));
}

double Observed_stick_state::log_likelihood(const Hidden_stick_state & hss, const kjb::Normal_distribution & offset_dist) const
{
    // The original modification was observed_value = actual_value + (sample * homogeneous_scalar)
    // So to find the PDF of the sample, we need (observed_value - actual_value) / homogeneous_scalar
    return kjb::pdf(offset_dist, (image_endpoints_homo_(0, 0) - hss.get_image_endpoints_homo()(0, 0)) / image_endpoints_homo_(2, 0))
            + kjb::pdf(offset_dist, (image_endpoints_homo_(1, 0) - hss.get_image_endpoints_homo()(1, 0)) / image_endpoints_homo_(2, 0))
            + kjb::pdf(offset_dist, (image_endpoints_homo_(0, 1) - hss.get_image_endpoints_homo()(0, 1)) / image_endpoints_homo_(2, 1))
            + kjb::pdf(offset_dist, (image_endpoints_homo_(1, 1) - hss.get_image_endpoints_homo()(1, 1)) / image_endpoints_homo_(2, 1));
}

}}
