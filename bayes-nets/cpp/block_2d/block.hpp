#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <vector>

#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>

#include "initial_block_rvs.hpp"
#include "state.hpp"
#include "block_geom.hpp"

namespace fracture { namespace block_2d {

class Block
{
    friend class boost::serialization::access;

    class Timestamp_out_of_bounds_exception : std::exception
    {
//    public:
//        virtual const char *what() noexcept;
    };

public:

    // required for serialization/deserialization
    Block() {}

    Block(const Camera & c, const Initial_block_rvs & rvs) :
            geom_(rvs.get_initial_width(), rvs.get_initial_height()),
            init_timestamp_(0)
    {
        states_over_time_.push_back(State(c, geom_, rvs.get_initial_x(), rvs.get_initial_y()));
    }

    Block(
            const Camera & c,
            const Block_geom & my_geom,
            const State & init_state,
            size_t init_timestamp,
            size_t final_timestamp
    ) : 
            geom_(my_geom),
            init_timestamp_(init_timestamp)
    {
        states_over_time_.push_back(init_state);
        for(size_t idx = 1; idx < final_timestamp - init_timestamp; idx++)
        {
            states_over_time_.push_back(State(c, geom_, states_over_time_[idx - 1].get_hidden_state()));
        }
    }

    const Block_geom & get_local_geometry() const { return geom_; }

    bool is_active(size_t timestamp) const
    {
        return (timestamp >= init_timestamp_)
                && (timestamp < (states_over_time_.size() + init_timestamp_));
    }

    const State & get_state(size_t timestamp) const;

    void update_projections(const Camera & c)
    {
        for(State & s : states_over_time_)
        {
            s.get_hidden_state().update_projection(c);
        }
    }

    void update_initial_hidden_state(const Camera & c, const Hidden_state & hs)
    {
        states_over_time_[0].set_hidden_state(hs);
        for(size_t i = 1; i < states_over_time_.size(); i++)
        {
            states_over_time_[i].set_hidden_state(Hidden_state(c, geom_, states_over_time_[i - 1].get_hidden_state()));
        }
    }

    void set_geometry(const Block_geom & g) { geom_ = g; }

    double log_prob(const kjb::Normal_distribution & image_noise_dist) const
    {
        double accum = 0.0;
        for(const State & s : states_over_time_)
        {
            accum += s.log_prob(image_noise_dist);
        }
        return accum;
    }

    bool operator==(const Block &other) const;

    void recalculate_parent_values(const Camera &c, const Initial_block_rvs &ibr);
    void recalculate_child_values(const Camera &c, const Initial_block_rvs &ibr, const Block_geom &parent_geom, const Hidden_state &parent_state, const Fracture_rvs &fr, Block_geom::Fragment_side fs);
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & geom_ & init_timestamp_ & states_over_time_;
    }
private:
    Block_geom geom_;
    size_t init_timestamp_;
    std::vector<State> states_over_time_;
};

}}

#endif // BLOCK_HPP
