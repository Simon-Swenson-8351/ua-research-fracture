#ifndef INITIAL_BLOCK_RVS_HPP
#define INITIAL_BLOCK_RVS_HPP

#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>

#include <m_cpp/m_matrix.h>
#include <prob_cpp/prob_sample.h>

#include "prob.hpp"
#include "camera.hpp"
#include "util.hpp"

namespace fracture { namespace block_2d {

class Initial_block_rvs
{
    friend class boost::serialization::access;
public:
    static double calc_init_x_mean(double c_l, double c_r) { return (c_l + c_r) / 2.0; }
    static double calc_init_x_std(double c_l, double c_r)
    {
        return (c_r - c_l) / 16.0;
    }
    static kjb::Normal_distribution calc_init_x_dist(double c_l, double c_r)
    {
        return kjb::Normal_distribution(
                calc_init_x_mean(c_l, c_r),
                calc_init_x_std(c_l, c_r));
    }

    static double calc_init_y_mean(double c_t, double c_b) { return (c_t + c_b) / 2.0; }
    static double calc_init_y_std(double c_t, double c_b)
    {
        return (c_t - c_b) / 16.0;
    }
    static kjb::Normal_distribution calc_init_y_dist(double c_t, double c_b)
    {
        return kjb::Normal_distribution(
                calc_init_y_mean(c_t, c_b),
                calc_init_y_std(c_t, c_b));
    }

    // I'd guess that a log's about an 8 inch diameter.
    static constexpr double INIT_WIDTH_MEAN = 0.2;
    // IDK guessing here.
    static constexpr double INIT_WIDTH_STD = 0.075;
    static const prob::Truncated_normal_distribution INIT_WIDTH_DIST;

    static constexpr double INIT_HEIGHT_MEAN = 0.3;
    static constexpr double INIT_HEIGHT_STD = 0.2;
    static const prob::Truncated_normal_distribution INIT_HEIGHT_DIST;

    // required for serialization/deserialization
    Initial_block_rvs() {}

    Initial_block_rvs(const Camera & c)
    {
        update_distributions(c);
        init_x_ = kjb::sample(init_x_dist_);
        init_y_ = kjb::sample(init_y_dist_);
        init_w_ = prob::sample(INIT_WIDTH_DIST);
        init_h_ = prob::sample(INIT_HEIGHT_DIST);
    }

    const kjb::Normal_distribution & get_initial_x_distribution() const { return init_x_dist_; }
    double get_initial_x() const { return init_x_; }
    const kjb::Normal_distribution & get_initial_y_distribution() const { return init_y_dist_; }
    double get_initial_y() const { return init_y_; }
    double get_initial_width() const { return init_w_; }
    double get_initial_height() const { return init_h_; }

    void update_distributions(const Camera & c)
    {
        init_x_dist_ = calc_init_x_dist(c.CAMERA_LEFT, c.get_camera_right());
        init_y_dist_ = calc_init_y_dist(c.get_camera_top(), c.CAMERA_BOTTOM);
    }

    void set_initial_x(double val)
    {
        init_x_ = val;
    }

    void set_initial_y(double val)
    {
        init_y_ = val;
    }

    void set_initial_width(double val)
    {
        if(prob::pdf(INIT_WIDTH_DIST, val) == 0.0) throw prob::No_support_exception(); //util::err_str(__FILE__, __LINE__);
        init_w_ = val;
    }

    void set_initial_height(double val)
    {
        if(prob::pdf(INIT_HEIGHT_DIST, val) == 0.0) throw prob::No_support_exception(); //util::err_str(__FILE__, __LINE__);
        init_h_ = val;
    }

    double log_prob() const
    {
        return kjb::log_pdf(init_x_dist_, init_x_)
                + kjb::log_pdf(init_y_dist_, init_y_)
                // 20191217 experiment to see if we get local optima with width/height clamped
                /*+ prob::log_pdf(INIT_WIDTH_DIST, init_w_)
                + prob::log_pdf(INIT_HEIGHT_DIST, init_h_)*/;
    }

    bool operator==(const Initial_block_rvs &other) const;

    void recalculate_values(const Camera & c) { update_distributions(c); }
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar << init_x_ << init_y_ << init_w_ << init_h_;
    }
    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar >> init_x_ >> init_y_ >> init_w_ >> init_h_;
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
    // Calculated from c_l and c_r
    kjb::Normal_distribution init_x_dist_;
    // Sampled
    double init_x_;
    // Calculated from c_t and c_b
    kjb::Normal_distribution init_y_dist_;
    // Sampled
    double init_y_;
    // Sampled
    double init_w_;
    // Sampled
    double init_h_;
};

}}

#endif // INITIAL_BLOCK_RVS_HPP
