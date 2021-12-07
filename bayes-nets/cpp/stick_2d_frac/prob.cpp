#include <boost/math/distributions.hpp>
#include <boost/math/policies/policy.hpp>

#include "prob.hpp"

namespace stick_2d_frac { namespace prob {

double pdf(const Truncated_normal_distribution & dist, double p) {
    if (p >= dist.get_low() && p < dist.get_high()) {
        return pdf(dist.get_base_dist(), p) / dist.get_normalizer();
    } else {
        return 0;
    }
}

double sample(const Truncated_normal_distribution &dist) {
    static const kjb::Uniform_distribution truncated_normal_sampling_dist;
    double s;
    do
    {
        s = kjb::sample(dist.get_base_dist());
    } while (s < dist.get_low() || s > dist.get_high());
    return s;
}

}}
