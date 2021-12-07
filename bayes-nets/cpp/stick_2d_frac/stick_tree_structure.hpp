#ifndef FRACTURE_TREE_STRUCTURE_HPP
#define FRACTURE_TREE_STRUCTURE_HPP

#include <memory>
#include <vector>
#include <stack>

#include <boost/serialization/access.hpp>
#include <boost/random.hpp>

#include <prob_cpp/prob_distribution.h>

namespace stick_2d_frac { namespace sample {

/**
 * @brief Represents the structure of the various fractures from an initial
 * stick (the root) to its children, grandchildren, etc. This class is only
 * meant to represent the *structure* of the fracture, so it does not contain
 * any data other than the structure of the tree.
 */
class Stick_tree_structure
{
    friend class boost::serialization::access;
// Static stuff
public:
    // Useful for tree traversal.
    enum Direction {
        D_LEFT,
        D_RIGHT
    };

    class Static {
    public:
        Static() :
            default_frac_chance_(0.5)
        {
            std::vector<double> tmp = {0.25, 0.25, 0.25, 0.25};
            default_single_depth_dist_ = kjb::Categorical_distribution<>(tmp);
        }
        const kjb::Categorical_distribution<> & get_default_single_depth_dist() const {
            return default_single_depth_dist_;
        }
        double get_default_frac_chance() const {
            return default_frac_chance_;
        }
    private:
        kjb::Categorical_distribution<> default_single_depth_dist_;
        double default_frac_chance_;
    };
    static const Static STATIC;

    static std::unique_ptr<std::stack<Direction>> id_to_directions(unsigned id);
    static unsigned directions_to_id(const std::stack<Stick_tree_structure::Direction> & directions);
    static unsigned directions_to_id(std::stack<Stick_tree_structure::Direction> & directions);
// Member stuff
public:
    // Initialize from a single, deterministic depth
    Stick_tree_structure(int depth, unsigned id = 0);

    // Initialize from a single, sampled depth. A distribution is given, and we will
    // perform the sample.
    Stick_tree_structure(const kjb::Categorical_distribution<> & dist = STATIC.get_default_single_depth_dist(), unsigned id = 1);

    // Initialize from a variable depth. Essentially, this entails that there is a
    // binary variable at each node in the tree. If the variable is sampled as 1, a
    // fracture happens, otherwise, no fracture happens.
    Stick_tree_structure(double frac_chance = STATIC.get_default_frac_chance(), unsigned id = 1);

    ~Stick_tree_structure();

    unsigned get_id() const;
    bool is_leaf() const;
    const Stick_tree_structure * get_left() const;
    const Stick_tree_structure * get_right() const;

    // Might need this for the vector adapter.
    unsigned get_max_id_in_tree() const;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & l_ & r_ & id_;
    }
private:
    Stick_tree_structure * l_;
    Stick_tree_structure * r_;

    unsigned id_;
};

}}

#endif // FRACTURE_TREE_STRUCTURE_H
