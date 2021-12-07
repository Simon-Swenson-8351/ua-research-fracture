import math
import typing
import enum

import numpy as np
import scipy.stats as stats

import util

import stick_sample


# Handles information regarding the probability distributions present in the Bayes network and the relationships between
# them. Also
class BayesNet:

    version = '20190703-00'

    def __init__(self, num_ims: int, im_w: int, im_h: int):
        self.num_ims = num_ims
        self.im_w = im_w
        self.im_h = im_h

        frac_tree_depth_low = 0
        frac_tree_depth_high = 4
        self.frac_tree_depth_dist = stats.randint(frac_tree_depth_low, frac_tree_depth_high)

        c_t_loc = 2.0
        c_t_scale = 10.0
        self.c_t_dist = stats.uniform(loc=c_t_loc, scale=c_t_scale)

        # s_0_v_x_in_m_s ~ Normal
        s_0_v_x_mean = 0.0
        s_0_v_x_stddev = 2.0
        self.s_0_v_x_in_m_s_dist = stats.norm(loc=s_0_v_x_mean, scale=s_0_v_x_stddev)

        # s_0_v_y_in_m_s ~ Normal
        s_0_v_y_mean = 1.0
        s_0_v_y_stddev = 2.0
        self.s_0_v_y_in_m_s_dist = stats.norm(loc=s_0_v_y_mean, scale=s_0_v_y_stddev)

        # s_0_ang ~ Uniform
        s_0_ang_loc = 0
        s_0_ang_scale = 2 * math.pi
        self.s_0_ang_dist = stats.uniform(loc=s_0_ang_loc, scale=s_0_ang_scale)

        # s_0_ang_vel_in_rad_s ~ Normal
        s_0_ang_vel_mean = 0.0
        s_0_ang_vel_stddev = 8 * math.pi
        self.s_0_ang_vel_in_rad_s_dist = stats.norm(loc=s_0_ang_vel_mean, scale=s_0_ang_vel_stddev)

        # obs_err_x ~ Normal
        obs_err_x_mean = 0.0
        obs_err_x_stddev = math.sqrt(self.im_w * self.im_h) / 64
        self.obs_err_x_dist = stats.norm(loc=obs_err_x_mean, scale=obs_err_x_stddev)

        # obs_err_y ~ Normal
        obs_err_y_mean = 0.0
        obs_err_y_stddev = math.sqrt(self.im_w * self.im_h) / 64
        self.obs_err_y_dist = stats.norm(loc=obs_err_y_mean, scale=obs_err_y_stddev)

    # s_0_p_x ~ Normal
    def s_0_p_x_dist(self, c_l: float, c_r: float) -> stats.rv_continuous:
        s_0_p_x_mean   =     (c_r + c_l) /  2
        # Normally, for forward sampling, the abs() below wouldn't be necessary. However, Metropolis-Hastings can walk
        # c_t to be lower than c_b. (In which case the camera is flipped horizontally and vertically). This causes a
        # problem if we have a negative scale.
        s_0_p_x_stddev = abs((c_r - c_l) / 16)
        return stats.norm(loc=s_0_p_x_mean, scale=s_0_p_x_stddev)

    # s_0_p_y ~ Normal
    def s_0_p_y_dist(self, c_t: float, c_b: float) -> stats.rv_continuous:
        s_0_p_y_mean   =     (c_t + c_b) /  2
        s_0_p_y_stddev = abs((c_t - c_b) / 16)
        return stats.norm(loc=s_0_p_y_mean, scale=s_0_p_y_stddev)

    # stick_len ~ Uniform
    def stick_len_dist(self, c_t: float, c_b: float) -> stats.rv_continuous:
        # stick length between 1/8 and 1/4 of the top boundary
        loc   =     (c_t - c_b) / 8
        scale = abs((c_t - c_b) / 8)
        return stats.uniform(loc=loc, scale=scale)

    def frac_loc_dist(self, stick_len: float) -> stats.rv_continuous:
        return stats.uniform(loc=0, scale=stick_len)

    def frac_time_1_dist(self, start_timestamp: int, end_timestamp: int) -> stats.rv_discrete:
        scale = round((end_timestamp - start_timestamp) / 4)
        return stats.randint(low=1, high=scale)

    # Need end_timestamp so that frac_time_1_cpd and frac_time_i_cpd have consistent signatures.
    def frac_time_i_dist(self, start_timestamp: int, end_timestamp: int) -> stats.rv_discrete:
        loc = start_timestamp + 1
        p = 0.5
        return stats.geom(loc=loc, p=p)

    def build_fracture_time_dist_list(self, num_fractures: int) -> typing.List[stats.rv_discrete]:
        if num_fractures > 0:
            result = [self.frac_time_1_dist] + [self.frac_time_i_dist] * (num_fractures - 1)
        else:
            result = []
        return result

    def new_sample_forward(self, rng: np.random.RandomState):
        result = stick_sample.StickSample(BayesNet.version, self.num_ims, self.im_w, self.im_h)

        result.frac_tree_depth = self.frac_tree_depth_dist.rvs(random_state=rng)
        result.c_t = self.c_t_dist.rvs(random_state=rng)
        result.stick_len = self.stick_len_dist(result.c_t, result.c_b).rvs(random_state=rng)

        result.s_0_p_x = self.s_0_p_x_dist(result.c_l, result.c_r).rvs(random_state=rng)
        result.s_0_v_x_in_m_s = self.s_0_v_x_in_m_s_dist.rvs(random_state=rng)
        result.s_0_p_y = self.s_0_p_y_dist(result.c_t, result.c_b).rvs(random_state=rng)
        result.s_0_v_y_in_m_s = self.s_0_v_y_in_m_s_dist.rvs(random_state=rng)
        result.s_0_ang = self.s_0_ang_dist.rvs(random_state=rng)
        result.s_0_ang_vel_in_rad_s = self.s_0_ang_vel_in_rad_s_dist.rvs(random_state=rng)

        if result.frac_tree_depth > 0:
            result.fracture_sample = stick_sample.StickSample.FractureSample.from_distributions(
                self.build_fracture_time_dist_list(result.frac_tree_depth),
                self.frac_loc_dist,
                rng,
                0,
                result.num_ims,
                result.stick_len,
                result.frac_tree_depth
            )

        result.calc_stick_states()
        result.observed_endpoints = [None] * result.num_ims

        c_mat = result.c_mat
        for im_idx in range(result.num_ims):
            stick_pts = result.stick_states.as_lines(im_idx)
            old_shape = stick_pts.shape
            stick_pts = np.reshape(stick_pts, (old_shape[0] * old_shape[1], old_shape[2]))
            stick_pxs = util.apply_homogeneous_transformation(
                c_mat,
                stick_pts
            )
            # Add the camera jiggle
            result.observed_endpoints[im_idx] = np.copy(stick_pxs)
            result.observed_endpoints[im_idx][:, 0] += self.obs_err_x_dist.rvs(
                (result.observed_endpoints[im_idx].shape[0],),
                random_state=rng
            )
            result.observed_endpoints[im_idx][:, 1] += self.obs_err_y_dist.rvs(
                (result.observed_endpoints[im_idx].shape[0],),
                random_state=rng
            )
            result.observed_endpoints[im_idx] = np.reshape(
                result.observed_endpoints[im_idx],
                old_shape
            )

        return result

    # Here, we assume sample.stick_states is not stale. If you are unsure it is stale, call
    # sample.calc_stick_states() before calling this function. (Warning: This is a lot of computation, so make sure
    def calc_and_set_log_prob(self, sample) -> float:
        sample.log_prob = 0.0
        # Just add the log prob of every sampled node
        sample.log_prob += self.frac_tree_depth_dist.logpmf(sample.frac_tree_depth)
        sample.log_prob += self.c_t_dist.logpdf(sample.c_t)
        sample.log_prob += self.stick_len_dist(sample.c_t, sample.c_b).logpdf(sample.stick_len)
        sample.log_prob += self.s_0_p_x_dist(sample.c_l, sample.c_r).logpdf(sample.s_0_p_x)
        sample.log_prob += self.s_0_v_x_in_m_s_dist.logpdf(sample.s_0_v_x_in_m_s)
        sample.log_prob += self.s_0_p_y_dist(sample.c_t, sample.c_b).logpdf(sample.s_0_p_y)
        sample.log_prob += self.s_0_v_y_in_m_s_dist.logpdf(sample.s_0_v_y_in_m_s)
        sample.log_prob += self.s_0_ang_dist.logpdf(sample.s_0_ang)
        sample.log_prob += self.s_0_ang_vel_in_rad_s_dist.logpdf(sample.s_0_ang_vel_in_rad_s)
        sample.log_prob += self.__calc_fracture_sample_log_prob(
            sample.stick_len,
            0,
            sample.num_ims,
            sample.fracture_sample,
            self.build_fracture_time_dist_list(sample.frac_tree_depth)
        )
        sample.log_prob += self.__calc_observed_endpoint_log_prob(sample)

    def __calc_fracture_sample_log_prob(
            self,
            stick_len,
            start_time,
            end_time,
            fracture_sample,
            fracture_time_dist_list
    ):
        if fracture_sample is None:
            return 0.0
        result = 0.0
        result += fracture_time_dist_list[0](start_time, end_time).logpmf(fracture_sample.fr_time)
        result += self.frac_loc_dist(stick_len).logpdf(fracture_sample.fr_loc)
        result += self.__calc_fracture_sample_log_prob(
            fracture_sample.fr_loc,
            fracture_sample.fr_time,
            end_time,
            fracture_sample.childl,
            fracture_time_dist_list[1:]
        )
        result += self.__calc_fracture_sample_log_prob(
            stick_len - fracture_sample.fr_loc,
            fracture_sample.fr_time,
            end_time,
            fracture_sample.childr,
            fracture_time_dist_list[1:]
        )
        return result

    def __calc_observed_endpoint_log_prob(self, sample):
        result = 0.0
        c_mat = sample.c_mat
        for im_idx in range(sample.num_ims):
            cur_observed_endpoints = sample.observed_endpoints[im_idx]
            cur_observed_endpoints = np.reshape(
                cur_observed_endpoints,
                (cur_observed_endpoints.shape[0] * cur_observed_endpoints.shape[1], cur_observed_endpoints.shape[2])
            )
            stick_pts = sample.stick_states.as_lines(im_idx)
            # Reshape to one point per row
            stick_pts = np.reshape(
                stick_pts,
                cur_observed_endpoints.shape
            )
            stick_pxs = util.apply_homogeneous_transformation(
                c_mat,
                stick_pts
            )
            # In the current forward sampling implementation, these values are added to the actual endpoint positions,
            # so we need to subtract them off here.
            result += np.sum(self.obs_err_x_dist.logpdf(cur_observed_endpoints[:, 0] - stick_pxs[:, 0]))
            result += np.sum(self.obs_err_y_dist.logpdf(cur_observed_endpoints[:, 1] - stick_pxs[:, 1]))
        return result

