#include <vector>

#include <prob_cpp/prob_sample.h>

#include "config.hpp"
#include "sample.hpp"
#include "sample_vector_adapter.hpp"
#include "util.hpp"

int main(int argc, char *argv[])
{
    using namespace fracture;
    using namespace block_2d;

    cfg::Arguments_data_gen args;

    args.rng_seed_ = 42;
    size_t num_samples = 64;
    double delta = 0.00001;
    double nudge_val = 4.0;

    kjb::seed_sampling_rand(args.rng_seed_);

    for(size_t i = 0; i < num_samples; i++)
    {
        // Single entry archive
        Sample sample(args.num_ims_, args.im_w_, args.im_h_, args.cam_fps_);

        // Change our variables to be significant enough to stop explaining the data well.
        // Then, see if the log probability is worse.
        double initial_log_prob = sample.log_prob();

        sample.set_camera_top(sample.get_camera().get_camera_top()
                + nudge_val * Camera::C_T_STD);
        assert(sample.log_prob() < initial_log_prob);
        sample.set_camera_top(sample.get_camera().get_camera_top()
                - nudge_val * Camera::C_T_STD);
        assert(util::double_eq(sample.log_prob(), initial_log_prob, delta));

        sample.set_block_initial_x(sample.get_initial_block_rvs().get_initial_x()
                + nudge_val * sample.get_initial_block_rvs().get_initial_x_distribution().standard_deviation());
        assert(sample.log_prob() < initial_log_prob);
        sample.set_block_initial_x(sample.get_initial_block_rvs().get_initial_x()
                - nudge_val * sample.get_initial_block_rvs().get_initial_x_distribution().standard_deviation());
        assert(util::double_eq(sample.log_prob(), initial_log_prob, delta));

        sample.set_block_initial_y(sample.get_initial_block_rvs().get_initial_y()
                + nudge_val * sample.get_initial_block_rvs().get_initial_y_distribution().standard_deviation());
        assert(sample.log_prob() < initial_log_prob);
        sample.set_block_initial_y(sample.get_initial_block_rvs().get_initial_y()
                - nudge_val * sample.get_initial_block_rvs().get_initial_y_distribution().standard_deviation());
        assert(util::double_eq(sample.log_prob(), initial_log_prob, delta));

        sample.set_block_initial_width(sample.get_initial_block_rvs().get_initial_width()
                + nudge_val * Initial_block_rvs::INIT_WIDTH_STD);
        assert(sample.log_prob() < initial_log_prob);
        sample.set_block_initial_width(sample.get_initial_block_rvs().get_initial_width()
                - nudge_val * Initial_block_rvs::INIT_WIDTH_STD);
        assert(util::double_eq(sample.log_prob(), initial_log_prob, delta));

        sample.set_block_initial_height(sample.get_initial_block_rvs().get_initial_height()
                + nudge_val * Initial_block_rvs::INIT_HEIGHT_STD);
        assert(sample.log_prob() < initial_log_prob);
        sample.set_block_initial_height(sample.get_initial_block_rvs().get_initial_height()
                - nudge_val * Initial_block_rvs::INIT_HEIGHT_STD);
        assert(util::double_eq(sample.log_prob(), initial_log_prob, delta));

        sample.set_fracture_location(sample.get_fracture_rvs().get_fracture_location() / 100.0);
        assert(sample.log_prob() < initial_log_prob);
        sample.set_fracture_location(sample.get_fracture_rvs().get_fracture_location() * 100.0);
        assert(util::double_eq(sample.log_prob(), initial_log_prob, delta));

        sample.set_right_x_momentum(sample.get_fracture_rvs().get_right_x_momentum()
                + nudge_val * Fracture_rvs::R_X_MOMENTUM_STD);
        assert(sample.log_prob() < initial_log_prob);
        sample.set_right_x_momentum(sample.get_fracture_rvs().get_right_x_momentum()
                - nudge_val * Fracture_rvs::R_X_MOMENTUM_STD);
        assert(util::double_eq(sample.log_prob(), initial_log_prob, delta));

        sample.set_left_angular_momentum(sample.get_fracture_rvs().get_left_angular_momentum()
                + nudge_val * Fracture_rvs::L_ANGULAR_MOMENTUM_STD);
        assert(sample.log_prob() < initial_log_prob);
        sample.set_left_angular_momentum(sample.get_fracture_rvs().get_left_angular_momentum()
                - nudge_val * Fracture_rvs::L_ANGULAR_MOMENTUM_STD);
        assert(util::double_eq(sample.log_prob(), initial_log_prob, delta));


        // violate our priors and see if an exception is appropriately thrown.
        // basically, for any distribution with support subset of R, set it to
        // something that's outside the support.
        try
        {
            sample.set_camera_top(-0.01);
            assert(false);
        }
        catch(prob::No_support_exception &e)
        {
            // pass
        }
        try
        {
            sample.set_block_initial_width(-0.01);
            assert(false);
        }
        catch(prob::No_support_exception &e)
        {
            // pass
        }
        try
        {
            sample.set_block_initial_height(-0.01);
            assert(false);
        }
        catch(prob::No_support_exception &e)
        {
            // pass
        }
        try
        {
            sample.set_fracture_location(-0.01);
            assert(false);
        }
        catch(prob::No_support_exception &e)
        {
            // pass
        }
        try
        {
            sample.set_fracture_location(sample.get_initial_block_rvs().get_initial_width() + 0.01);
            assert(false);
        }
        catch(prob::No_support_exception &e)
        {
            // pass
        }
        try
        {
            sample.set_right_x_momentum(-0.01);
            assert(false);
        }
        catch(prob::No_support_exception &e)
        {
            // pass
        }
        try
        {
            sample.set_left_angular_momentum(-0.01);
            assert(false);
        }
        catch(prob::No_support_exception &e)
        {
            // pass
        }

    }

    return 0;
}
