functions {

    int rounded_real_to_int(real x, int min_val, int max_val);
    matrix draw_px(matrix im, int px_x, int px_y, real intensity);
    matrix draw_pt_non_homo(matrix im, vector non_homo_pt, matrix cam, real intensity);
    matrix draw_pt_homo(matrix im, vector homo_pt, matrix cam, real intensity);
    vector non_homo_coord_to_homo_coord(vector non_homo_coord);
    vector homo_coord_to_non_homo_coord(vector homo_coord);
    matrix cam_im_bounds_to_cam_mat(real cam_left, real cam_right, real cam_top, real cam_bot, int im_width, int im_height);
    int cam_fps_category_to_value(int cam_fps_category);

    // Stan doesn't have a method to round from real to int, which we need to 
    // draw pixels to images. Following code taken from:
    // https://discourse.mc-stan.org/t/real-to-integer-conversion/5622/6
    int rounded_real_to_int(real x, int min_val, int max_val) {
        // This assumes that min_val >= 0 is the minimum integer in range, 
        //  max_val > min_val,
        // and that x has already been rounded. 
        //  It should find the integer equivalent to x.
        int range = (max_val - min_val+1)/2; // We add 1 to make sure that truncation doesn't exclude a number
        int mid_pt = min_val + range;
        int out;
        while(range > 0) {
          if(x == mid_pt){
            out = mid_pt;
            range = 0;
          } else {
            // figure out if range == 1
            range =  (range+1)/2; 
            mid_pt = x > mid_pt ? mid_pt + range: mid_pt - range; 
            }
        }
        return out;
    }


    matrix draw_px(matrix im, int px_x, int px_y, real intensity) {
        matrix[rows(im), cols(im)] result = im;
        // check input bounds. Do nothing if oob.
        if(    px_y < 1 
            || px_y > rows(im)
            || px_x < 1
            || px_x > cols(im)
            || intensity < 0.0
            || intensity > 1.0) {
            return result;
        }
        result[px_y, px_x] = intensity;
        return result;
    }

    matrix draw_pt_non_homo(matrix im, vector non_homo_pt, matrix cam, real intensity) {
        return draw_pt_homo(im, non_homo_coord_to_homo_coord(non_homo_pt), cam, intensity);
    }

    matrix draw_pt_homo(matrix im, vector homo_pt, matrix cam, real intensity) {
        vector[rows(homo_pt) - 1] im_coord;
        int px_x;
        int px_y;
        im_coord = homo_coord_to_non_homo_coord(cam * homo_pt);
        im_coord[1] = round(im_coord[1]);
        im_coord[2] = round(im_coord[2]);

        // Need to perform bounds checking before converting to int, since the 
        // conversion routine needs bounds. We'll use the image pixel bounds.
        if (   im_coord[1] <      1.0 - 0.0001 /* Small delta after round */
            || im_coord[1] > rows(im) + 0.0001
            || im_coord[2] <      1.0 - 0.0001
            || im_coord[2] > cols(im) + 0.0001) {

            return im;
        }

        px_x = rounded_real_to_int(im_coord[2], 1, cols(im));
        px_y = rounded_real_to_int(im_coord[1], 1, rows(im));
        return draw_px(im, px_x, px_y, intensity);
    }

    vector non_homo_coord_to_homo_coord(vector non_homo_coord) {
        vector[rows(non_homo_coord) + 1] result;
        result[1:rows(non_homo_coord)] = non_homo_coord;
        result[rows(non_homo_coord) + 1] = 1.0;
        return result;
    }

    vector homo_coord_to_non_homo_coord(vector homo_coord) {
        vector[rows(homo_coord) - 1] result;
        result = homo_coord[1:(rows(homo_coord) - 1)];
        result = result * (1.0 / homo_coord[rows(homo_coord)]);
        return result;
    }

    // Given camera bounds (left, right, top, bottom) in world coordinates and 
    // image dimensions, computes the corresponding isometric camera matrix.
    matrix cam_im_bounds_to_cam_mat(real cam_left, real cam_right, real cam_top, real cam_bot, int im_width, int im_height) {
        return [[                                0, -im_height / (cam_top - cam_bot),   cam_top * im_height /   (cam_top - cam_bot) ],
                [im_width / (cam_right - cam_left),                                0, -cam_left * im_width  / (cam_right - cam_left)],
                [                                0,                                0,                                              1]];
    }

    int cam_fps_category_to_value(int cam_fps_category) {
        if(cam_fps_category == 1) {
            return 24;
        } else if(cam_fps_category == 2) {
            return 30;
        } else if(cam_fps_category == 3) {
            return 60;
        } else if(cam_fps_category == 4) {
            return 120;
        } else {
            reject("unknown cam_fps_category");
            return 0;
        }
    }
}

