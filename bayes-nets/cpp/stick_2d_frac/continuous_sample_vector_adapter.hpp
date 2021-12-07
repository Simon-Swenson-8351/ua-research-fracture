#ifndef CONTINUOUS_SAMPLE_VECTOR_ADAPTER_HPP
#define CONTINUOUS_SAMPLE_VECTOR_ADAPTER_HPP

#include <diff_cpp/diff_gradient.h>

#include "continuous_sample.hpp"

namespace stick_2d_frac {

/**
 * @brief library ergo requires data to be in the form of a vector. This
 * class simply wraps the ContinuousSample data and makes it appear as a
 * vector to ergo.
 */
class Continuous_sample_vector_adapter {

public:
    Continuous_sample_vector_adapter();

    double get(const sample::Continuous_sample * s, size_t idx) const;
    void set(sample::Continuous_sample * s, size_t idx, double val) const;
    size_t size(const sample::Continuous_sample * s) const;
};

double continuous_sample_log_posterior(const sample::Continuous_sample & cs)
{
    return cs.log_posterior();
}

kjb::Vector continuous_sample_log_gradient(const sample::Continuous_sample & cs)
{
    Continuous_sample_vector_adapter csva;
    // TODO figure out how to define the deltas
    std::vector<double> deltas(csva.size(&cs), 0.0000001);
    return kjb::gradient_ffd<
            double(*)(const sample::Continuous_sample &),
            sample::Continuous_sample,
            Continuous_sample_vector_adapter>(
                    continuous_sample_log_posterior,
                    cs,
                    deltas,
                    Continuous_sample_vector_adapter()
            );
}

}

#endif // CONTINUOUS_SAMPLE_VECTOR_ADAPTER_HPP
