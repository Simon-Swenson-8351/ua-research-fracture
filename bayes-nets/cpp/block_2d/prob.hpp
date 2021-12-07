#ifndef PROB_HPP
#define PROB_HPP

#include <vector>
#include <cmath>

#include <boost/serialization/split_member.hpp>
#include <boost/serialization/access.hpp>
#include <boost/math/distributions.hpp>
#include <boost/random/discrete_distribution.hpp>

#include <prob_cpp/prob_sample.h>


namespace fracture { namespace prob {

class Truncated_normal_distribution
{
    friend class boost::serialization::access;
public:
    Truncated_normal_distribution() :
        base_dist_(0.0, 1.0),
        low_(double(-INFINITY)),
        high_(double(INFINITY)),
        low_cdf_(kjb::cdf(base_dist_, low_)),
        high_cdf_(kjb::cdf(base_dist_, high_)),
        normalizer_(high_cdf_ - low_cdf_)
    {
    }

    Truncated_normal_distribution(double mean, double std, double low, double high);
    double get_low() const { return low_; }
    double get_high() const { return high_; }
    double get_low_cdf() const { return low_cdf_; }
    double get_high_cdf() const { return high_cdf_; }
    double get_mean() const { return base_dist_.mean(); }
    double get_std() const { return base_dist_.standard_deviation(); }
    double get_normalizer() const { return normalizer_; }
    const kjb::Normal_distribution & get_base_dist() const { return base_dist_; }

    bool operator==(const Truncated_normal_distribution &other) const;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        double bd_mean = base_dist_.mean();
        double bd_std = base_dist_.standard_deviation();
        ar << bd_mean << bd_std << low_ << high_ << low_cdf_ << high_cdf_ << normalizer_;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        double bd_mean;
        double bd_std;
        ar >> bd_mean >> bd_std >> low_ >> high_ >> low_cdf_ >> high_cdf_ >> normalizer_;
        base_dist_ = kjb::Normal_distribution(bd_mean, bd_std);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
    kjb::Normal_distribution base_dist_;
    double low_;
    double high_;
    double low_cdf_;
    double high_cdf_;
    double normalizer_;
};
double pdf(const Truncated_normal_distribution & dist, double p);
double log_pdf(const Truncated_normal_distribution & dist, double p);
double sample(const Truncated_normal_distribution & dist);

class No_support_exception : std::exception
{
//public:
//    No_support_exception(const std::string &s) :
//        std::exception(),
//        s_(s)
//    {}

//    virtual const char* what() const noexcept;
//private:
//    std::string s_;
};

class Invalid_parameters_exception : std::exception {};

}}

#endif // PROB_HPP
