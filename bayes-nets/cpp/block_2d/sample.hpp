#ifndef SAMPLE_HPP
#define SAMPLE_HPP

#include <vector>
#include <memory>
#include <utility>

#include <boost/serialization/access.hpp>

#include <prob_cpp/prob_sample.h>

#include "camera.hpp"
#include "initial_block_rvs.hpp"
#include "fracture_rvs.hpp"
#include "block.hpp"
#include "block_geom.hpp"
#include "hidden_state.hpp"

namespace fracture { namespace block_2d {

class Sample {
    friend class boost::serialization::access;
    typedef double(*distance_metric)(const kjb::Vector &, const kjb::Vector &);
public:


    // required for serialization/deserialization
    Sample() {}

    // Constructs a new Sample using the forward sampling strategy.
    Sample(unsigned num_ims, unsigned im_w, unsigned im_h, double cam_fps) :
            num_ims_(num_ims),
            cam_(im_w, im_h, cam_fps),
            init_block_rvs_(cam_),
            parent_block_(cam_, init_block_rvs_),
            frac_rvs_(parent_block_.get_local_geometry()),
            left_block_(init_child_block(Block_geom::Fragment_side::FS_LEFT, num_ims)),
            right_block_(init_child_block(Block_geom::Fragment_side::FS_RIGHT, num_ims))
    {}

    unsigned get_num_ims() const { return num_ims_; }
    const Camera & get_camera() const { return cam_; }
    Camera & get_camera() { return cam_; }
    const Initial_block_rvs & get_initial_block_rvs() const { return init_block_rvs_; }
    Initial_block_rvs & get_initial_block_rvs() { return init_block_rvs_; }
    const Block & get_parent_block() const { return parent_block_; }
    Block & get_parent_block() { return parent_block_; }
    const Fracture_rvs & get_fracture_rvs() const { return frac_rvs_; }
    Fracture_rvs & get_fracture_rvs() { return frac_rvs_; }
    const Block & get_left_block() const { return left_block_; }
    Block & get_left_block() { return left_block_; }
    const Block & get_right_block() const { return right_block_; }
    Block & get_right_block() { return right_block_; }

    std::vector<const kjb::Matrix_d<3,4> *> get_image_polygon_actual(unsigned timestamp) const
    {
        std::vector<const kjb::Matrix_d<3,4> *> r;
        if(timestamp == 0)
        {
            r.push_back(&parent_block_.get_state(timestamp).get_hidden_state().get_image_polygon());
        }
        else{
            r.push_back(&left_block_.get_state(timestamp).get_hidden_state().get_image_polygon());
            r.push_back(&right_block_.get_state(timestamp).get_hidden_state().get_image_polygon());
        }
        return r;
    }

    std::vector<const kjb::Matrix_d<3,4> *> get_image_polygon_observed(unsigned timestamp) const
    {
        std::vector<const kjb::Matrix_d<3,4> *> r;
        if(timestamp == 0)
        {
            r.push_back(&parent_block_.get_state(timestamp).get_observed_state().get_image_polygon());
        }
        else{
            r.push_back(&left_block_.get_state(timestamp).get_observed_state().get_image_polygon());
            r.push_back(&right_block_.get_state(timestamp).get_observed_state().get_image_polygon());
        }
        return r;
    }

    std::vector<const kjb::Vector_d<3> *> get_image_center_of_mass(unsigned timestamp) const
    {
        std::vector<const kjb::Vector_d<3> *> r;
        if(timestamp == 0)
        {
            r.push_back(&parent_block_.get_state(timestamp).get_hidden_state().get_image_center_of_mass());
        }
        else
        {
            r.push_back(&left_block_.get_state(timestamp).get_hidden_state().get_image_center_of_mass());
            r.push_back(&right_block_.get_state(timestamp).get_hidden_state().get_image_center_of_mass());
        }
        return r;
    }

    // I decided to make these top-level of Sample class, since various other variables
    // may depend on these.
    void set_camera_top(double val);
    void set_block_initial_x(double val);
    void set_block_initial_y(double val);
    void set_block_initial_width(double val);
    void set_block_initial_height(double val);
    void set_fracture_location(double val);
    void set_right_x_momentum(double val);
    void set_left_angular_momentum(double val);
    void forward_sample_hidden_rvs();

