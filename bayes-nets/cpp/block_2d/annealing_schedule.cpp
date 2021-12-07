#include "annealing_schedule.hpp"

namespace fracture
{
namespace block_2d
{

Annealing_schedule::~Annealing_schedule(){}

No_annealing_schedule::~No_annealing_schedule(){}
double No_annealing_schedule::get_temperature(unsigned time){return 1.0;}

Traditional_annealing_schedule::~Traditional_annealing_schedule(){}
double Traditional_annealing_schedule::get_temperature(unsigned time)
{
    last_temperature_ = std::pow(alpha_, time - last_time_) * last_temperature_;
    last_time_ = time;
    return last_temperature_;
}

}
}
