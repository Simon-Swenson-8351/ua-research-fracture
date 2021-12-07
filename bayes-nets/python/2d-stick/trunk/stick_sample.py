import math

import numpy as np

import util


# Represents a setting of the random variables in a Bayes Net to particular values
# I wanted a clear delineation between the structure of the Bayes Net (distributions)
# and the actual samples drawn from that Bayes Net
class StickSample:

    # Represents the samples for a fracture tree (fracture location and time)
    # Since this is a tree structure (with a variable depth), I have broken it out into a separate
    # class from StickSample.
    class FractureSample:

        def __init__(self, fr_time, fr_loc):
            self.fr_time = fr_time
            self.fr_loc = fr_loc
            self.childl = None
            self.childr = None

        @classmethod
        def from_dict(cls, dictionary):
            result = cls(dictionary['fr_time'], dictionary['fr_loc'])
            if dictionary.get('childl'):
                result.childl = cls.from_dict(dictionary['childl'])
            if dictionary.get('childr'):
                result.childr = cls.from_dict(dictionary['childr'])
            return result

        @classmethod
        def from_distributions(
                cls,
                fr_time_cpd_list,
                fr_loc_cpd,
                rng,
                start_timestamp,
                subtree_end_timestamp,
                stick_len,
                fr_tree_depth
        ):
            if fr_tree_depth == 0:
                return None
            else:
                fr_time = fr_time_cpd_list[0](start_timestamp, subtree_end_timestamp).rvs(random_state=rng)
                fr_loc = fr_loc_cpd(stick_len).rvs(random_state=rng)
                result = cls(fr_time, fr_loc)
                result.childl = cls.from_distributions(
                    fr_time_cpd_list[1:],
                    fr_loc_cpd,
                    rng,
                    fr_time,
                    subtree_end_timestamp,
                    fr_loc,
                    fr_tree_depth - 1
                )
                result.childr = cls.from_distributions(
                    fr_time_cpd_list[1:],
                    fr_loc_cpd,
                    rng,
                    fr_time,
                    subtree_end_timestamp,
                    stick_len - fr_loc,
                    fr_tree_depth - 1

                )
                return result

        def to_dict(self):
            result = self.__dict__.copy()
            if self.childl is not None:
                result['childl'] = self.childl.to_dict()
            else:
                result['childl'] = None
            if self.childr is not None:
                result['childr'] = self.childr.to_dict()
            else:
                result['childr'] = None
            return result
    # END class FractureSample

    # This is a helper class that is able to simulate a stick's travel throughout the scene, given just the initial
    # state of the stick and the fracture event samples (2d_stick_sample.SampleSet.FractureSample).
    # You can think of this as part of a sample. It really represents a bunch of deterministic nodes within the
    # Bayes Net.
    class StickStates:

        transition_matrix = np.array([
            [1, 1, 0, 0, 0, 0, 0],
            [0, 1, 0, 0, 0, 0, 0],
            [0, 0, 1, 1, 0, 0, 0],
            [0, 0, 0, 1, 1, 0, 0],
            [0, 0, 0, 0, 1, 0, 0],
            [0, 0, 0, 0, 0, 1, 1],
            [0, 0, 0, 0, 0, 0, 1],
        ])

        def __init__(
                self,
                start_timestamp: int,
                sequence_end_timestamp: int,
                stick_len: float,
                frac_sample_set
        ):
            self.start_timestamp = start_timestamp
            self.length = stick_len
            self.__geometry = None
            if frac_sample_set is None:
                self.end_timestamp = sequence_end_timestamp
                self.childl = None
                self.childr = None
            else:
                self.end_timestamp = frac_sample_set.fr_time
                self.childl = StickSample.StickStates(
                    self.end_timestamp,
                    sequence_end_timestamp,
                    frac_sample_set.fr_loc,
                    frac_sample_set.childl
                )
                self.childr = StickSample.StickStates(
                    self.end_timestamp,
                    sequence_end_timestamp,
                    self.length - frac_sample_set.fr_loc,
                    frac_sample_set.childr
                )
            self.states = np.zeros((self.end_timestamp - self.start_timestamp, 7))

        def is_active(self, timestamp: int) -> bool:
            return self.start_timestamp <= timestamp < self.end_timestamp

        # This should only be called on the root node, or undefined behavior will occur.
        def set_and_propagate_initial_state(self, initial_state_vector: np.array, sequence_end_timestamp):
            self.__set_initial_state(initial_state_vector)
            for i in range(1, sequence_end_timestamp):
                self.__update(i)

        def __set_initial_state(self, initial_state_vector: np.array):
            self.states[0] = initial_state_vector

        def __update(self, timestamp: int):
            rel_timestamp = timestamp - self.start_timestamp
            if self.is_active(timestamp):
                self.states[rel_timestamp] = StickSample.StickStates.transition_matrix @ self.states[rel_timestamp - 1]
            elif timestamp == self.end_timestamp:
                # Time to perform the split
                rel_timestamp = timestamp - self.start_timestamp
                cur_parent_state = self.states[rel_timestamp - 1]

                frag1_offset = 1 / 2 * (self.childl.length - self.length)
                frag1_s0 = np.copy(cur_parent_state)
                frag1_s0[0] += frag1_offset * math.cos(cur_parent_state[5])
                frag1_s0[1] += frag1_offset * (
                            math.cos(cur_parent_state[5] + cur_parent_state[6]) - math.cos(cur_parent_state[5]))
                frag1_s0[2] += frag1_offset * math.sin(cur_parent_state[5])
                frag1_s0[3] += frag1_offset * (
                            math.sin(cur_parent_state[5] + cur_parent_state[6]) - math.sin(cur_parent_state[5]))
                frag1_s0 = StickSample.StickStates.transition_matrix @ frag1_s0
                self.childl.__set_initial_state(frag1_s0)

                frag2_offset = 1 / 2 * (self.length - self.childr.length)
                frag2_s0 = np.copy(cur_parent_state)
                frag2_s0[0] += frag2_offset * math.cos(cur_parent_state[5])
                frag2_s0[1] += frag2_offset * (
                            math.cos(cur_parent_state[5] + cur_parent_state[6]) - math.cos(cur_parent_state[5]))
                frag2_s0[2] += frag2_offset * math.sin(cur_parent_state[5])
                frag2_s0[3] += frag2_offset * (
                            math.sin(cur_parent_state[5] + cur_parent_state[6]) - math.sin(cur_parent_state[5]))
                frag2_s0 = StickSample.StickStates.transition_matrix @ frag2_s0
                self.childr.__set_initial_state(frag2_s0)
            else:
                self.childl.__update(timestamp)
                self.childr.__update(timestamp)

        # returns an n x 2 x 2 ndarray, where result[i] represents a pair of points denoting the each line in world space.
        # Indexing is: line index first, then point index (line endpoint), then x, y.
        def as_lines(self, timestamp: int) -> np.array:
            return np.array(self.__as_lines_helper(timestamp))

        # Returns a n-entry list
        def __as_lines_helper(self, timestamp: int) -> list:
            result = []
            if self.start_timestamp <= timestamp < self.end_timestamp:
                cur_transform = self.get_stick_transformation(timestamp)
                result += [util.apply_homogeneous_transformation(cur_transform, self.geometry).tolist()]
            # If I exist, my children should not exist
            elif self.childl is not None and self.childr is not None:
                result += self.childl.__as_lines_helper(timestamp)
                result += self.childr.__as_lines_helper(timestamp)
            return result

        def get_stick_transformation(self, timestamp: int) -> np.array:
            if self.start_timestamp <= timestamp < self.end_timestamp \
                    and self.states is not None:
                state = self.states[timestamp - self.start_timestamp]
                return np.array([
                    [math.cos(state[5]), -math.sin(state[5]), state[0]],
                    [math.sin(state[5]), math.cos(state[5]), state[2]],
                    [0, 0, 1]
                ])
            else:
                return None

        @property
        def geometry(self):
            if self.__geometry is not None:
                return self.__geometry
            else:
                half = self.length / 2
                return np.array([
                    [-half, 0],
                    [half, 0]
                ])
    #  END class StickStates

    def __init__(self, version, num_ims, im_w, im_h):
        self.version = version

        # variables from cmd line arguments
        self.num_ims = num_ims
        self.im_w = im_w
        self.im_h = im_h

        # fixed nodes
        self.c_l = 0.0
        self.c_b = 0.0
        self.c_fps = 30.0
        self.gravity = -9.8
        self.s_0_a_y = self.gravity / self.c_fps ** 2

        # randomly-sampled variables
        self.frac_tree_depth = None
        self.c_t = None
        self.stick_len = None
        self.s_0_p_x = None
        self.s_0_v_x_in_m_s = None
        self.s_0_p_y = None
        self.s_0_v_y_in_m_s = None
        self.s_0_ang = None
        self.s_0_ang_vel_in_rad_s = None
        # type expected to be a list of numpy arrays.
        # list needed, since the number of stick endpoints is variable between frames.
        self.observed_endpoints = []
        # type expected to be FractureSampleSet
        self.fracture_sample = None

        # An instance of StickInfo, representing all calculated states of all sticks as they fly around.
        self.stick_states = None

        # Used for MH sampler
        self.log_prob = None

    @classmethod
    def from_dict(cls, version, dict_in):
        if dict_in['version'] != version:
            raise Exception('Wrong version. Expected {}, got {}'.format(version, dict_in['version']))
        result = cls(version, dict_in['num_ims'], dict_in['im_w'], dict_in['im_h'])
        result.__dict__.update(dict_in)
        for i in range(len(result.observed_endpoints)):
            result.observed_endpoints[i] = np.array(result.observed_endpoints[i])
        if dict_in.get('fracture_sample') is not None:
            result.fracture_sample = StickSample.FractureSample.from_dict(
                dict_in['fracture_sample']
            )
        else:
            result.fracture_sample = None
        result.calc_stick_states()
        return result

    def to_dict(self):
        result = self.__dict__.copy()
        for i in range(len(result['observed_endpoints'])):
            result['observed_endpoints'][i] = result['observed_endpoints'][i].tolist()
        if result.get('fracture_sample') is not None:
            result['fracture_sample'] = result['fracture_sample'].to_dict()
        result['stick_states'] = None
        return result

    # calculated variables. We'll use properties for these to make them appear as normal
    # members.
    @property
    def px_per_unit(self):
        return self.im_h / (self.c_t - self.c_b)

    @property
    def unit_per_px(self):
        return (self.c_t - self.c_b) / self.im_h

    @property
    def c_r(self) -> float:
        return self.c_l + self.im_w * ((self.c_t - self.c_b) / self.im_h)

    @property
    def c_mat(self) -> np.array:
        # px_per_unit is calculated, so store it locally once.
        ppu = self.px_per_unit
        return np.array([
            [  0, -ppu,  self.c_t * ppu],
            [ppu,    0, -self.c_l * ppu],
            [  0,    0,               1]
        ])

    @property
    def s_0_v_x(self):
        return self.s_0_v_x_in_m_s / self.c_fps

    @property
    def s_0_v_y(self):
        return self.s_0_v_y_in_m_s / self.c_fps

    @property
    def s_0_ang_vel(self):
        return self.s_0_ang_vel_in_rad_s / self.c_fps

    def calc_stick_states(self):
        self.stick_states = StickSample.StickStates(
            0,
            self.num_ims,
            self.stick_len,
            self.fracture_sample
        )
        self.stick_states.set_and_propagate_initial_state(
            np.array([
                self.s_0_p_x,
                self.s_0_v_x,
                self.s_0_p_y,
                self.s_0_v_y,
                self.s_0_a_y,
                self.s_0_ang,
                self.s_0_ang_vel
            ]),
            self.num_ims
        )
