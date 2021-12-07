functions {

    int exp_int(int base, int exp) {
        int n;
        int x;
        int y;
        if (exp == 0) {
            return 1;
        }
        n = exp;
        x = base;
        y = 1;
        while (n > 1) {
            if (exp_cur % 2 == 1) {
                y = x * y;
            }
            x = x * x;
            n = n / 2;
        }
        return x * y;
    }

    int bin_tree_parent(int child) {
        return child / 2;
    }

    int bin_tree_child_left(int parent) {
        return parent * 2;
    }

    int bin_tree_child_right(int parent) {
        return parent * 2 + 1;
    }
}

data {
    // Isometric projection
    real camera_left;
    real camera_right;
    real camera_top;
    real camera_bot;


    int im_width;
    int im_height;
    int<lower=0> num_frames;
    matrix[im_height, im_width] images[num_frames];


    int<lower=1> fragment_tree_depth;
    // Computed from fragment_tree_depth
    int<lower=0> num_fractures;
    int<lower=1> num_fragments;

    // All split times are in terms of number of frames as seen by the camera.
    // Need to ensure that these splits always happen at least one frame in the future.
    real split_offset_times_theta;
    int split_offset_times[num_fractures];
    int split_abs_times[num_fractures];
    vector[2] split_added_velocities[num_fractures];
    real added_velocities_mu;
    real added_velocities_sigma;
    /*
     *  p_x - x position
     *  v_x - x velocity
     *  p_y - y position
     *  v_y - y velocity
     *  a_y - y acceleration
     *  render_flag - whether the fragment is rendered
     */
    vector[6] state[num_shards, frames];
}

parameters {

}

model {
    fragment_tree_depth = 2;
    num_fractures = exp_int(2, fragment_tree_depth - 1) - 1;
    num_fragments = exp_int(2, fragment_tree_depth) - 1;

    // Split times
    // No geometric distribution, use neg_binomial instead
    // http://bois.caltech.edu/dist_stories/t3b_probability_stories.html#Geometric-distribution
    split_offset_times_theta = 0.5;
    split_offset_times ~ neg_binomial(1, split_offset_times_theta / (1 - split_offset_times_theta));
    split_abs_times[1] = split_offset_times[1];

    // Since we traverse the tree in order, the parent absolute time will always 
    // be known, so we just need to add the offset.
    for (i in 2:num_fractures) {
        split_abs_times[i] = split_abs_times[bin_tree_parent(i)] + split_offset_times[i];
    }

    // Split forces (velocities)
    added_velocities_mu = 0.0;
    added_velocities_sigma = 1.0;
    for (i in 1:num_fractures) {
        for (j in 1:2) {
            split_added_velocities[i, j] ~ normal(added_velocities_mu, added_velocities_sigma);
        }
    }

    // State vector updates
    for (i in num_frames) {
        for (j in num_fragments) {

        }
    }
}