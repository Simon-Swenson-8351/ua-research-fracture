#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <memory>
#include <vector>

#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>

#include <m_cpp/m_vector_d.h>
#include <m_cpp/m_matrix_d.h>
#include <prob_cpp/prob_pdf.h>

#include "prob.hpp"
#include "util.hpp"

namespace fracture { namespace block_2d {

class Camera
{
    friend class boost::serialization::access;
public:
    static constexpr double CAMERA_LEFT = 0.0;
    static constexpr double CAMERA_BOTTOM = 0.0;

    static constexpr double C_T_MEAN = 3.0;
    static constexpr double C_T_STD = 1.0;
    static constexpr double C_T_LOW = 0.0;
    static constexpr double C_T_HIGH = double(INFINITY);
    static const prob::Truncated_normal_distribution C_T_DIST;

    // required for serialization/deserialization
    Camera();

    // Creates a new camera, using forward sampling, to generate the
    // top of the camera in world coordinates, then the image aspect
    // ratio to generate the right side of the camera in world
    // coodinates.
    Camera(unsigned image_width, unsigned image_height, double frames_per_second);

    unsigned get_image_width() const;
    unsigned get_image_height() const;
    double get_image_aspect_ratio() const;
    double get_camera_top() const;
    double get_camera_bottom() const;
    double get_camera_left() const;
    double get_camera_right() const;
    double get_camera_height() const;
    double get_camera_width() const;
    double get_meters_per_pixel() const;
    const kjb::Matrix_d<3,3> get_camera_matrix() const;
    double get_frames_per_second() const;
    double get_image_noise_std() const;
    const kjb::Normal_distribution get_image_noise_dist() const;

    void set_camera_top(double val);

    // No need to pass in homogeneous coordinates. That will be done
    // automatically if the input vector is of dimension 2. If the
    // dimension is 3, it is assumed to already be in homogeneous coordinates.
    // For any other dimensionality, an exception is thrown.
    kjb::Vector_d<3> project(const kjb::Vector_d<3> & world_coordinate) const;

    double log_prob() const { return prob::log_pdf(C_T_DIST, c_t_); }

    bool operator==(const Camera &other) const;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & im_w_ & im_h_ & c_t_ & fps_;
    }

private:
    unsigned im_w_;
    unsigned im_h_;
    double c_t_;
    double fps_;
};

}}

#endif // CAMERA_HPP
