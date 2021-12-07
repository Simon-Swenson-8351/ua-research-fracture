#ifndef STICK_LOCAL_ENDPOINTS_HPP
#define STICK_LOCAL_ENDPOINTS_HPP

#include <memory>

#include <boost/serialization/access.hpp>

#include <m_cpp/m_matrix.h>

#include "util.hpp"

namespace stick_2d_frac { namespace sample {

// This class unifies two concepts: stick length, and the corresponding local
// endpoints. It allows any client that wants to use one version or another
// to do so seemlessly. Some tasks (like sampling fracture location) are easier
// with the length. Some tasks, like finding the world coordinates or projecting
// the points onto an image, are a better with the endpoint matrix.
class Stick_length_info
{
    friend class boost::serialization::access;
public:
    Stick_length_info(double len) :
        len_(len),
        local_endpoints_homo_(3, 2)
    {
        local_endpoints_homo_(0, 0) = -len / 2.0; local_endpoints_homo_(0, 1) = len / 2.0;
        local_endpoints_homo_(1, 0) =        0.0; local_endpoints_homo_(1, 1) =       0.0;
        local_endpoints_homo_(2, 0) =        1.0; local_endpoints_homo_(2, 1) =       1.0;
    }
    Stick_length_info(const kjb::Matrix & local_endpoints, bool endpoints_homogeneous = true):
        len_(local_endpoints(0, 1)),
        local_endpoints_homo_(local_endpoints)
    {
        if(endpoints_homogeneous)
        {
            len_ /= local_endpoints(2, 1);
        }
        else
        {
            local_endpoints_homo_.resize(local_endpoints.get_num_rows() + 1, local_endpoints.get_num_cols(), 1.0);
        }
    }

    double get_length() const { return len_; }
    const kjb::Matrix & get_local_endpoints_homo() const { return local_endpoints_homo_; }
    std::unique_ptr<kjb::Matrix> get_local_endpoints_non_homo() const { return util::homo_col_vecs_to_non_homo_col_vecs(local_endpoints_homo_); }

    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & len_ & local_endpoints_homo_;
    }
private:
    double len_;
    kjb::Matrix local_endpoints_homo_;
};

}}

#endif // STICK_LOCAL_ENDPOINTS_HPP
