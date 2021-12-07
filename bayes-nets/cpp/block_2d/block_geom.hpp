#ifndef BLOCK_GEOM_HPP
#define BLOCK_GEOM_HPP

#include <sstream>

#include <boost/serialization/access.hpp>

#include <m_cpp/m_matrix.h>

#include "initial_block_rvs.hpp"

namespace fracture {
namespace block_2d {

// Geometry of the block in a coordinate space local to the object's center of 
// mass.
class Block_geom
{
    friend class boost::serialization::access;
public:
    enum Fragment_side
    {
        FS_LEFT,
        FS_RIGHT
    };

    // required for serialization/deserialization
    Block_geom() {}

    // Creates an oriented box with the given width and height.
    Block_geom(double w, double h) :
            w_(w),
            h_(h)
    {
        recalculate_local_endpoints();
    }

    const kjb::Matrix_d<3,4> & get_local_endpoints() const { return local_endpoints_; }
    double get_width() const { return w_; }
    double get_height() const { return h_; }
    double get_volume() const { return w_ * h_; }

    void recalculate_local_endpoints();

    double momentum_to_velocity(double momentum) const { return momentum / get_volume(); }
    // Given a parent's geometry, we assume we are a child of that geometry, then 
    // we figure out the presumed x offset (in world coordinates) of our center 
    // of mass from the original block's center of mass.
    double fracture_x_offset(const Block_geom & parent_geom, Fragment_side fs) const;

    bool operator==(const Block_geom &other) const;

    void recalculate_parent_values(const Initial_block_rvs &ibr);
    void recalculate_child_values(const Initial_block_rvs &ibr, double frac_loc, Block_geom::Fragment_side fs);
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){}
private:
    // Calculated from Initial_block_rvs
    kjb::Matrix_d<3,4> local_endpoints_;
    // Calculated from Initial_block_rvs
    double w_;
    // Calculated from Initial_block_rvs
    double h_;
};

}}

#endif // BLOCK_GEOM_HPP
