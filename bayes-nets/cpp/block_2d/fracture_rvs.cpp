#include "fracture_rvs.hpp"

namespace fracture { namespace block_2d {

const prob::Truncated_normal_distribution Fracture_rvs::R_X_MOMENTUM_DIST(
        Fracture_rvs::R_X_MOMENTUM_MEAN,
        Fracture_rvs::R_X_MOMENTUM_STD,
        0,
        double(INFINITY));

const prob::Truncated_normal_distribution Fracture_rvs::L_ANGULAR_MOMENTUM_DIST(
        Fracture_rvs::L_ANGULAR_MOMENTUM_MEAN,
        Fracture_rvs::L_ANGULAR_MOMENTUM_STD,
        0,
        double(INFINITY));

bool Fracture_rvs::operator==(const Fracture_rvs &other) const
{
    return (frac_loc_dist_ == other.frac_loc_dist_)
            && (frac_loc_ == other.frac_loc_)
            && (r_x_momentum_ == other.r_x_momentum_)
            && (l_angular_momentum_ == other.l_angular_momentum_);
}

}}
