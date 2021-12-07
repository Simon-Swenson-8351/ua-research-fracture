#include <memory>
#include <utility>
#include <algorithm>

#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>

#include "stick_tree_structure.hpp"

namespace stick_2d_frac { namespace sample {

std::unique_ptr<std::stack<Stick_tree_structure::Direction>> Stick_tree_structure::id_to_directions(unsigned id)
{
    std::unique_ptr<std::stack<Stick_tree_structure::Direction>> result =
            std::unique_ptr<std::stack<Stick_tree_structure::Direction>>(
                    new std::stack<Stick_tree_structure::Direction>());
    while(id > 0)
    {
        if(id % 2 == 1)
        {
            // odd implies left
            result->push(D_LEFT);
        }
        else
        {
            // even implies right
            result->push(D_RIGHT);
        }
        id = (id - 1) / 2;
    }
    return std::move(result);
}

unsigned Stick_tree_structure::directions_to_id(const std::stack<Stick_tree_structure::Direction> & directions)
{
    std::stack<Stick_tree_structure::Direction> tmp(directions);
    return directions_to_id(tmp);
}

unsigned Stick_tree_structure::directions_to_id(std::stack<Stick_tree_structure::Direction> & directions) {
    unsigned result(0);
    while(!directions.empty()) {
        Direction dir(directions.top());
        switch(dir) {
        case D_LEFT:
            result *= 2;
            result += 1;
            // Use a bit of fall-through magic here.
        case D_RIGHT:
            result += 1;
            break;
        default:
            throw "unknown direction";
        }
        directions.pop();
    }
    return result;
}

Stick_tree_structure::Stick_tree_structure(int depth, unsigned id) :
    id_(id)
{

    if(depth > 1)
    {
        l_ = new Stick_tree_structure(depth - 1, id * 2 + 1);
        r_ = new Stick_tree_structure(depth - 1, id * 2 + 2);
    }
    else {
        l_ = nullptr;
        r_ = nullptr;
    }
}

// Need to explicitly create the vectors, else got ambiguous constructor call error.
Stick_tree_structure::Stick_tree_structure(const kjb::Categorical_distribution<> & dist, unsigned id) :
    Stick_tree_structure(kjb::sample(dist), id) { }

Stick_tree_structure::Stick_tree_structure(double frac_chance, unsigned id) :
    id_(id)
{
    static const kjb::Uniform_distribution VARIABLE_DEPTH_DIST;
    if (kjb::sample(VARIABLE_DEPTH_DIST) < frac_chance) {
        l_ = new Stick_tree_structure(frac_chance, id * 2 + 1);
        r_ = new Stick_tree_structure(frac_chance, id * 2 + 2);
    } else {
        l_ = nullptr;
        r_ = nullptr;
    }
}

Stick_tree_structure::~Stick_tree_structure() {
    if(l_) {
        delete l_;
        delete r_;
    }
}

unsigned Stick_tree_structure::get_id() const {
    return id_;
}

bool Stick_tree_structure::is_leaf() const {
    return !l_;
}

const Stick_tree_structure * Stick_tree_structure::get_left() const {
    return l_;
}

const Stick_tree_structure * Stick_tree_structure::get_right() const {
    return r_;
}

unsigned Stick_tree_structure::get_max_id_in_tree() const {
    if(!l_)
    {
        return id_;
    }
    // You might assume the right subtree is guaranteed to have a higher id. However,
    // this is only the case with full trees. What if the left subtree has greater
    // depth?
    return std::max(l_->get_max_id_in_tree(), r_->get_max_id_in_tree());
}

}}
