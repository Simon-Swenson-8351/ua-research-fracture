#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "config.hpp"
#include "sample.hpp"
#include "util.hpp"
#include "sample_vector_adapter.hpp"

namespace fracture
{
namespace block_2d
{

// This file needs a bit of work. The problem is that, when one rvs is updated,
// all deterministic updates from that must be updated as well. It was a chore
// to track all those down, but I think I've got them all at this point. However,
// bug-free is not a guarantee.
void Sample::set_camera_top(double val)
{
    cam_.set_camera_top(val);
    update_camera_depends();
}

void Sample::set_block_initial_x(double val)
{
    init_block_rvs_.set_initial_x(val);
    update_initial_block_pos_depends();
}

void Sample::set_block_initial_y(double val)
{
    init_block_rvs_.set_initial_y(val);
    update_initial_block_pos_depends();
}

void Sample::set_block_initial_width(double val)
{
    // 20191217 experiment: don't let the block get small enough
    // for a large angular momentum to make sense
    /*
    init_block_rvs_.set_initial_width(val);
    update_initial_block_geom_depends();
    */
}

void Sample::set_block_initial_height(double val)
{
    // 20191217 experiment: don't let the block get small enough
    // for a large angular momentum to make sense
    /*
    init_block_rvs_.set_initial_height(val);
    update_initial_block_geom_depends();
    */
}

void Sample::set_fracture_location(double val)
{
    frac_rvs_.set_fracture_location(val);
    left_block_.set_geometry(Block_geom(frac_rvs_.get_fracture_location(), init_block_rvs_.get_initial_height()));
    right_block_.set_geometry(Block_geom(init_block_rvs_.get_initial_width() - frac_rvs_.get_fracture_location(), init_block_rvs_.get_initial_height()));
    update_child_hidden_states();
}

void Sample::set_right_x_momentum(double val)
{
    frac_rvs_.set_right_x_momentum(val);
    update_child_hidden_states();
}

void Sample::set_left_angular_momentum(double val)
{
    frac_rvs_.set_left_angular_momentum(val);
    update_child_hidden_states();
}

void Sample::update_camera_depends()
{
    init_block_rvs_.update_distributions(cam_);
    parent_block_.update_projections(cam_);
    left_block_.update_projections(cam_);
    right_block_.update_projections(cam_);
}

void Sample::update_initial_block_pos_depends()
{
    parent_block_.update_initial_hidden_state(cam_, Hidden_state(cam_, parent_block_.get_local_geometry(), init_block_rvs_.get_initial_x(), init_block_rvs_.get_initial_y()));
    update_child_hidden_states();
}

void Sample::update_initial_block_geom_depends()
{
    parent_block_.set_geometry(Block_geom(init_block_rvs_.get_initial_width(), init_block_rvs_.get_initial_height()));
    parent_block_.update_initial_hidden_state(cam_, Hidden_state(
            cam_,
            parent_block_.get_local_geometry(),
            init_block_rvs_.get_initial_x(),
            init_block_rvs_.get_initial_y()));
    frac_rvs_.init_fracture_location(parent_block_.get_local_geometry());
    left_block_.set_geometry(Block_geom(frac_rvs_.get_fracture_location(), init_block_rvs_.get_initial_height()));
    right_block_.set_geometry(Block_geom(init_block_rvs_.get_initial_width() - frac_rvs_.get_fracture_location(), init_block_rvs_.get_initial_height()));
    update_child_hidden_states();
}

void Sample::update_child_hidden_states()
{
    const Block_geom & lbgeom = left_block_.get_local_geometry();
    const Block_geom & rbgeom = right_block_.get_local_geometry();
    left_block_.update_initial_hidden_state(cam_, Hidden_state(
            cam_,
            lbgeom,
            parent_block_.get_state(0).get_hidden_state(),
            lbgeom.fracture_x_offset(parent_block_.get_local_geometry(), Block_geom::FS_LEFT),
            lbgeom.momentum_to_velocity(frac_rvs_.get_left_x_momentum()) / cam_.get_frames_per_second(),
            lbgeom.momentum_to_velocity(frac_rvs_.get_left_angular_momentum()) / cam_.get_frames_per_second()
    ));
    right_block_.update_initial_hidden_state(cam_, Hidden_state(
            cam_,
            rbgeom,
            parent_block_.get_state(0).get_hidden_state(),
            rbgeom.fracture_x_offset(parent_block_.get_local_geometry(), Block_geom::FS_RIGHT),
            rbgeom.momentum_to_velocity(frac_rvs_.get_right_x_momentum()) / cam_.get_frames_per_second(),
            rbgeom.momentum_to_velocity(frac_rvs_.get_right_angular_momentum()) / cam_.get_frames_per_second()
    ));
}

void Sample::forward_sample_hidden_rvs()
{
    // TODO this is very inefficient to do one at a time. I'm doing this now to save a bit
    // of code, but it's doing very similar state propagation updates every time, 8 times
    // over.
    // We want to update the data structure in one shot, but essentially are doing so 8
    // times and propagating those calculated values.
    set_camera_top(prob::sample(Camera::C_T_DIST));
    set_block_initial_x(kjb::sample(init_block_rvs_.get_initial_x_distribution()));
    set_block_initial_y(kjb::sample(init_block_rvs_.get_initial_y_distribution()));
    set_block_initial_width(prob::sample(Initial_block_rvs::INIT_WIDTH_DIST));
    set_block_initial_height(prob::sample(Initial_block_rvs::INIT_HEIGHT_DIST));
    set_fracture_location(prob::sample(frac_rvs_.get_fracture_location_distribution()));
    set_right_x_momentum(prob::sample(Fracture_rvs::R_X_MOMENTUM_DIST));
    set_left_angular_momentum(prob::sample(Fracture_rvs::L_ANGULAR_MOMENTUM_DIST));
}

Block Sample::init_child_block(Block_geom::Fragment_side fs, unsigned num_ims)
{
    double w;
    double x_mom;
    double ang_mom;
    switch(fs)
    {
    case Block_geom::FS_LEFT:
        w = frac_rvs_.get_fracture_location();
        x_mom = frac_rvs_.get_left_x_momentum();
        ang_mom = frac_rvs_.get_left_angular_momentum();
        break;
    case Block_geom::FS_RIGHT:
        w = init_block_rvs_.get_initial_width() - frac_rvs_.get_fracture_location();
        x_mom = frac_rvs_.get_right_x_momentum();
        ang_mom = frac_rvs_.get_right_angular_momentum();
        break;
    default:
        throw util::Unhandled_enum_value_exception(); //util::err_str(__FILE__, __LINE__);
    }
    Block_geom bg(w, init_block_rvs_.get_initial_height());
    State s(
            cam_,
            bg,
            parent_block_.get_state(0).get_hidden_state(),
            bg.fracture_x_offset(parent_block_.get_local_geometry(), fs),
            bg.momentum_to_velocity(x_mom) / cam_.get_frames_per_second(),
            bg.momentum_to_velocity(ang_mom) / cam_.get_frames_per_second());
    return Block(cam_, bg, s, 1, num_ims);
}
std::tuple<std::shared_ptr<Sample>, double, unsigned> aggregate_samples(const std::vector<std::tuple<std::shared_ptr<Sample>, double, unsigned>> & vec, size_t first, size_t last)
{
    double log_prob = 0.0;
    unsigned number_acceptances = 0;
    for(size_t i = first; i < last; i++)
    {
        log_prob += std::get<1>(vec[i]);
        number_acceptances += std::get<2>(vec[i]);
    }
    log_prob /= (last - first);
    return std::tuple<std::shared_ptr<Sample>, double, unsigned>(std::get<0>(vec[(first + last) / 2]), log_prob, number_acceptances);
}

void Sample::recalculate_values()
{
    init_block_rvs_.recalculate_values(cam_);
    parent_block_.recalculate_parent_values(cam_, init_block_rvs_);
    frac_rvs_.recalculate_values(parent_block_.get_local_geometry());
    left_block_.recalculate_child_values(cam_, init_block_rvs_, parent_block_.get_local_geometry(), parent_block_.get_state(0).get_hidden_state(), frac_rvs_, Block_geom::FS_LEFT);
    right_block_.recalculate_child_values(cam_, init_block_rvs_, parent_block_.get_local_geometry(), parent_block_.get_state(0).get_hidden_state(), frac_rvs_, Block_geom::FS_RIGHT);
}

bool Sample::operator==(const Sample &other) const
{
    return (num_ims_ == other.num_ims_)
            && (cam_ == other.cam_)
            && (init_block_rvs_ == other.init_block_rvs_)
            && (parent_block_ == other.parent_block_)
            && (frac_rvs_ == other.frac_rvs_)
            && (left_block_ == other.left_block_)
            && (right_block_ == other.right_block_);
}


Sample &Sample::add_rvs(Sample &other)
{
    Sample_vector_adapter sva;
    for(size_t i = 0; i < sva.size(this); i++)
    {
        sva.set(this, i, sva.get(&other, i));
    }
}

Sample &Sample::div_rvs(double d)
{
    Sample_vector_adapter sva;
    for(size_t i = 0; i < sva.size(this); i++)
    {
        sva.set(this, i, sva.get(this, i) / d);
    }
}

void save_sample(
        const std::string &data_dir,
        unsigned data_idx,
        const Sample &in)
{
    std::ofstream ar_f(
            util::get_data_archive_path(
                    data_dir,
                    data_idx).string(),
            std::ios_base::out | std::ios_base::trunc
    );
    boost::archive::text_oarchive ar_far(ar_f);
    ar_far << in;
}

void load_sample(
        const std::string &data_dir,
        unsigned data_idx,
        Sample &out)
{
    std::ifstream ar_file(util::get_data_archive_path(data_dir, data_idx).string());
    boost::archive::text_iarchive ar(ar_file);
    ar >> out;
}

void save_data_record(
        const std::string &data_dir,
        unsigned data_idx,
        const Data_record_20191031 &in)
{
    std::ofstream ar_f(
            util::get_data_archive_path(
                    data_dir,
                    data_idx).string(),
            std::ios_base::out | std::ios_base::trunc
    );
    boost::archive::text_oarchive ar_far(ar_f);
    ar_far << in;
}

void load_data_record(
        const std::string &data_dir,
        unsigned data_idx,
        Data_record_20191031 &out)
{
    std::ifstream ar_file(util::get_data_archive_path(data_dir, data_idx).string());
    boost::archive::text_iarchive ar(ar_file);
    ar >> out;
}

void save_sample_vector(
        const std::string &data_dir,
        unsigned data_idx,
        util::Inference_type it,
        const std::vector<unsigned> &flex_vars,
        const std::vector<Sample *> &in)
{
    std::ofstream ar_file(
            util::get_inference_archive_path(
                    data_dir,
                    data_idx,
                    it,
                    flex_vars).string(),
            std::ios_base::out | std::ios_base::trunc
    );
    boost::archive::text_oarchive ar_far(ar_file);
    ar_far << in;
}

void load_sample_vector(
        const std::string &data_dir,
        unsigned dataset_idx,
        util::Inference_type it,
        const std::vector<unsigned> &flex_vars,
        std::vector<Sample *> &out)
{
    std::string p = util::get_inference_archive_path(
                data_dir,
                dataset_idx,
                it,
                flex_vars).string();
    std::ifstream ar_file(
            p,
            std::ios_base::in
    );
    boost::archive::text_iarchive ar_far(ar_file);
    ar_far >> out;
}

void save_inference_record(
        const std::string &data_dir,
        unsigned data_idx,
        util::Inference_type it,
        const std::vector<unsigned> &flex_vars,
        const Inference_record_20191031 &in)
{
    std::ofstream ar_file(
            util::get_inference_archive_path(
                    data_dir,
                    data_idx,
                    it,
                    flex_vars).string(),
            std::ios_base::out | std::ios_base::trunc
    );
    boost::archive::text_oarchive ar_far(ar_file);
    ar_far << in;
}

void load_inference_record(
        const std::string &data_dir,
        unsigned dataset_idx,
        util::Inference_type it,
        const std::vector<unsigned> &flex_vars,
        Inference_record_20191031 &out)
{
    std::string p = util::get_inference_archive_path(
                data_dir,
                dataset_idx,
                it,
                flex_vars).string();
    std::cout << p << "\n";
    std::ifstream ar_file(
            p,
            std::ios_base::in
    );
    boost::archive::text_iarchive ar_far(ar_file);
    ar_far >> out;
}

static std::string get_csv_attr(
        util::Csv_record_attrs cra,
        unsigned row_num,
        const block_2d::Sample &s,
        double log_prob,
        bool accepted,
        double forward_sample_log_prob)
{
    switch(cra)
    {
    case util::CRA_ROW_NUM:
        return std::to_string(row_num);
        break;
    case util::CRA_CAM_TOP:
        return std::to_string(s.get_camera().get_camera_top());
        break;
    case util::CRA_INIT_X_POS:
        return std::to_string(s.get_initial_block_rvs().get_initial_x());
        break;
    case util::CRA_INIT_Y_POS:
        return std::to_string(s.get_initial_block_rvs().get_initial_y());
        break;
    case util::CRA_INIT_WIDTH:
        return std::to_string(s.get_initial_block_rvs().get_initial_width());
        break;
    case util::CRA_INIT_HEIGHT:
        return std::to_string(s.get_initial_block_rvs().get_initial_height());
        break;
    case util::CRA_RIGHT_MOMENTUM:
        return std::to_string(s.get_fracture_rvs().get_right_x_momentum());
        break;
    case util::CRA_LEFT_ANGULAR_MOMENTUM:
        return std::to_string(s.get_fracture_rvs().get_left_angular_momentum());
        break;
    case util::CRA_FRAC_LOC:
        return std::to_string(s.get_fracture_rvs().get_fracture_location());
        break;
    case util::CRA_LOG_PROB:
        return std::to_string(log_prob);
        break;
    case util::CRA_ACCEPTED:
        return std::to_string(accepted);
        break;
    case util::CRA_FORWARD_SAMPLE_LOG_PROB:
        return std::to_string(forward_sample_log_prob);
        break;
    default:
        throw util::Unhandled_enum_value_exception(); //util::err_str(__FILE__, __LINE__);
    }
}

void save_csv(
        const std::string &data_dir,
        unsigned data_idx,
        util::Inference_type it,
        const std::vector<unsigned> &flex_vars,
        const std::vector<Sample *> &s,
        const std::vector<double> &log_prob,
        const std::vector<bool> &accepted,
        double forward_sample_log_prob)
{
    boost::filesystem::path p = util::get_csv_path(data_dir, data_idx, it, flex_vars);
    std::ofstream f(p.string(), std::ios_base::out | std::ios_base::trunc);
    util::write_csv_header(f);
    f << "\n";
    for(unsigned i = 0; i < s.size(); i++)
    {
        write_csv_line(
                f,
                i,
                *s[i],
                log_prob[i],
                accepted[i],
                forward_sample_log_prob);
    }
}

void write_csv_line(
        std::ofstream & f,
        unsigned row_num,
        const block_2d::Sample &s,
        double log_prob,
        bool accepted,
        double forward_sample_log_prob)
{
    bool written = false;
    for(unsigned i = 0; i < util::CRA_COUNT; i++)
    {
        if(!util::RECORD_ATTRS_TO_WRITE[i])
        {
            continue;
        }
        if(written)
        {
            f << ",";
        }
        f << "\"" << get_csv_attr(
                 util::Csv_record_attrs(i),
                 row_num,
                 s,
                 log_prob,
                 accepted,
                 forward_sample_log_prob) << "\"";
        written = true;
    }
    f << "\n";
}
void cache_log_probs(
        const std::vector<std::vector<Sample *>> &samples,
        std::vector<std::vector<double>> &log_probs)
{
    log_probs.clear();
    log_probs.reserve(samples.size());
    for(unsigned i = 0; i < samples.size(); i++)
    {
        log_probs.push_back(std::vector<double>());
        log_probs[i].reserve(samples[0].size());
        for(unsigned j = 0; j < samples[0].size(); j++)
        {
            log_probs[i].push_back(samples[i][j]->log_prob());
        }
    }
}

}}
