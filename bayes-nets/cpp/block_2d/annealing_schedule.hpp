#ifndef ANNEALING_SCHEDULE_HPP
#define ANNEALING_SCHEDULE_HPP

#include <math.h>

namespace fracture
{
namespace block_2d
{

class Annealing_schedule
{
public:
    Annealing_schedule(){}
    virtual ~Annealing_schedule();
    virtual double get_temperature(unsigned time) = 0;
};

class No_annealing_schedule : public Annealing_schedule
{
public:
    No_annealing_schedule(){}
    virtual ~No_annealing_schedule();
    virtual double get_temperature(unsigned time);
};

class Traditional_annealing_schedule : public Annealing_schedule
{
public:
    Traditional_annealing_schedule(double initial_temp, double alpha) :
        alpha_(alpha),
        last_time_(0),
        last_temperature_(initial_temp)
    {}
    virtual ~Traditional_annealing_schedule();
    virtual double get_temperature(unsigned time);
private:
    double alpha_;
    unsigned last_time_;
    double last_temperature_;
};

}
}

#endif // ANNEALING_SCHEDULE_HPP
