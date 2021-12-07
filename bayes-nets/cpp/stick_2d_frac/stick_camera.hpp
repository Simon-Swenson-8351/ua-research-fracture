#ifndef STICK_CAMERA_HPP
#define STICK_CAMERA_HPP

#include <memory>

#include <boost/serialization/access.hpp>

#include <m_cpp/m_matrix.h>
#include <prob_cpp/prob_pdf.h>

#include "prob.hpp"

namespace stick_2d_frac { namespace sample {

class Camera
{
    friend class boost::serialization::access;
public:
    static constexpr double CAMERA_LEFT = 0.0;
    static constexpr double CAMERA_BOTTOM = 0.0;

    // Creates a new camera, using forward sampling, to generate the
    // top of the camera in world coordinates, then the image aspect
    // ratio to generate the right side of the camera in world
    // coodinates.
    Camera(unsigned image_width, unsigned image_height, double frames_per_second);

    unsigned get_image_width() const { return im_w_; }
    unsigned get_image_height() const { return im_h_; }
    double get_camera_top() const { return c_t_; }
    double get_camera_right() const { return c_r_; }
    double get_camera_height() const { return c_t_ - CAMERA_BOTTOM; }
    double get_camera_width() const { return c_r_ - CAMERA_LEFT; }
    double get_meters_per_pixel() const {return meter_px_; }
    const kjb::Matrix & get_camera_matrix() const { return c_mat_; }
    double get_frames_per_second() const { return fps_; }

    void set_camera_top(double val);

    // No need to pass in homogeneous coordinates. That will be done
    // automatically if the input vector is of dimension 2. If the
    // dimension is 3, it is assumed to already be in homogeneous coordinates.
    // For any other dimensionality, an exception is thrown.
    std::unique_ptr<kjb::Vector> project(const kjb::Vector & world_coordinate) const;

    double log_prior() const { return prob::pdf(prob::C_T_DIST, c_t_); }

    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & im_w_ & im_h_ & c_t_ & c_r_ & meter_px_ & c_mat_ & fps_;
    }
private:
    unsigned im_w_;
    unsigned im_h_;
    double im_aspect_ratio_;
    double c_t_;
    double c_r_;
    double meter_px_;
    kjb::Matrix c_mat_;
    double fps_;
};

}}

#endif // STICK_CAMERA_HPP
