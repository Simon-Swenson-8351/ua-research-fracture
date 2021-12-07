#include "initial_block_rvs.hpp"
#include "util.hpp"

namespace fracture { namespace block_2d {

const prob::Truncated_normal_distribution Initial_block_rvs::INIT_WIDTH_DIST(
        Initial_block_rvs::INIT_WIDTH_MEAN,
        Initial_block_rvs::INIT_WIDTH_STD,
        0.0,
        double(INFINITY));

const prob::Truncated_normal_distribution Initial_block_rvs::INIT_HEIGHT_DIST(
        Initial_block_rvs::INIT_HEIGHT_MEAN,
        Initial_block_rvs::INIT_HEIGHT_STD,
        0.0,
        double(INFINITY));

bool Initial_block_rvs::operator==(const Initial_block_rvs &other) const
{
    return (init_x_dist_ == other.init_x_dist_)
            && (init_x_ == other.init_x_)
            && (init_y_dist_ == other.init_y_dist_)
            && (init_y_ == other.init_y_)
            && init_w_ == other.init_w_
            && init_h_ == other.init_h_;
}

}}
