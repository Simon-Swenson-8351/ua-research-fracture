import copy
import math

import scipy.stats as stats

import stick_sample


class StickMHSampler:

    # observation is a SampleSet
    def __init__(
            self,
            observation,
            num_burn_in_samples: int,
            num_saved_samples: int,
            num_chains
    ):
        self.observation = observation
        self.prev_sample = None
        self.saved_samples = [None] * num_saved_samples
        self.best_sample = None
        self.num_burn_in_samples = num_burn_in_samples
        self.num_saved_samples = num_saved_samples
        self.num_chains = num_chains
        self.cur_iter = 0

    # Just use forward sampling in this case. We overwrite some of the observation's nodes to make sure we don't know
    # *everything*
    def _new_sample_mh_from_observation(self, rng):
        result = copy.deepcopy(self.observation)

        result.c_t = self.bn.c_t_dist.rvs(random_state=rng)

        result.s_0_p_x = self.bn.s_0_p_x_dist(
            result.c_l,
            result.c_r
        ).rvs(random_state=rng)

        result.s_0_p_y = self.bn.s_0_p_y_dist(
            result.c_t,
            result.c_b
        ).rvs(random_state=rng)

        result.s_0_v_x_in_m_s = stick_sample.StickSample.s_0_v_x_in_m_s_dist.rvs(random_state=rng)
        result.s_0_v_y_in_m_s = stick_sample.StickSample.s_0_v_y_in_m_s_dist.rvs(random_state=rng)
        result.s_0_ang = stick_sample.StickSample.s_0_ang_dist.rvs(random_state=rng)
        result.s_0_ang_vel_in_rad_s = stick_sample.StickSample.s_0_ang_vel_in_rad_s_dist.rvs(random_state=rng)

        return result

    # A factory for producing a new StickSample from an old StickSample, using the MH algorithm to
    # both (1) propose the new variable assignment and (2) accept/reject the new sample. If the sample is rejected, the
    # value returned will be a deep copy of old_sample_set.
    def _new_sample_mh_from_old_sample(self, rng):
        result = copy.deepcopy(self.prev_sample)

        result.c_t = self.c_t_proposal_cpd(
            self.prev_sample.c_t).rvs(random_state=rng)
        result.s_0_p_x = self.s_0_p_x_proposal_cpd(
            self.prev_sample.s_0_p_x).rvs(random_state=rng)
        result.s_0_p_y = self.s_0_p_y_proposal_cpd(
            self.prev_sample.s_0_p_y).rvs(random_state=rng)
        result.s_0_v_x_in_m_s = self.s_0_v_x_in_m_s_proposal_cpd(
            self.prev_sample.s_0_v_x_in_m_s).rvs(random_state=rng)
        result.s_0_v_y_in_m_s = self.s_0_v_x_in_m_s_proposal_cpd(
            self.prev_sample.s_0_v_y_in_m_s).rvs(random_state=rng)
        result.s_0_ang = self.s_0_ang_proposal_cpd(
            self.prev_sample.s_0_ang).rvs(random_state=rng)
        result.s_0_ang_vel_in_rad_s = self.s_0_ang_vel_in_rad_s_proposal_cpd(
            self.prev_sample.s_0_ang_vel_in_rad_s).rvs(random_state=rng)

        log_diff = result.continuous_log_prob - self.prev_sample.continuous_log_prob
        if math.log(rng.uniform()) > log_diff:
            # Reset the values to what they were previously (avoid another deep copy)
            result.c_t = self.prev_sample.c_t
            result.s_0_p_x = self.prev_sample.s_0_p_x
            result.s_0_p_y = self.prev_sample.s_0_p_y
            result.continuous_log_prob = self.prev_sample.continuous_log_prob
            # I'm assuming a deep copy here is more efficient than re-simulating the sticks.
            result.stick_states = copy.deepcopy(self.prev_sample.stick_states)

        return result

    def run_all(self, rng):
        for i in range(self.cur_iter, self.num_burn_in_samples + self.num_saved_samples):
            self.step(rng)

    def step(self, rng):
        if self.prev_sample is None:
            cur_sample = self._new_sample_mh_from_observation(rng)
        else:
            cur_sample = self._new_sample_mh_from_old_sample(rng)

        if self.cur_iter >= self.num_burn_in_samples:
            self.saved_samples[self.cur_iter - self.num_burn_in_samples] = cur_sample
            if self.best_sample is None:
                self.best_sample = cur_sample
            elif self.best_sample.continuous_log_prob < cur_sample.continuous_log_prob:
                self.best_sample = cur_sample

        self.prev_sample = cur_sample
        self.cur_iter += 1

    def c_t_proposal_cpd(self, c_t_prev):
        # A good value for this stddev will depend on whatever c_t_dist is in StickBayesNet.
        stddev = 2.0
        return stats.norm(loc=c_t_prev, scale=stddev)

    def s_0_p_x_proposal_cpd(self, s_0_p_x_prev):
        stddev = 1.0
        return stats.norm(loc=s_0_p_x_prev, scale=stddev)

    def s_0_p_y_proposal_cpd(self, s_0_p_y_prev):
        stddev = 1.0
        return stats.norm(loc=s_0_p_y_prev, scale=stddev)

    def s_0_v_x_in_m_s_proposal_cpd(self, s_0_v_x_in_m_s_prev):
        stddev = 0.5
        return stats.norm(loc=s_0_v_x_in_m_s_prev, scale=stddev)

    def s_0_v_y_in_m_s_proposal_cpd(self, s_0_v_y_in_m_s_prev):
        stddev = 0.5
        return stats.norm(loc=s_0_v_y_in_m_s_prev, scale=stddev)

    def s_0_ang_proposal_cpd(self, s_0_ang_prev):
        stddev = 0.1
        return stats.norm(loc=s_0_ang_prev, scale=stddev)

    def s_0_ang_vel_in_rad_s_proposal_cpd(self, s_0_ang_vel_in_rad_s_prev):
        stddev = 0.1
        return stats.norm(loc=s_0_ang_vel_in_rad_s_prev, scale=stddev)
