import json
import numpy as np
import scipy.stats as stats
import math
import sys

sys.path.append('../lib')
import util
import discretization
import discrete_prob

if __name__ == '__main__':
    num_samples = 10

    # Fixed nodes
    im_w = 640
    im_h = 480
    num_ims = 30

    # All physical units are in meters.
    c_l = 0.0
    c_b = 0.0
    # c_t ~ Uniform
    c_t_loc = 5.0
    c_t_scale = 45.0
    c_t_cpd = DiscreteCPD.from_DiscretizedFunction( \
        discretization.discretize_1d_fn( \
            stats.uniform(loc, scale).pdf,
            128,
            5.0,
            50.0 \
        ) \
    )

    c_fps = 60.0

    # initial state of the center of mass

    # s_0_v ~ Normal
    s_0_v_mean = np.array([0.0, 0.0])
    s_0_v_stddev = np.array([[1.0, 0.0], [0.0, 1.0]])

    s_0_angle_alpha = 0
    s_0_angle_beta = 2 * math.pi

    s_0_angle_pdf = stats.uniform(alpha, beta - alpha).pdf
    s_0_angle_pmf = discretization.discretize(s_0_angle_pdf, )

    s_0_len_base = 2.0
    # Should we use a bounded distribution to ensure we can't have negative 
    # length?
    # TODO is Beta a reasonable choice?
    # s_0_len_unscaled_offset ~ Beta
    s_0_len_unscaled_offset_alpha = 2.0
    s_0_len_unscaled_offset_beta = 2.0

    s_0_len_offset_scaling_factor = 2.0
    s_0_len_offset = lambda unscaled_offset: s_0_len_offset_scaling_factor * unscaled_offset

    s_0_len = lambda scaled_offset: s_0_len_base + scaled_offset

    s_0_ang_vel_mean = 0.0
    s_0_ang_vel_stddev = math.pi / 4

    cam_pixel_intensity_mean = 0.0
    cam_pixel_intensity_stddev = 0.1

    gravity = -9.8

    transition = np.array([
        [1, 1, 0, 0, 0],
        [0, 1, 0, 0, 0],
        [0, 0, 1, 1, 0],
        [0, 0, 0, 1, 1],
        [0, 0, 0, 0, 1]])