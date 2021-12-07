#include <iostream>

#include <prob_cpp/prob_sample.h>

#include "stick_camera.hpp"
#include "prob.hpp"

namespace stick_2d_frac { namespace sample {

Camera::Camera(unsigned image_width, unsigned image_height, double frames_per_second) :
    im_w_(image_width),
    im_h_(image_height),
    im_aspect_ratio_(double(image_width) / double(image_height)),
    c_mat_(kjb::Matrix(3,3)),
    fps_(frames_per_second)
{
    // (1, 0), (0, 1), (0, 2), (1, 2) depend on c_t, so that's updated in the setter for c_t.
    c_mat_(0, 0) = 0;
                      c_mat_(1, 1) = 0;
    c_mat_(2, 0) = 0; c_mat_(2, 1) = 0; c_mat_(2, 2) = 1;
    set_camera_top(prob::sample(prob::C_T_DIST));
}

void Camera::set_camera_top(double val) {
    c_t_ = val;
    c_r_ = c_t_ * im_aspect_ratio_;
    meter_px_ = (c_t_ - CAMERA_BOTTOM) / im_h_;
    // (1, 0), (0, 1), (0, 2), (1, 2) depend on c_t
                                  c_mat_(0, 1) = -1 / meter_px_; c_mat_(0, 2) = c_t_ / meter_px_;
    c_mat_(1, 0) = 1 / meter_px_;                                c_mat_(1, 2) = -CAMERA_LEFT / meter_px_;
}

std::unique_ptr<kjb::Vector> Camera::project(const kjb::Vector & world_coordinate) const {
    std::unique_ptr<kjb::Vector> result = std::unique_ptr<kjb::Vector>(
            new kjb::Vector(world_coordinate));
    switch(world_coordinate.size()) {
    case 2:
        result->resize(3, 1);
        break;
    case 3:
        break;
    default:
        std::stringstream ex_stream;
        ex_stream << "wrong dimensionality for world_coordinate: " << world_coordinate.size();
        throw ex_stream.str();
    }
    *result = c_mat_ * *result;
    switch(world_coordinate.size()) {
    case 2:
        *result /= (*result)[result->size() - 1];
        result->resize(result->size() - 1);
        break;
    default:
        throw "wrong dimensionality";
    }
    return std::move(result);
}

}}
