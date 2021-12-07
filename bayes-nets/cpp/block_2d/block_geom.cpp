#include "block_geom.hpp"
#include "util.hpp"

namespace fracture {
namespace block_2d {

double Block_geom::fracture_x_offset(const Block_geom & parent_geom, Fragment_side fs) const
{
    switch(fs)
    {
    case FS_LEFT:
        return (-parent_geom.get_width() + w_) / 2;
    case FS_RIGHT:
        return (parent_geom.get_width() - w_) / 2;
    default:
        throw util::Unhandled_enum_value_exception();
    }
}

bool Block_geom::operator==(const Block_geom &other) const
{
    return (local_endpoints_ == other.local_endpoints_)
            && (w_ == other.w_)
            && (h_ == other.h_);
}

void Block_geom::recalculate_local_endpoints()
{
    for(int idx = 0; idx < 4; idx++)
    {
        double x = -w_/2;
        double y = -h_/2;
        // y coord
        switch(idx)
        {
        case 0:
        case 1:
            y += h_;
        }
        // x coord
        switch(idx)
        {
        case 1:
        case 2:
            x += w_;
        }
        local_endpoints_(0, idx) = x;
        local_endpoints_(1, idx) = y;
        local_endpoints_(2, idx) = 1.0;
    }
}

void Block_geom::recalculate_parent_values(const Initial_block_rvs &ibr)
{
    w_ = ibr.get_initial_width();
    h_ = ibr.get_initial_height();
    recalculate_local_endpoints();
}

void Block_geom::recalculate_child_values(const Initial_block_rvs &ibr, double frac_loc, Block_geom::Fragment_side fs)
{
    h_ = ibr.get_initial_height();
    switch(fs)
    {
    case Block_geom::FS_LEFT:
        w_ = frac_loc;
        break;
    case Block_geom::FS_RIGHT:
        w_ = ibr.get_initial_width() - frac_loc;
        break;
    default:
        throw util::Unhandled_enum_value_exception();
    }
    recalculate_local_endpoints();
}

}
}
