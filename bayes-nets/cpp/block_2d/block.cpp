#include "block.hpp"
#include "util.hpp"

namespace fracture {
namespace block_2d {

const State & Block::get_state(size_t timestamp) const
{
    if(!is_active(timestamp)) throw Timestamp_out_of_bounds_exception();//util::err_str(__FILE__, __LINE__);
    return states_over_time_[timestamp - init_timestamp_];
}

bool Block::operator==(const Block &other) const
{
    return (geom_ == other.geom_)
            && (init_timestamp_ == other.init_timestamp_)
            && (states_over_time_ == other.states_over_time_);
}

void Block::recalculate_parent_values(const Camera &c, const Initial_block_rvs &ibr)
{
    geom_.recalculate_parent_values(ibr);
    states_over_time_[0].get_hidden_state().recalculate_initial_parent_state_values(c, ibr, geom_);
}

void Block::recalculate_child_values(const Camera &c, const Initial_block_rvs &ibr, const Block_geom &parent_geom, const Hidden_state &parent_state, const Fracture_rvs &fr, Block_geom::Fragment_side fs)
{
    geom_.recalculate_child_values(ibr, fr.get_fracture_location(), fs);
    states_over_time_[0].get_hidden_state().recalculate_fracture_values(c, parent_geom, parent_state, geom_, fr, fs);
    for(unsigned i = 1; i < states_over_time_.size(); i++)
    {
        states_over_time_[i].get_hidden_state().recalculate_no_fracture_values(c, geom_, states_over_time_[i - 1].get_hidden_state());
    }
}

}
}