data {
    // Images. These are the observed variables.
    int im_width;
    int im_height;
    int num_ims;

    // This is in m/s, so should be clamped to -9.8 m/s.
    real gravity;

    // Since movement is relative to the camera, we'll clamp the camera bottom-left 
    // to (0, 0) when the model is sampled.
    real cam_bot;
    real cam_left;
    real cam_fps;

    // Whether we're estimating observed values (images)
    int<lower = 0, upper = 1> run_estimation;

    // Priors
    real cam_top_alpha;
    real cam_top_beta;

    // real p_x_0_mean;
    // We can assume the "action" is more likely to be in the center of the 
    // image, so the mean will be that.
    real p_x_0_stddev;
    real v_x_0_mean;
    real v_x_0_stddev;
    // real p_y_0_mean;
    real p_y_0_stddev;
    real v_y_0_mean;
    real v_y_0_stddev;
}

parameters {
    // Camera nodes. These should be inferred based on gravity.
    real cam_top;


    // p_x
    // v_x
    // p_y
    // v_y
    real state_0_p_x;
    real state_0_v_x;
    real state_0_p_y;
    real state_0_v_y;
}

transformed parameters {
    // This is deterministic, based on cam_frame_height and the aspect ratio of 
    // the images. ASSUMING NO STRETCHING!
    real cam_right;

    // Should be a 3x3 matrix in this case, since we're going from 2-d 
    // homogeneous coordinates to 2-d homogeneous coordinates. This is 
    // deterministic, given the camera bounds.
    matrix[3, 3] cam_matrix;

    // meters / frame
    real state_0_grav;

    vector[5] state[num_ims];

    cam_right = cam_left + im_width * (cam_top - cam_bot) / im_height;
    cam_matrix = cam_im_bounds_to_cam_mat(cam_left, cam_right, cam_top, cam_bot, im_width, im_height);
    state_0_grav = gravity / cam_fps;

    state[1, 1] = state_0_p_x;
    state[1, 2] = state_0_v_x;
    state[1, 3] = state_0_p_y;
    state[1, 4] = state_0_v_y;
    state[1, 5] = state_0_grav;
    for (i in 2:num_ims) {
        state[i] = [[1, 1, 0, 0, 0],
                    [0, 1, 0, 0, 0],
                    [0, 0, 1, 1, 0],
                    [0, 0, 0, 1, 1],
                    [0, 0, 0, 0, 1]] * state[i - 1];
    }
}

model {
    cam_top ~ uniform(cam_top_alpha, cam_top_beta);
    state_0_p_x ~ normal((cam_left + cam_right) / 2, p_x_0_stddev);
    state_0_v_x ~ normal(v_x_0_mean, v_x_0_stddev);
    state_0_p_y ~ normal((cam_top + cam_bot) / 2, p_y_0_stddev);
    state_0_v_y ~ normal(v_y_0_mean, v_y_0_stddev);
}

// Generate synthetic data
generated quantities {
    matrix[im_height, im_width] ims_sim[num_ims] = rep_array(rep_matrix(rep_vector(0.0, im_height), im_width), num_ims);
    for (i in 1:num_ims) {
        vector[2] cur_pt = [state[i, 1], state[i, 3]]';
        ims_sim[i] = draw_pt_non_homo(ims_sim[i], cur_pt, cam_matrix, 1.0);
    }
}