#ifndef SAMPLE_VECTOR_ADAPTER_HPP
#define SAMPLE_VECTOR_ADAPTER_HPP

#include <diff_cpp/diff_gradient.h>

#include "sample.hpp"

namespace fracture { namespace block_2d {

enum Rvs_idx
{
    RI_C_T,
    RI_INIT_X,
    RI_INIT_Y,
    RI_INIT_W,
    RI_INIT_H,
    RI_FRAC_LOC,
    RI_R_X_MOM,
    RI_L_ANG_MOM,

    // INSERT NEW ENTRIES ABOVE THIS LINE
    RI_COUNT
};

const std::string Rvs_idx_str[RI_COUNT] = {
    "Camera Top",
    "Initial X",
    "Initial Y",
    "Initial Width",
    "Initial Height",
    "Fracture Location",
    "X Momentum",
    "Angular Momentum"
};

/**
 * @brief library ergo requires data to be in the form of a vector. This
 * class simply wraps the ContinuousSample data and makes it appear as a
 * vector to ergo.
 */
class Sample_vector_adapter {
public:
    Sample_vector_adapter() {}

    double get(const Sample * s, size_t idx) const;
    void set(Sample * s, size_t idx, double val) const;
    // c_t
    // init_x
    // init_y
    // init_w
    // init_h
    // frac_loc
    // r_x_momentum
    // l_ang_momentum
    size_t size(const Sample * s) const { return RI_COUNT; }
};

double sample_log_prob(const Sample &s);
kjb::Vector sample_log_gradient(const Sample &s);

}}

#endif // SAMPLE_VECTOR_ADAPTER_HPP
