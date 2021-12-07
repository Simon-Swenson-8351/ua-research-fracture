#include <iostream>

#include <boost/version.hpp>
#include <prob_cpp/prob_sample.h>

#include "camera.hpp"
#include "prob.hpp"
#include "util.hpp"

namespace fracture { namespace block_2d {

const prob::Truncated_normal_distribution Camera::C_T_DIST(
        C_T_MEAN,
        C_T_STD,
        C_T_LOW,
        C_T_HIGH
);

Camera::Camera(){}

Camera::Camera(unsigned image_width, unsigned image_height, double frames_per_second) :
        im_w_(image_width),
        im_h_(image_height),
        c_t_(prob::sample(C_T_DIST)),
        fps_(frames_per_second)
{}

unsigned Camera::get_image_width() const { return im_w_; }
unsigned Camera::get_image_height() const { return im_h_; }
double Camera::get_image_aspect_ratio() const
{
    return double(im_w_) / double(im_h_);
}
double Camera::get_camera_top() const{ return c_t_; }
double Camera::get_camera_bottom() const { return CAMERA_BOTTOM; }
double Camera::get_camera_left() const { return CAMERA_LEFT; }
double Camera::get_camera_right() const
{
    return c_t_ * get_image_aspect_ratio();
}
double Camera::get_camera_height() const { return c_t_ - CAMERA_BOTTOM; }
double Camera::get_camera_width() const { return get_camera_right() - CAMERA_LEFT; }
double Camera::get_meters_per_pixel() const
{
    return (c_t_ - CAMERA_BOTTOM) / im_h_;
}
const kjb::Matrix_d<3,3> Camera::get_camera_matrix() const
{
    kjb::Matrix_d<3,3> r;
    r.fill({0.0,0.0,0.0});
                                           r(0, 1) = -1 / get_meters_per_pixel(); r(0, 2) = c_t_ / get_meters_per_pixel();
    r(1, 0) =  1 / get_meters_per_pixel();                                        r(1, 2) = -CAMERA_LEFT / get_meters_per_pixel();
                                                                                  r(2, 2) = 1.0;
    return r;
}
double Camera::get_frames_per_second() const { return fps_; }
double Camera::get_image_noise_std() const
{
    return std::sqrt(double(im_h_) * double(im_w_)) / 512.0;
}
const kjb::Normal_distribution Camera::get_image_noise_dist() const
{
    return kjb::Normal_distribution(0.0, get_image_noise_std());
}

void Camera::set_camera_top(double val) {
    if(prob::pdf(C_T_DIST, val) == 0.0) throw prob::No_support_exception();
    c_t_ = val;
}

kjb::Vector_d<3> Camera::project(const kjb::Vector_d<3> & world_coordinate) const {
    return get_camera_matrix() * world_coordinate;
}

bool Camera::operator==(const Camera &other) const
{
    return (im_w_ == other.im_w_)
            && (im_h_ == other.im_h_)
            && (c_t_ == other.c_t_)
            && (fps_ == other.fps_);
}

}}
