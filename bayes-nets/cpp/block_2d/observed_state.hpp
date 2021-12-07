#ifndef OBSERVED_STATE_HPP
#define OBSERVED_STATE_HPP

#include <boost/version.hpp>
#include <boost/serialization/access.hpp>
#if BOOST_VERSION >= 106400
#include <boost/serialization/boost_array.hpp>
#else
#include <boost/serialization/array.hpp>
#endif
#include <boost/serialization/split_member.hpp>

#include <m_cpp/m_matrix_d.h>
#include <prob_cpp/prob_sample.h>

#include "hidden_state.hpp"

namespace fracture { namespace block_2d {

class Observed_state
{
    friend class boost::serialization::access;
public:
    // Default constructor needed for boost serialization/deserialization
    Observed_state() {}

    Observed_state(
            const Hidden_state & hs,
            const kjb::Normal_distribution & image_noise_dist)
    {
        image_polygon_obs_ = hs.get_image_polygon();
        for(int i = 0; i < image_polygon_obs_.get_num_cols(); i++)
        {
            for(int j = 0; j < 2; j++)
            {
                // Ensure the noise is added in a consistent way by dividing by the homogeneous
                // component first.
                image_polygon_obs_(j, i) /= image_polygon_obs_(2, i);
                image_polygon_obs_(j, i) += kjb::sample(image_noise_dist);
            }
            image_polygon_obs_(2, i) = 1.0;
        }
    }

    const kjb::Matrix_d<3,4> & get_image_polygon() const { return image_polygon_obs_; }

    double log_prob(
            const kjb::Matrix_d<3,4> & image_polygon_actual,
            const kjb::Normal_distribution & image_noise_dist) const
    {
        double accum = 0.0;
        for(size_t i = 0; i < image_polygon_obs_.get_num_cols(); i++)
        {
            for(size_t j = 0; j < 2; j++)
            {
                accum += kjb::log_pdf(image_noise_dist, image_polygon_obs_(j, i) - (image_polygon_actual(j, i) / image_polygon_actual(2, i)));
            }
        }
        return accum;
    }

    bool operator==(const Observed_state &other) const
    {
        return image_polygon_obs_ == other.image_polygon_obs_;
    }

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        boost::array<double, 12> tmp;
        for(unsigned row = 0; row < 3; row++) for(unsigned col = 0; col < 4; col++) tmp[row * 4 + col] = image_polygon_obs_[row][col];
        ar << tmp;
    }
    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        boost::array<double, 12> tmp;
        ar >> tmp;
        for(unsigned row = 0; row < 3; row++) for(unsigned col = 0; col < 4; col++) image_polygon_obs_[row][col] = tmp[row * 4 + col];
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()
private:
    // sampled from gaussian noise surrounding each actual image polygon point.
    kjb::Matrix_d<3,4> image_polygon_obs_;
};

}}

#endif // OBSERVED_STATE_HPP