    double log_prob() const
    {
        return cam_.log_prob() // 1 variable: c_t
                + init_block_rvs_.log_prob() // 4 variables: w, h, x|c_t, y|c_t
                + frac_rvs_.log_prob() // 3 variables: loc|w, mom, ang_mom
                // total hidden rvs: 8
                + parent_block_.log_prob(cam_.get_image_noise_dist()) // 4 variables: 4 endpoints * 1 frame
                + left_block_.log_prob(cam_.get_image_noise_dist()) // 116 variables: 4 endpoints * 29 frames
                + right_block_.log_prob(cam_.get_image_noise_dist()) // 116 variables: 4 endpoints * 29 frames
                // total observed rvs: 236
                ;
    }

    void recalculate_values();

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & num_ims_ & cam_ & init_block_rvs_ & parent_block_ & frac_rvs_ & left_block_ & right_block_;
        recalculate_values();
    }

    // Used for aggregation
    Sample &add_rvs(Sample &other);
    Sample &div_rvs(double d);

    bool operator==(const Sample &other) const;
private:
    Block init_child_block(Block_geom::Fragment_side fs, unsigned num_ims);
    void update_camera_depends();
    void update_initial_block_pos_depends();
    void update_initial_block_geom_depends();
    void update_child_hidden_states();

    unsigned num_ims_;

    Camera cam_;

    Initial_block_rvs init_block_rvs_;
    Block parent_block_;

    Fracture_rvs frac_rvs_;
    Block left_block_;
    Block right_block_;
};

std::tuple<std::shared_ptr<Sample>, double, unsigned> aggregate_samples(const std::vector<std::tuple<std::shared_ptr<Sample>, double, unsigned>> & vec, size_t first, size_t last);


class Data_record_20191031
{
    friend class boost::serialization::access;
public:
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & rng_seed_ & sample_;
    }

    unsigned rng_seed_;
    Sample sample_;
};

class Inference_record_20191031
{
    friend class boost::serialization::access;
public:
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & rng_seed_ & samples_;
    }

    unsigned rng_seed_;
    std::vector<Sample *> samples_;
};

void save_sample(
        const std::string &data_dir,
        unsigned data_idx,
        const Sample &in);
void load_sample(
        const std::string &data_dir,
        unsigned data_idx,
        Sample &out);
void save_data_record(
        const std::string &data_dir,
        unsigned data_idx,
        const Data_record_20191031 &in);
void load_data_record(
        const std::string &data_dir,
        unsigned data_idx,
        Data_record_20191031 &out);
void save_sample_vector(
        const std::string &data_dir,
        unsigned data_idx,
        util::Inference_type it,
        const std::vector<unsigned> &flex_vars,
        const std::vector<Sample *> &in);
void load_sample_vector(
        const std::string &data_dir,
        unsigned dataset_idx,
        util::Inference_type it,
        const std::vector<unsigned> &flex_vars,
        std::vector<Sample *> &out);
void save_inference_record(
        const std::string &data_dir,
        unsigned data_idx,
        util::Inference_type it,
        const std::vector<unsigned> &flex_vars,
        const Inference_record_20191031 &in);
void load_inference_record(
        const std::string &data_dir,
        unsigned dataset_idx,
        util::Inference_type it,
        const std::vector<unsigned> &flex_vars,
        Inference_record_20191031 &out);

// TODO these are related to inference results, and should probably be
// put somewhere else. Sample really should be agnostic to inference.
void save_csv(
        const std::string &data_dir,
        unsigned data_idx,
        util::Inference_type it,
        const std::vector<unsigned> &flex_vars,
        const std::vector<Sample *> &s,
        const std::vector<double> &log_prob,
        const std::vector<bool> &accepted,
        double forward_sample_log_prob);

void write_csv_line(
        std::ofstream &f,
        unsigned row_num,
        const block_2d::Sample &s,
        double log_prob,
        bool accepted,
        double forward_sample_log_prob);

void save_comparative_csv(
        const std::string &data_dir,
        unsigned data_idx,
        const Sample &data,
        util::Inference_type it,
        const std::vector<unsigned> &flex_vars,
        const std::vector<Sample *> &inference);

void write_comparative_csv_line(
        std::ofstream &f,
        const Sample &data,
        const unsigned chain_idx,
        const unsigned sample_idx,
        const Sample &inference);

}}

#endif // SAMPLE_HPP
