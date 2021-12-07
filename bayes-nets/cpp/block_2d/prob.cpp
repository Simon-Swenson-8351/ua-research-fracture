#include <cmath>

#include <boost/math/distributions.hpp>
#include <boost/math/policies/policy.hpp>

#include "prob.hpp"
#include "util.hpp"

namespace fracture { namespace prob {

Truncated_normal_distribution::Truncated_normal_distribution(double mean, double std, double low, double high) :
    base_dist_(mean, std),
    low_(low),
    high_(high),
    low_cdf_(kjb::cdf(base_dist_, low_)),
    high_cdf_(kjb::cdf(base_dist_, high_)),
    normalizer_(high_cdf_ - low_cdf_)
{
    if(std < 0)  throw Invalid_parameters_exception(); //util::err_str(__FILE__, __LINE__);
    if(low >= high)
    {
        low_ = high;
        high_ = low;
    }
}

bool Truncated_normal_distribution::operator==(const Truncated_normal_distribution &other) const
{
    return (base_dist_ == other.base_dist_)
            && (low_ == other.low_)
            && (high_ == other.high_);
}

double pdf(const Truncated_normal_distribution & dist, double p) {
    if (p >= dist.get_low() && p < dist.get_high()) {
        return boost::math::pdf(dist.get_base_dist(), p) / dist.get_normalizer();
    } else {
        return 0.0;
    }
}

double log_pdf(const Truncated_normal_distribution & dist, double p)
{
    if (p >= dist.get_low() && p < dist.get_high()) {
        return std::log(boost::math::pdf(dist.get_base_dist(), p) / dist.get_normalizer());
    } else {
        return double(-INFINITY);
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

//const char* No_support_exception::what() const noexcept
//{
//    return s_.data();
//}

}}
