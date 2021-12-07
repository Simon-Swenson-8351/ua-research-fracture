#ifndef STATE_HPP
#define STATE_HPP

#include <boost/serialization/access.hpp>

#include "hidden_state.hpp"
#include "observed_state.hpp"

namespace fracture { namespace block_2d {

class State
{
    friend class boost::serialization::access;
public:
    State() {}
    State(const Hidden_state & hs, const Observed_state & os): hs_(hs), os_(os) {}

    // Used to construct the root fragment's initial state
    State(const Camera & c, const Block_geom & bg, double x, double y) :
        hs_(c, bg, x, y),
        os_(hs_, c.get_image_noise_dist())
    {
    }

    // Used to construct a state from the deterministic simulation. No fracture,
    // just simulate one frame
    State(const Camera & c, const Block_geom & bg, const Hidden_state & prev_state) :
            hs_(c, bg, prev_state),
            os_(hs_, c.get_image_noise_dist())
    {
    }

    // Nudges the state by the given offsets, then simulates deterministically
    // for one frame.
    State(const Camera & c, const Block_geom & bg, const Hidden_state & prev_state, double x_offset, double x_vel_offset, double ang_vel_offset) :
        hs_(c, bg, prev_state, x_offset, x_vel_offset, ang_vel_offset),
        os_(hs_, c.get_image_noise_dist())
    {
    }

    const Hidden_state & get_hidden_state() const { return hs_; }
    Hidden_state & get_hidden_state() { return hs_; }
    const Observed_state & get_observed_state() const { return os_; }
    Observed_state & get_observed_state() { return os_; }

    void set_hidden_state(const Hidden_state & hs) { hs_ = hs; }

    double log_prob(const kjb::Normal_distribution & image_noise_dist) const
    {
        return os_.log_prob(hs_.get_image_polygon(), image_noise_dist);
    }

    bool operator==(const State &other) const
    {
        return (hs_ == other.hs_)
                && (os_ == other.os_);
    }

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & hs_ & os_;
    }
private:
    Hidden_state hs_;
    Observed_state os_;
};

}}

#endif // STATE_HPP
