#ifndef FRACTURE_RVS_HPP
#define FRACTURE_RVS_HPP

#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>

#include "prob.hpp"
#include "block_geom.hpp"

namespace fracture { namespace block_2d {

class Fracture_rvs {
    friend class boost::serialization::access;
public:
    static double calc_frac_loc_mean(double block_w) { return block_w/2; }
    static double calc_frac_loc_std(double block_w)
    {
        return block_w / 4;
    }
    static prob::Truncated_normal_distribution calc_frac_loc_dist(double block_w)
    {
        return prob::Truncated_normal_distribution(
                calc_frac_loc_mean(block_w),
                calc_frac_loc_std(block_w),
                0.0,
                block_w
        );
    }

    static constexpr double R_X_MOMENTUM_MEAN = 0.0;
    static constexpr double R_X_MOMENTUM_STD = 0.05;
    static const prob::Truncated_normal_distribution R_X_MOMENTUM_DIST;

    static constexpr double L_ANGULAR_MOMENTUM_MEAN = 0.0;
    static constexpr double L_ANGULAR_MOMENTUM_STD = 0.2;
    static const prob::Truncated_normal_distribution L_ANGULAR_MOMENTUM_DIST;

    // required for serialization/deserialization
    Fracture_rvs() {}

    Fracture_rvs(const Block_geom & geom) :
            frac_loc_dist_(calc_frac_loc_dist(geom.get_width())),
            frac_loc_(prob::sample(frac_loc_dist_)),
            r_x_momentum_(prob::sample(R_X_MOMENTUM_DIST)),
            l_angular_momentum_(prob::sample(L_ANGULAR_MOMENTUM_DIST))
    {}

    void init_fracture_location(const Block_geom & g)
    {
        frac_loc_dist_ = calc_frac_loc_dist(g.get_width());
    }

    void recalculate_values(const Block_geom & g)
    {
        init_fracture_location(g);
    }

    double get_fracture_location() const { return frac_loc_; }
    const prob::Truncated_normal_distribution & get_fracture_location_distribution() const { return frac_loc_dist_; }
    double get_left_x_momentum() const { return -r_x_momentum_; }
    double get_right_x_momentum() const { return r_x_momentum_; }
    double get_left_angular_momentum() const { return l_angular_momentum_; }
    double get_right_angular_momentum() const { return -l_angular_momentum_; }

    void set_fracture_location(double val)
    {
        if(prob::pdf(frac_loc_dist_, val) == 0.0) throw prob::No_support_exception(); //util::err_str(__FILE__, __LINE__);
        frac_loc_ = val;
    }

    void set_right_x_momentum(double val)
    {
        if(prob::pdf(R_X_MOMENTUM_DIST, val) == 0.0) throw prob::No_support_exception(); //util::err_str(__FILE__, __LINE__);
        r_x_momentum_ = val;
    }

    void set_left_angular_momentum(double val)
    {
        if(prob::pdf(L_ANGULAR_MOMENTUM_DIST, val) == 0.0) throw prob::No_support_exception(); //util::err_str(__FILE__, __LINE__);
        l_angular_momentum_ = val;
    }

    double log_prob() const
    {
        return prob::pdf(R_X_MOMENTUM_DIST, r_x_momentum_)
                + prob::pdf(L_ANGULAR_MOMENTUM_DIST, l_angular_momentum_)
                + prob::pdf(frac_loc_dist_, frac_loc_);
    }

    bool operator==(const Fracture_rvs &other) const;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & frac_loc_ & r_x_momentum_ & l_angular_momentum_;
    }

private:
    // Fracture location will be somewhere between 0 and initial width.
    // Calculated from initial block width
    prob::Truncated_normal_distribution frac_loc_dist_;
    // Sampled from the distr. above
    double frac_loc_;

    // Must be >= 0
    // Sampled
    double r_x_momentum_;

    // Must be >= 0
    // Sampled
    double l_angular_momentum_;
};

}}

#endif // FRACTURE_RVS_HPP
