#ifndef STICK_2D_FRAC_PROB_HPP
#define STICK_2D_FRAC_PROB_HPP

#include <vector>
#include <cmath>

#include <boost/math/distributions.hpp>
#include <boost/math/policies/policy.hpp>
#include <boost/random/discrete_distribution.hpp>

#include <prob_cpp/prob_sample.h>

namespace stick_2d_frac { namespace prob {

class Truncated_normal_distribution {
public:
    Truncated_normal_distribution(double mean, double std, double low, double high) :
        base_dist_(mean, std),
        low_(low),
        high_(high),
        low_cdf_(kjb::cdf(base_dist_, low_)),
        high_cdf_(kjb::cdf(base_dist_, high_)),
        normalizer_(high_cdf_ - low_cdf_)
    {
        if(std < 0)
        {
            throw "std < 0";
        }
        if(low >= high)
        {
            throw "low >= high";
        }
    }
    double get_low() const { return low_; }
    double get_high() const { return high_; }
    double get_low_cdf() const { return low_cdf_; }
    double get_high_cdf() const { return high_cdf_; }
    double get_mean() const { return base_dist_.mean(); }
    double get_std() const { return base_dist_.standard_deviation(); }
    double get_normalizer() const { return normalizer_; }
    const kjb::Normal_distribution & get_base_dist() const { return base_dist_; }
private:
    kjb::Normal_distribution base_dist_;
    double low_;
    double high_;
    double low_cdf_;
    double high_cdf_;
    double normalizer_;
};
double pdf(const Truncated_normal_distribution & dist, double p);
double sample(const Truncated_normal_distribution & dist);

const double C_T_MEAN = 8.0;
const double C_T_STD = 2.0;
const double C_T_LOW = 0.0;
const double C_T_HIGH = double(INFINITY);
const Truncated_normal_distribution C_T_DIST = Truncated_normal_distribution(
        C_T_MEAN,
        C_T_STD,
        C_T_LOW,
        C_T_HIGH
);

inline double calc_len_0_mean(double c_width, double c_height)
{
    return std::sqrt(c_width * c_height) / 4;
}
inline double calc_len_0_std(double c_width, double c_height)
{
    return std::sqrt(c_width * c_height) / 16;
}
inline std::unique_ptr<Truncated_normal_distribution> new_len_0_dist(double c_width, double c_height)
{
    return std::unique_ptr<Truncated_normal_distribution>(
            new Truncated_normal_distribution(
                    calc_len_0_mean(c_width, c_height),
                    calc_len_0_std(c_width, c_height),
                    0.0,
                    double(INFINITY)));
}

inline double calc_frac_loc_i_mean(double stick_len) {
    return stick_len / 2;
}
inline double calc_frac_loc_i_std(double stick_len) {
    return stick_len / 4;
}
inline std::unique_ptr<Truncated_normal_distribution> new_frac_loc_i_dist(double stick_len) {
    return std::unique_ptr<Truncated_normal_distribution>(
            new Truncated_normal_distribution(
                    calc_frac_loc_i_mean(stick_len),
                    calc_frac_loc_i_std(stick_len),
                    0.0,
                    stick_len));
}

inline double calc_0_p_x_mean(double c_l, double c_r)
{
    return (c_r + c_l) / 2;
}
inline double calc_state_0_p_x_std(double c_l, double c_r)
{
    return (c_r - c_l) / 16;
}
inline std::unique_ptr<kjb::Normal_distribution> new_state_0_p_x_dist(double c_l, double c_r)
{
    return std::unique_ptr<kjb::Normal_distribution>(
            new kjb::Normal_distribution(
                    calc_0_p_x_mean(c_l, c_r),
                    calc_state_0_p_x_std(c_l, c_r)));
}

inline double calc_state_0_p_y_mean(double c_t, double c_b) {
    return (c_t + c_b) / 2;
}
inline double calc_state_0_p_y_std(double c_t, double c_b) {
    return (c_t - c_b) / 16;
}
inline std::unique_ptr<kjb::Normal_distribution> new_state_0_p_y_dist(double c_t, double c_b) {
    return std::unique_ptr<kjb::Normal_distribution>(
            new kjb::Normal_distribution(
                    calc_state_0_p_y_mean(c_t, c_b),
                    calc_state_0_p_y_std(c_t, c_b)));
}

const double STATE_0_V_X_IN_M_S_MEAN = 0.0;
const double STATE_0_V_X_IN_M_S_STD = 2.0;
const kjb::Normal_distribution STATE_0_V_X_IN_M_S_DIST(
        STATE_0_V_X_IN_M_S_MEAN,
        STATE_0_V_X_IN_M_S_STD
);

const double STATE_0_V_Y_IN_M_S_MEAN = 1.0;
const double STATE_0_V_Y_IN_M_S_STD = 2.0;
const kjb::Normal_distribution STATE_0_V_Y_IN_M_S_DIST(
        STATE_0_V_Y_IN_M_S_MEAN,
        STATE_0_V_Y_IN_M_S_STD
);

const double STATE_0_ANG_MEAN = 0.0;
const double STATE_0_ANG_STD = M_PI / 2;
const kjb::Normal_distribution STATE_0_ANG_DIST(
        STATE_0_ANG_MEAN,
        STATE_0_ANG_STD
);

const double STATE_0_ANG_VEL_IN_R_S_MEAN = 0.0;
const double STATE_0_ANG_VEL_IN_R_S_STD = 8 * M_PI;
const kjb::Normal_distribution STATE_0_ANG_VEL_IN_R_S_DIST(
        STATE_0_ANG_VEL_IN_R_S_MEAN,
        STATE_0_ANG_VEL_IN_R_S_STD
);

// For now, we are acting as if the noise has a diagonal covariance
// composed of one repeated scalar. That's why we only use a single distribution here.
// We'll sample it twice, one for each dimension in the image plane.
inline double calc_noise_offset_std(unsigned im_height, unsigned im_width) {
    return std::sqrt(double(im_width) * double(im_height)) / 64;
}
inline std::unique_ptr<kjb::Normal_distribution> new_noise_offset_dist(unsigned im_height, unsigned im_width) {
    return std::unique_ptr<kjb::Normal_distribution>(
            new kjb::Normal_distribution(
                    0.0,
                    calc_noise_offset_std(im_height, im_width)));
}

const double FR_T_OFFSET_P = 0.5;
// This is an offset distribution.
const boost::math::geometric_distribution<> FR_T_OFFSET_DIST(FR_T_OFFSET_P);
// Because of the offset distribution, need to add the previous fracture time.
inline unsigned fr_t_sample(unsigned prev_fr_t) {
    return prev_fr_t + 1 + unsigned(kjb::sample(FR_T_OFFSET_DIST));
}

// TODO find out where to put these to avoid a circular include
//    double log_prior(const sample::Sample & s);
//    double log_likelihood(const sample::Sample & s);
//    double log_posterior(const sample::Sample & s);
//    double gradient(const sample::Sample & s, const std::vector<double> & deltas);

}

}

#endif
