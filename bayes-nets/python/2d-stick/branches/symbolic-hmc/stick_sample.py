import math

import numpy as np
import scipy.stats as stats
import sympy as sp
import sympy.utilities.lambdify

import util

import symbolic_prob


def state_p_x_idx():
    return 0


def state_v_x_idx():
    return 1


def state_p_y_idx():
    return 2


def state_v_y_idx():
    return 3


def state_a_y_idx():
    return 4


def state_ang_idx():
    return 5


def state_ang_vel_idx():
    return 6


def state_trans_matrix_expr():
    l = [[0] * 7 for i in range(7)]

    p_x = state_p_x_idx()
    v_x = state_v_x_idx()
    p_y = state_p_y_idx()
    v_y = state_v_y_idx()
    a_y = state_a_y_idx()
    ang = state_ang_idx()
    av = state_ang_vel_idx()

    l[p_x][p_x] = 1
    l[p_x][v_x] = 1

    l[v_x][v_x] = 1

    l[p_y][p_y] = 1
    l[p_y][v_y] = 1

    l[v_y][v_y] = 1
    l[v_y][a_y] = 1

    l[a_y][a_y] = 1

    l[ang][ang] = 1
    l[ang][av] = 1

    l[av][av] = 1

    return sp.Matrix(l)


def state_trans_matrix_from_sym():
    return np.array(state_trans_matrix_expr()).astype(np.float64)


# Handles information regarding the probability distributions present in the Bayes network and the relationships between
# them.
class StickSample:

    version = '20190710-00'

    # We'll make all the constants class variables.
    num_ims_expr = sp.symbols('num_ims')
    im_w_expr = sp.symbols('im_w')
    im_h_expr = sp.symbols('im_h')
    c_l = sp.RealNumber(0)
    c_b = sp.RealNumber(0)
    c_fps = sp.RealNumber(30)
    gravity = sp.Rational(-98, 10)
    s_0_a_y = gravity / c_fps ** 2

    frac_tree_depth_low = sp.Integer(0)
    frac_tree_depth_high = sp.Integer(3)
    frac_tree_depth_expr = sp.symbols('frac_tree_depth')
    frac_tree_depth_pmf_expr = symbolic_prob.uniform_pmf(
        frac_tree_depth_expr,
        frac_tree_depth_low,
        frac_tree_depth_high
    )
    # Have to add 1 to high due to an inconsistency in how scipy.stats defines the range vs my code. Probably I
    # should make them consistent at some point. TODO
    frac_tree_depth_pmf = stats.randint(low=frac_tree_depth_low, high=frac_tree_depth_high + 1)

    # maybe it would follow a Gaussian in practice. These uniform distributions are really bad for HMC.
    # c_t ~ normal
    c_t_mean = sp.RealNumber(8)
    c_t_std  = sp.RealNumber(2)
    c_t_expr = sp.symbols('c_t')
    c_t_pdf_expr = symbolic_prob.norm_pdf(
        c_t_expr,
        c_t_mean,
        c_t_std
    )
    c_t_pdf = stats.norm(
        loc=c_t_mean,
        scale=c_t_std
    )

    # To deal with unit conversion
    # We can use this expression for both horizontal and vertical unit conversions as long as neither axis is squished,
    # which is a realistic assumption
    meters_px_expr = (c_t_expr - c_b) / im_h_expr

    # Just base c_r off of the aspect ratio of the image and whatever c_t happens to be.
    c_r_expr = c_l + im_w_expr * meters_px_expr

    c_h_expr = c_t_expr - c_b
    c_w_expr = c_r_expr - c_l

    c_mat_expr = sp.Matrix([
        [                 0, -1 / meters_px_expr, c_t_expr / meters_px_expr],
        [1 / meters_px_expr,                   0,     -c_l / meters_px_expr],
        [                 0,                   0,                         1]
    ])

    # stick_len ~ normal
    # Probably, the object that's being fractured will be entirely in frame.
    stick_len_mean_expr = (c_t_expr - c_b) / 4
    stick_len_std_expr  = (c_t_expr - c_b) / 8
    stick_len_expr = sp.symbols('stick_len')
    stick_len_pdf_expr = symbolic_prob.norm_pdf(
        stick_len_expr,
        stick_len_mean_expr,
        stick_len_std_expr
    )

    # s_0_p_x ~ normal
    # Stick should be somewhere near the center
    s_0_p_x_mean_expr = (c_r_expr + c_l) /  2
    s_0_p_x_std_expr  = (c_r_expr - c_l) / 16
    s_0_p_x_expr = sp.symbols('s_0_p_x')
    s_0_p_x_pdf_expr = symbolic_prob.norm_pdf(
        s_0_p_x_expr,
        s_0_p_x_mean_expr,
        s_0_p_x_std_expr
    )

    # s_0_p_y ~ normal
    s_0_p_y_mean_expr = (c_t_expr + c_b) /  2
    s_0_p_y_std_expr  = (c_t_expr - c_b) / 16
    s_0_p_y_expr = sp.symbols('s_0_p_y')
    s_0_p_y_pdf_expr = symbolic_prob.norm_pdf(
        s_0_p_y_expr,
        s_0_p_y_mean_expr,
        s_0_p_y_std_expr
    )

    # s_0_v_x_in_m_s ~ normal
    s_0_v_x_in_m_s_mean = sp.RealNumber(0)
    s_0_v_x_in_m_s_std = sp.RealNumber(2)
    s_0_v_x_in_m_s_expr = sp.symbols('s_0_v_x_in_m_s')
    s_0_v_x_in_m_s_pdf_expr = symbolic_prob.norm_pdf(
        s_0_v_x_in_m_s_expr,
        s_0_v_x_in_m_s_mean,
        s_0_v_x_in_m_s_std
    )
    s_0_v_x_in_m_s_pdf = stats.norm(loc=s_0_v_x_in_m_s_mean, scale=s_0_v_x_in_m_s_std)

    # convert from m/s to m/frame
    s_0_v_x_expr = s_0_v_x_in_m_s_expr / c_fps

    # s_0_v_x ~ normal
    # s_0_v_y_in_m_s ~ Normal
    s_0_v_y_in_m_s_mean = sp.RealNumber(1)
    s_0_v_y_in_m_s_std = sp.RealNumber(2)
    s_0_v_y_in_m_s_expr = sp.symbols('s_0_v_y_in_m_s')
    s_0_v_y_in_m_s_pdf_expr = symbolic_prob.norm_pdf(
        s_0_v_y_in_m_s_expr,
        s_0_v_y_in_m_s_mean,
        s_0_v_y_in_m_s_std
    )
    s_0_v_y_in_m_s_pdf = stats.norm(loc=s_0_v_y_in_m_s_mean, scale=s_0_v_y_in_m_s_std)

    s_0_v_y_expr = s_0_v_y_in_m_s_expr / c_fps

    # s_0_ang ~ normal
    s_0_ang_mean = sp.RealNumber(0)
    s_0_ang_std = sp.pi / 2
    s_0_ang_expr = sp.symbols('s_0_ang')
    s_0_ang_pdf_expr = symbolic_prob.norm_pdf(
        s_0_ang_expr,
        s_0_ang_mean,
        s_0_ang_std
    )
    s_0_ang_pdf = stats.norm(loc=s_0_ang_mean, scale=float(s_0_ang_std))

    # s_0_ang_vel_in_rad_s ~ normal
    s_0_ang_vel_in_rad_s_mean = sp.RealNumber(0)
    s_0_ang_vel_in_rad_s_stddev = 8 * sp.pi
    s_0_ang_vel_in_rad_s_expr = sp.symbols('s_0_ang_vel_in_rad_s')
    s_0_ang_vel_in_rad_s_pdf_expr = symbolic_prob.norm_pdf(
        s_0_ang_vel_in_rad_s_expr,
        s_0_ang_vel_in_rad_s_mean,
        s_0_ang_vel_in_rad_s_stddev
    )
    s_0_ang_vel_in_rad_s_pdf = stats.norm(loc=s_0_ang_vel_in_rad_s_mean, scale=float(s_0_ang_vel_in_rad_s_stddev))

    s_0_ang_vel_expr = s_0_ang_vel_in_rad_s_expr / c_fps

    # The BayesNet class represents the structure of the directed graph and the distributions that are to be drawn from,
    # and instances of BayesNet represent actual samples.
    #
    # Here, we use the constructor when doing forward sampling from the prior. However, we will use copies of existing
    # instances when running inference.
    def __init__(self, num_ims: int, im_w: int, im_h: int, rng):
        self.num_ims = int(StickSample.num_ims_expr.subs(StickSample.num_ims_expr, num_ims))
        self.im_w = int(StickSample.im_w_expr.subs(StickSample.im_w_expr, im_w))
        self.im_h = int(StickSample.im_h_expr.subs(StickSample.im_h_expr, im_h))

        self.frac_tree_depth = StickSample.frac_tree_depth_pmf.rvs(random_state=rng)

        self.c_t = StickSample.c_t_pdf.rvs(random_state=rng)

        self.stick_len_mean = float(StickSample.stick_len_mean_expr.subs(StickSample.c_t_expr, self.c_t))
        self.stick_len_std = float(StickSample.stick_len_std_expr.subs(StickSample.c_t_expr, self.c_t))
        self.stick_len_pdf = stats.norm(loc=self.stick_len_mean, scale=self.stick_len_std)
        self.stick_len = self.stick_len_pdf.rvs(random_state=rng)

        self.s_0_p_x_mean = float(
            StickSample.s_0_p_x_mean_expr
            .subs(StickSample.c_t_expr, self.c_t)
            .subs(StickSample.im_h_expr, self.im_h)
            .subs(StickSample.im_w_expr, self.im_w)
        )
        self.s_0_p_x_stddev = float(
            StickSample.s_0_p_x_std_expr
            .subs(StickSample.c_t_expr, self.c_t)
            .subs(StickSample.im_h_expr, self.im_h)
            .subs(StickSample.im_w_expr, self.im_w)
        )
        self.s_0_p_x_pdf = stats.norm(loc=self.s_0_p_x_mean, scale=self.s_0_p_x_stddev)
        self.s_0_p_x = self.s_0_p_x_pdf.rvs(random_state=rng)

        self.s_0_p_y_mean = float(StickSample.s_0_p_y_mean_expr.subs(StickSample.c_t_expr, self.c_t))
        self.s_0_p_y_stddev = float(StickSample.s_0_p_y_std_expr.subs(StickSample.c_t_expr, self.c_t))
        self.s_0_p_y_pdf = stats.norm(loc=self.s_0_p_y_mean, scale=self.s_0_p_y_stddev)
        self.s_0_p_y = self.s_0_p_y_pdf.rvs(random_state=rng)

        self.s_0_v_x_in_m_s = StickSample.s_0_v_x_in_m_s_pdf.rvs(random_state=rng)

        self.s_0_v_y_in_m_s = StickSample.s_0_v_y_in_m_s_pdf.rvs(random_state=rng)

        self.s_0_ang = StickSample.s_0_ang_pdf.rvs(random_state=rng)

        self.s_0_ang_vel_in_rad_s = self.s_0_ang_vel_in_rad_s_pdf.rvs(random_state=rng)

        # We can put the scipy.stats objects in here, since we're using offsets to do things numerically with samples.
        # However, when we do things symbolically with PDFs, it's not that simple, since each PDF technically has a
        # unique mean.
        self.world_endpoints_obs_x_stddev = float(
            SymbolicStickStates.world_endpoints_obs_x_std_expr
            .subs(StickSample.c_t_expr, self.c_t)
            .subs(StickSample.im_h_expr, self.im_h)
            .subs(StickSample.im_w_expr, self.im_w)
        )
        self.world_endpoints_obs_x_pdf = stats.norm(scale=self.world_endpoints_obs_x_stddev)
        self.world_endpoints_obs_y_stddev = float(
            SymbolicStickStates.world_endpoints_obs_y_std_expr
            .subs(StickSample.c_t_expr, self.c_t)
            .subs(StickSample.im_h_expr, self.im_h)
            .subs(StickSample.im_w_expr, self.im_w)
        )
        self.world_endpoints_obs_y_pdf = stats.norm(scale=self.world_endpoints_obs_y_stddev)

        # The actual structure (and thus symbolic representation) of the fracture tree depends on the frac_tree_depth
        # sampled above.
        # All variables related to fracture and stick states are handled by different components of this tree.
        self.stick_tree = StickTree(self.frac_tree_depth)
        self.stick_tree.populate_symbolic_frac_info(StickSample.stick_len_expr)
        self.stick_tree.populate_sampled_frac_info(self.num_ims, self.stick_len, rng)
        init_state_vec_expr = sp.Matrix([
            [StickSample.s_0_p_x_expr],
            [StickSample.s_0_v_x_expr],
            [StickSample.s_0_p_y_expr],
            [StickSample.s_0_v_y_expr],
            [StickSample.s_0_a_y],
            [StickSample.s_0_ang_expr],
            [StickSample.s_0_ang_vel_expr],
        ])
        self.stick_tree.populate_symbolic_stick_states(
            num_ims,
            StickSample.stick_len_expr,
            init_state_vec_expr
        )
        self.stick_tree.populate_sampled_stick_states(
            self.c_mat,
            self.s_0_p_x,
            self.s_0_v_x_in_m_s,
            self.s_0_p_y,
            self.s_0_v_y_in_m_s,
            self.s_0_ang,
            self.s_0_ang_vel_in_rad_s,
            self.stick_len,
            self.world_endpoints_obs_x_pdf,
            self.world_endpoints_obs_y_pdf,
            rng,
        )

        self._symbolic_continuous_joint = None
        self._symbolic_continuous_log_joint = None
        self._continuous_log_prob = None

    @property
    def meters_px(self):
        return float(
            StickSample.meters_px_expr
            .subs(StickSample.im_h_expr, self.im_h)
            .subs(StickSample.c_t_expr, self.c_t)
        )

    @property
    def c_r(self):
        return float(
            StickSample.c_r_expr
            .subs(StickSample.im_w_expr, self.im_w)
            .subs(StickSample.im_h_expr, self.im_h)
            .subs(StickSample.c_t_expr, self.c_t)
         )

    @property
    def c_mat(self):
        return np.array(
            StickSample.c_mat_expr
            .subs(StickSample.im_w_expr, self.im_w)
            .subs(StickSample.im_h_expr, self.im_h)
            .subs(StickSample.c_t_expr, self.c_t)
        ).astype(np.float64)

    @property
    def c_h(self):
        return float(StickSample.c_h_expr.subs(StickSample.c_t_expr, self.c_t))


    @property
    def c_w(self):
        return float(
            StickSample.c_w_expr
            .subs(StickSample.im_h_expr, self.im_h)
            .subs(StickSample.im_w_expr, self.im_w)
            .subs(StickSample.c_r_expr, self.c_t)
        )

    @property
    def s_0_v_x(self):
        return float(
            StickSample.s_0_v_x_expr
            .subs(StickSample.s_0_v_x_in_m_s_expr, self.s_0_v_x_in_m_s)
        )

    @property
    def s_0_v_y(self):
        return float(
            StickSample.s_0_v_y_expr
            .subs(StickSample.s_0_v_y_in_m_s_expr, self.s_0_v_y_in_m_s)
        )

    @property
    def s_0_ang_vel(self):
        return float(
            StickSample.s_0_ang_vel_expr
            .subs(StickSample.s_0_ang_vel_in_rad_s_expr, self.s_0_ang_vel_in_rad_s)
        )

    @property
    def symbolic_continuous_joint(self):
        if self._symbolic_continuous_joint is None:
            self._symbolic_continuous_joint = StickSample.c_t_pdf_expr
            self._symbolic_continuous_joint *= StickSample.stick_len_pdf_expr
            self._symbolic_continuous_joint *= StickSample.s_0_p_x_pdf_expr
            self._symbolic_continuous_joint *= StickSample.s_0_p_y_pdf_expr
            self._symbolic_continuous_joint *= StickSample.s_0_v_x_in_m_s_pdf_expr
            self._symbolic_continuous_joint *= StickSample.s_0_v_y_in_m_s_pdf_expr
            self._symbolic_continuous_joint *= StickSample.s_0_ang_pdf_expr
            self._symbolic_continuous_joint *= StickSample.s_0_ang_vel_in_rad_s_pdf_expr
            self._symbolic_continuous_joint *= self.stick_tree.symbolic_continuous_joint
        return self._symbolic_continuous_joint

    # Since some samples, namely, the fracture tree depth and the time of each fracture, determine the dimensionality
    # of the Bayes Net, this cannot be a class method.
    # This term shouldn't really change unless we change the network structure (number of fractures and fracture times),
    # so, at this point, there's no need to worry about self._symbolic_log_joint becoming stale.
    @property
    def symbolic_continuous_log_joint(self):
        if self._symbolic_continuous_log_joint is None:
            self._symbolic_continuous_log_joint = sympy.utilities.lambdify(
                [
                    StickSample.im_h_expr,
                    StickSample.im_w_expr,
                    StickSample.c_t_expr,
                    StickSample.stick_len_expr,
                    StickSample.s_0_p_x_expr,
                    StickSample.s_0_p_y_expr,
                    StickSample.s_0_v_x_in_m_s_expr,
                    StickSample.s_0_v_y_in_m_s_expr,
                    StickSample.s_0_ang_expr,
                    StickSample.s_0_ang_vel_in_rad_s_expr,
                    # TODO need a better way to get the symbols for the endpoints to put them here.
                    #  Maybe try to represent (time, stick index) as a 2-d array?
                ],
                sp.expand_log(sp.ln(self.symbolic_continuous_joint), force=True),
            )
        return self._symbolic_continuous_log_joint

    @property
    def continuous_log_prob(self):
        if self._continuous_log_prob is None:
            self._continuous_log_prob = self.symbolic_continuous_log_joint \
                .subs(StickSample.im_h_expr,                 self.im_h) \
                .subs(StickSample.im_w_expr,                 self.im_w) \
                .subs(StickSample.c_t_expr,                  self.c_t) \
                .subs(StickSample.stick_len_expr,            self.stick_len) \
                .subs(StickSample.s_0_p_x_expr,              self.s_0_p_x) \
                .subs(StickSample.s_0_p_y_expr,              self.s_0_p_y) \
                .subs(StickSample.s_0_v_x_in_m_s_expr,       self.s_0_v_x_in_m_s) \
                .subs(StickSample.s_0_v_y_in_m_s_expr,       self.s_0_v_y_in_m_s) \
                .subs(StickSample.s_0_ang_expr,              self.s_0_ang) \
                .subs(StickSample.s_0_ang_vel_in_rad_s_expr, self.s_0_ang_vel_in_rad_s)
            for k, v in self.stick_tree.continuous_expr_val_pairs.items():
                self._continuous_log_prob = self._continuous_log_prob.subs(k, v)
            self._continuous_log_prob = float(self._continuous_log_prob)
        return self._continuous_log_prob
    
    @property
    def log_prob(self):
        raise Exception('not implemented')
    
    @property
    def log_prob_der_c_t(self):
        raise Exception('not implemented')

    @property
    def log_prob_der_stick_len(self):
        raise Exception('not implemented')

    # Identity follows the standard identity for binary trees. Root has id = 0, then left child 1, then right child 2,
    # and so on.
    @property
    def log_prob_der_frac_loc_i(self, stick_identity):
        raise Exception('not implemented')

    @property
    def log_prob_der_s_0_p_x(self):
        raise Exception('not implemented')

    @property
    def log_prob_der_s_0_p_y(self):
        raise Exception('not implemented')

    @property
    def log_prob_der_s_0_v_x_in_m_s(self):
        raise Exception('not implemented')

    @property
    def log_prob_der_s_0_v_y_in_m_s(self):
        raise Exception('not implemented')

    @property
    def log_prob_der_s_0_ang(self):
        raise Exception('not implemented')

    @property
    def log_prob_der_s_0_ang_vel_in_rad_s(self):
        raise Exception('not implemented')


# This class will have several elements per nodes. Essentially, each node represents a stick. All non-leaf nodes will
# (should) have symbolic_frac_info and sampled_frac_info set. Every node will have symbolic_stick_states and
# sampled_stick_states set.
class StickTree:

    # The constructor will create a full binary tree of empty nodes with the correct depth.
    def __init__(self, frac_tree_depth, identifier=0):
        self.id = identifier
        if frac_tree_depth > 0:
            self.l = StickTree(frac_tree_depth - 1, self.id * 2 + 1)
            self.r = StickTree(frac_tree_depth - 1, self.id * 2 + 2)
        else:
            self.l = None
            self.r = None
        self.symbolic_frac_info = None
        self.sampled_frac_info = None
        self.symbolic_stick_states = None
        self.sampled_stick_states = None
        self._symbolic_continuous_joint = None
        self._continuous_expr_val_pairs = None

    @classmethod
    def node_depth(cls, identity):
        return math.floor(math.log(identity + 1, 2))

    @classmethod
    def parent_node_id(cls, identity):
        return (identity - 1) // 2

    @property
    def depth(self):
        return StickTree.node_depth(self.id)

    @property
    def parent_id(self):
        return StickTree.parent_node_id(self.id)

    # If calling this on the root, parent_fr_time_expr should be left None
    def populate_symbolic_frac_info(self, my_len_expr, parent_fr_time_expr=None):
        if self.l is None:
            assert self.r is None, 'Tree had a left child but not a right child'
            return
        self.symbolic_frac_info = SymbolicFracInfo(self.id, my_len_expr, parent_fr_time_expr)
        self.l.populate_symbolic_frac_info(self.symbolic_frac_info.frac_loc_expr, self.symbolic_frac_info.frac_time_expr)
        self.r.populate_symbolic_frac_info(self.symbolic_frac_info.childr_len_expr, self.symbolic_frac_info.frac_time_expr)

    def populate_sampled_frac_info(self, num_ims, my_len, rng, parent_fr_time=None):
        if self.l is None:
            assert self.r is None, 'Tree had a left child but not a right child'
            return
        self.sampled_frac_info = SampledFracInfo(self.id, num_ims, my_len, rng, parent_fr_time)
        self.l.populate_sampled_frac_info(num_ims, self.sampled_frac_info.frac_loc, rng, self.sampled_frac_info.frac_time)

        # I would like to determine childr_len by using self.symbolic_frac_info.childr_len_expr, substituting in the
        # various values (to ensure consistency), but the problem is that childr_len_expr potentially cascades into an
        # expression of d unknowns, where d is the depth of that node. I expect there won't be too much trouble if we
        # ensure the invariant that childl_len + childr_len = parent_len
        childr_len = my_len - self.sampled_frac_info.frac_loc
        self.r.populate_sampled_frac_info(num_ims, childr_len, rng, self.sampled_frac_info.frac_time)

    # Recursively populates symbolic_stick_states (for self and children)
    def populate_symbolic_stick_states(self, num_ims, my_len_expr, my_init_state_vec_expr, my_start_time=0):
        if self.l is None:
            assert self.r is None, 'Tree had a left child but not a right child'
            # I'm a leaf. Base case. No fracture, so our end time should be the num_ims
            self.symbolic_stick_states = SymbolicStickStates(
                self.id,
                my_len_expr,
                my_start_time,
                num_ims,
                my_init_state_vec_expr
            )
        else:
            my_end_time = self.sampled_frac_info.frac_time
            self.symbolic_stick_states = SymbolicStickStates(
                self.id,
                my_len_expr,
                my_start_time,
                self.sampled_frac_info.frac_time,
                my_init_state_vec_expr
            )
            init_state_l, init_state_r = self.symbolic_stick_states.get_initial_children_state_exprs(
                self.symbolic_frac_info.frac_loc_expr
            )
            self.l.populate_symbolic_stick_states(
                num_ims,
                self.symbolic_frac_info.frac_loc_expr,
                init_state_l,
                self.sampled_frac_info.frac_time
            )
            self.r.populate_symbolic_stick_states(
                num_ims,
                self.symbolic_frac_info.childr_len_expr,
                init_state_r,
                self.sampled_frac_info.frac_time
            )

    def populate_sampled_stick_states(
            self,
            c_mat,
            s_0_p_x,
            s_0_v_x_in_m_s,
            s_0_p_y,
            s_0_v_y_in_m_s,
            s_0_ang,
            s_0_ang_vel_in_rad_s,
            s_0_len,
            world_endpoint_obs_x_pdf,
            world_endpoint_obs_y_pdf,
            rng,
            parent_fr_locs=[],
    ):
        init_state = np.reshape(SymbolicStickStates.subs_state_expr(
            self.id,
            self.symbolic_stick_states.state_exprs[0],
            s_0_p_x,
            s_0_v_x_in_m_s,
            s_0_p_y,
            s_0_v_y_in_m_s,
            s_0_ang,
            s_0_ang_vel_in_rad_s,
            s_0_len,
            parent_fr_locs,
        ), (7, ))
        my_len = SymbolicStickStates.subs_len_expr(
            self.id,
            self.symbolic_stick_states.len_expr,
            s_0_len,
            parent_fr_locs
        )
        self.sampled_stick_states = SampledStickStates(
            init_state,
            my_len,
            self.symbolic_stick_states.start_time,
            self.symbolic_stick_states.end_time,
            c_mat,
            world_endpoint_obs_x_pdf,
            world_endpoint_obs_y_pdf,
            rng,
        )
        if self.l is not None:
            assert self.r is not None
            self.l.populate_sampled_stick_states(
                c_mat,
                s_0_p_x,
                s_0_v_x_in_m_s,
                s_0_p_y,
                s_0_v_y_in_m_s,
                s_0_ang,
                s_0_ang_vel_in_rad_s,
                s_0_len,
                world_endpoint_obs_x_pdf,
                world_endpoint_obs_y_pdf,
                rng,
                parent_fr_locs + [self.sampled_frac_info.frac_loc],
            )
            self.r.populate_sampled_stick_states(
                c_mat,
                s_0_p_x,
                s_0_v_x_in_m_s,
                s_0_p_y,
                s_0_v_y_in_m_s,
                s_0_ang,
                s_0_ang_vel_in_rad_s,
                s_0_len,
                world_endpoint_obs_x_pdf,
                world_endpoint_obs_y_pdf,
                rng,
                parent_fr_locs + [self.sampled_frac_info.frac_loc],
            )

    # Returns an nx2x2 np.array representing the stick endpoints' actual coordinates, in world space.
    # Dimension 0 indexes over the current active sticks.
    # Dimension 1 indexes over each stick end point (2, in this case).
    # Dimension 2 indexes over (x, y).
    def endpoint_world_coords_act(self, timestamp):
        if self.sampled_stick_states.start_time <= timestamp < self.sampled_stick_states.end_time:
            return np.reshape(
                self.sampled_stick_states.world_endpoints_act[
                    timestamp - self.sampled_stick_states.start_time
                ],
                (1, 2, 2),
            )
        assert self.l is not None and self.r is not None, 'For any given timestamp within the image range, there ' \
                                                          'should be a valid stick in any path from root to leaf'
        return np.append(
            self.l.endpoint_world_coords_act(timestamp),
            self.r.endpoint_world_coords_act(timestamp),
            axis=0,
        )

    # Returns an nx2x2 np.array representing the stick endpoints' observed coordinates, in world space.
    # Dimension indexing is as in endpoint_world_coordinates_act.
    def endpoint_world_coords_obs(self, timestamp):
        if self.sampled_stick_states.start_time <= timestamp < self.sampled_stick_states.end_time:
            return np.reshape(
                self.sampled_stick_states.world_endpoints_obs[
                    timestamp - self.sampled_stick_states.start_time
                ],
                (1, 2, 2),
            )
        return np.append(
            self.l.endpoint_world_coords_obs(timestamp),
            self.r.endpoint_world_coords_obs(timestamp),
            axis=0
        )

    # Returns an nx2x2 np.array representing the stick endpoints' observed coordinates, in image space.
    # Dimension indexing is as in endpoint_world_coordinates_act.
    def endpoint_image_coords_act(self, timestamp):
        if self.sampled_stick_states.start_time <= timestamp < self.sampled_stick_states.end_time:
            return np.reshape(
                self.sampled_stick_states.im_endpoints_act[
                    timestamp - self.sampled_stick_states.start_time
                ],
                (1, 2, 2),
            )
        return np.append(
            self.l.endpoint_image_coords_act(timestamp),
            self.r.endpoint_image_coords_act(timestamp),
            axis=0
        )

    # Returns an nx2x2 np.array representing the stick endpoints' observed coordinates, in image space.
    # Dimension indexing is as in endpoint_world_coordinates_act.
    def endpoint_image_coords_obs(self, timestamp):
        if self.sampled_stick_states.start_time <= timestamp < self.sampled_stick_states.end_time:
            return np.reshape(
                self.sampled_stick_states.im_endpoints_obs[
                    timestamp - self.sampled_stick_states.start_time
                ],
                (1, 2, 2),
            )
        return np.append(
            self.l.endpoint_image_coords_obs(timestamp),
            self.r.endpoint_image_coords_obs(timestamp),
            axis=0
        )

    @property
    def symbolic_continuous_joint(self):
        if self._symbolic_continuous_joint is None:
            self._symbolic_continuous_joint = self.symbolic_stick_states.symbolic_continuous_joint
            if self.l is not None:
                assert self.r is not None
                self._symbolic_continuous_joint *= self.symbolic_frac_info.symbolic_continuous_joint
                self._symbolic_continuous_joint *= self.l.symbolic_continuous_joint
                self._symbolic_continuous_joint *= self.r.symbolic_continuous_joint
        return self._symbolic_continuous_joint

    @property
    def continuous_expr_val_pairs(self):
        if self._continuous_expr_val_pairs is None:
            self._continuous_expr_val_pairs = self._obs_endpoints_expr_val_pairs()
            if self.l is not None:
                assert self.r is not None
                self._continuous_expr_val_pairs[self.symbolic_frac_info.frac_loc_expr] = self.sampled_frac_info.frac_loc
                self._continuous_expr_val_pairs.update(self.l.continuous_expr_val_pairs)
                self._continuous_expr_val_pairs.update(self.r.continuous_expr_val_pairs)
        return self._continuous_expr_val_pairs

    def _obs_endpoints_expr_val_pairs(self):
        r = {}
        for i in range(self.symbolic_stick_states.lifetime):
            for endpoint_idx in range(2):
                for dim_idx in range(2):
                    r[self.symbolic_stick_states.world_endpoints_obs_exprs[i][endpoint_idx, dim_idx]] = \
                        self.sampled_stick_states.world_endpoints_obs[i][endpoint_idx, dim_idx]
        return r


# Will be contained within each non-leaf node in an instance of StickTree. Will contain the symbolic equations for
# fracture position and fracture timestamp.
class SymbolicFracInfo:

    frac_time_0_low = 1
    frac_time_0_high_expr = StickSample.num_ims_expr // 4
    frac_time_0_expr = sp.symbols('frac_time_0')
    frac_time_0_pdf_expr = symbolic_prob.uniform_pmf(
        frac_time_0_expr,
        frac_time_0_low,
        frac_time_0_high_expr
    )
    frac_time_i_p = 0.5

    def __init__(self, identity, my_len_expr, parent_fr_time_expr=None):
        if parent_fr_time_expr is None:  # identity should also be 0 at this point.
            self.frac_time_expr = SymbolicFracInfo.frac_time_0_expr
            self.frac_time_pmf_expr = SymbolicFracInfo.frac_time_0_pdf_expr
        else:
            self.frac_time_expr = frac_time_expr(identity)
            self.frac_time_pmf_expr = symbolic_prob.geometric_pmf(
                self.frac_time_expr,
                SymbolicFracInfo.frac_time_i_p,
                parent_fr_time_expr
            )

        self.len_expr = my_len_expr
        self.frac_loc_mean_expr = self.len_expr / 2
        self.frac_loc_std_expr = self.len_expr / 8
        self.frac_loc_expr = frac_loc_expr(identity)
        self.frac_loc_pdf_expr = symbolic_prob.norm_pdf(
            self.frac_loc_expr,
            self.frac_loc_mean_expr,
            self.frac_loc_std_expr
        )
        self.childr_len_expr = self.len_expr - self.frac_loc_expr

    @property
    def symbolic_continuous_joint(self):
        return self.frac_loc_pdf_expr


# Will be contained within each non-leaf node in an instance of StickTree. Will contain the sampled values for fracture
# position and fracture timestamp.
class SampledFracInfo:

    def __init__(self, identity, num_ims, my_len, rng, parent_fr_time=None):
        if identity == 0:
            self.frac_time_pmf = stats.randint(
                SymbolicFracInfo.frac_time_0_low,
                int(SymbolicFracInfo.frac_time_0_high_expr.subs(StickSample.num_ims_expr, num_ims) + 1)
            )
        else:
            # stats.geom is defined over 1..inf, so no need to add 1 here.
            self.frac_time_pmf = stats.geom(loc=parent_fr_time, p=SymbolicFracInfo.frac_time_i_p)
        self.frac_time = self.frac_time_pmf.rvs(random_state=rng)

        self.len = my_len
        self.frac_loc_mean = my_len / 2
        self.frac_loc_std = my_len / 8
        self.frac_loc_pdf = stats.norm(loc=self.frac_loc_mean, scale=self.frac_loc_std)
        self.frac_loc = self.frac_loc_pdf.rvs(random_state=rng)


class SymbolicStickStates:

    state_trans_mat_expr = state_trans_matrix_expr()

    world_endpoints_obs_x_std_expr = sp.sqrt(StickSample.c_h_expr * StickSample.c_w_expr) / 64
    world_endpoints_obs_y_std_expr = sp.sqrt(StickSample.c_h_expr * StickSample.c_w_expr) / 64

    # my_end_time is exclusive.
    def __init__(
            self,
            identity,
            my_len_expr,
            my_start_time,
            my_end_time,
            my_initial_state_expr
    ):
        self.len_expr = my_len_expr
        self.start_time = my_start_time
        self.end_time = my_end_time
        self.lifetime = self.end_time - self.start_time
        # These could feasibly be calculated on demand in @property methods, but memory's cheap, so I'd rather just
        # precompute these and store the expressions here. Additionally, I don't expect these objects' members to
        # change, so there's no possibility of the precomputed values becoming stale. If it becomes a problem later on,
        # I'll consider computing them on demand.
        self.state_exprs = [None] * self.lifetime
        self.world_endpoints_act_exprs = [None] * self.lifetime

        self.world_endpoints_obs_mean_exprs = [None] * self.lifetime
        self.world_endpoints_obs_exprs = [None] * self.lifetime
        self.world_endpoints_obs_pdf_exprs = [None] * self.lifetime

        self.local_endpoints_expr = SymbolicStickStates.len_expr_to_local_endpoints_expr(my_len_expr)

        cur_state_expr = my_initial_state_expr
        for i in range(self.lifetime):
            self.state_exprs[i] = cur_state_expr
            self.world_endpoints_act_exprs[i] = SymbolicStickStates.state_vec_to_world_endpoints(
                cur_state_expr,
                self.local_endpoints_expr
            )
            cur_state_expr = SymbolicStickStates.state_trans_mat_expr * cur_state_expr

        self.world_endpoints_obs_mean_exprs = self.world_endpoints_act_exprs

        for i in range(self.lifetime):
            self.world_endpoints_obs_exprs[i] = sp.Matrix([
                [stick_endpoint_obs_expr(my_start_time + i, identity, 0, 'x'),    stick_endpoint_obs_expr(my_start_time + i, identity, 0, 'y')],
                [stick_endpoint_obs_expr(my_start_time + i, identity, 1, 'x'),    stick_endpoint_obs_expr(my_start_time + i, identity, 1, 'y')],
            ])
            self.world_endpoints_obs_pdf_exprs[i] = sp.Matrix([
                [
                    symbolic_prob.norm_pdf(
                        self.world_endpoints_obs_exprs[i][0, 0],
                        self.world_endpoints_obs_mean_exprs[i][0, 0],
                        SymbolicStickStates.world_endpoints_obs_x_std_expr
                    ),
                    symbolic_prob.norm_pdf(
                        self.world_endpoints_obs_exprs[i][0, 1],
                        self.world_endpoints_obs_mean_exprs[i][0, 1],
                        SymbolicStickStates.world_endpoints_obs_y_std_expr
                    )
                ],
                [
                    symbolic_prob.norm_pdf(
                        self.world_endpoints_obs_exprs[i][1, 0],
                        self.world_endpoints_obs_mean_exprs[i][1, 0],
                        SymbolicStickStates.world_endpoints_obs_x_std_expr
                    ),
                    symbolic_prob.norm_pdf(
                        self.world_endpoints_obs_exprs[i][1, 1],
                        self.world_endpoints_obs_mean_exprs[i][1, 1],
                        SymbolicStickStates.world_endpoints_obs_y_std_expr
                    )
                ],
            ])

    # Since not all sticks will fracture, we don't figure this in the constructor. Instead, we let the tree node for
    # which we are a member deal with that.
    def get_initial_children_state_exprs(self, frac_loc_expr):
        final_state_expr = self.state_exprs[len(self.state_exprs) - 1]
        offset_factor = sp.Matrix([
            [sp.cos(final_state_expr[state_ang_idx()])],
            [(sp.cos(final_state_expr[state_ang_idx()] + final_state_expr[state_ang_vel_idx()])
                - sp.cos(final_state_expr[state_ang_idx()]))],
            [sp.sin(final_state_expr[state_ang_idx()])],
            [(sp.sin(final_state_expr[state_ang_idx()] + final_state_expr[state_ang_vel_idx()])
                - sp.sin(final_state_expr[state_ang_idx()]))],
            [0],
            [0],
            [0],
        ])
        c_l_o = -1/2 * (self.len_expr - frac_loc_expr)
        c_r_o = 1/2 * frac_loc_expr
        c_l_s = SymbolicStickStates.state_trans_mat_expr * (final_state_expr + c_l_o * offset_factor)
        c_r_s = SymbolicStickStates.state_trans_mat_expr * (final_state_expr + c_r_o * offset_factor)
        return c_l_s, c_r_s

    # Returns a 3x2 matrix. Each column is an endpoint in local coordinates.
    @classmethod
    def len_expr_to_local_endpoints_expr(cls, len_expr):
        return sp.Matrix([
            [-len_expr / 2, 0],
            [ len_expr / 2, 0],
        ])

    # Returns a 2x2 matrix. Each column is an endpoint in world coordinates.
    @classmethod
    def state_vec_to_world_endpoints(cls, state_vec_expr, local_endpoints_expr):
        return util.apply_homogeneous_transformation_sym(
            cls.state_expr_to_trans_matrix_expr(state_vec_expr),
            local_endpoints_expr
        )

    # Returns a 3x3 matrix which converts points in local coordinates to points in world coordinates.
    @classmethod
    def state_expr_to_trans_matrix_expr(cls, state_expr):
        return sp.Matrix([
            [sp.cos(state_expr[state_ang_idx()]), -sp.sin(state_expr[state_ang_idx()]), state_expr[state_p_x_idx()]],
            [sp.sin(state_expr[state_ang_idx()]),  sp.cos(state_expr[state_ang_idx()]), state_expr[state_p_y_idx()]],
            [                                  0,                                    0,                           1]
        ])

    @classmethod
    def subs_len_expr(cls, identity, len_expr, s_0_len, parent_fr_locs):
        parent_ids = cls._create_node_id_chain(StickTree.parent_node_id(identity))

        cur_expr = len_expr.subs(StickSample.stick_len_expr, s_0_len)
        for i in range(len(parent_ids)):
            cur_expr = cur_expr.subs(frac_loc_expr(parent_ids[i]), parent_fr_locs[i])
        return float(cur_expr)

    @classmethod
    def subs_state_expr(
            cls,
            identity,
            state_expr,
            s_0_p_x,
            s_0_v_x_in_m_s,
            s_0_p_y,
            s_0_v_y_in_m_s,
            s_0_ang,
            s_0_ang_vel_in_rad_s,
            s_0_len,
            parent_fr_locs
    ):
        parent_ids = cls._create_node_id_chain(StickTree.parent_node_id(identity))

        cur_expr = state_expr \
            .subs(StickSample.s_0_p_x_expr, s_0_p_x) \
            .subs(StickSample.s_0_v_x_in_m_s_expr, s_0_v_x_in_m_s) \
            .subs(StickSample.s_0_p_y_expr, s_0_p_y) \
            .subs(StickSample.s_0_v_y_in_m_s_expr, s_0_v_y_in_m_s) \
            .subs(StickSample.s_0_ang_expr, s_0_ang) \
            .subs(StickSample.s_0_ang_vel_in_rad_s_expr, s_0_ang_vel_in_rad_s) \
            .subs(StickSample.stick_len_expr, s_0_len)
        for i in range(len(parent_ids)):
            cur_expr = cur_expr.subs(frac_loc_expr(parent_ids[i]), parent_fr_locs[i])
        return np.reshape(np.array(cur_expr).astype(np.float64), (7, ))

    # Chains the given node id and all parent node ids into a list, starting with element 0 being the root node of the
    # tree, continuing on until the last list element is the given id. This helper is basically for collecting all ids
    # to
    @classmethod
    def _create_node_id_chain(cls, cur_id):
        if cur_id < 0:
            return []
        result = cls._create_node_id_chain(StickTree.parent_node_id(cur_id))
        result.append(cur_id)
        return result

    @property
    def symbolic_continuous_joint(self):
        r = 1
        for i in range(self.lifetime):
            for endpoint_idx in range(2):
                for dim_idx in range(2):
                    r *= self.world_endpoints_obs_pdf_exprs[i][endpoint_idx, dim_idx]
        return r


class SampledStickStates:

    state_trans_mat = state_trans_matrix_from_sym()

    # Ideally, since most of a stick's states are deterministic, given the initial conditions and fracture times,
    # locations, we'd want to just substitute values into the corresponding SymbolicStickStates. This reduces the amount
    # of "redundant" calculations (numerical vs symbolic calculations), but it might be a pain to substitute in all
    # those values. Additionally, trans_mat is based on trans_mat_expr, so, those calculations should be consistent
    # regardless. Perhaps a good middle ground is performing the state tick transformations both ways, but determining
    # initial state by substituting into the SymbolicStickStates.state_exprs[0].
    def __init__(
            self,
            my_initial_state,
            my_len,
            my_start_time,
            my_end_time,
            c_mat,
            world_endpoint_obs_x_pdf,
            world_endpoint_obs_y_pdf,
            rng
    ):
        self.len = my_len
        self.start_time = my_start_time
        self.end_time = my_end_time
        self.lifetime = self.end_time - self.start_time

        self.local_endpoints = SampledStickStates.len_to_local_endpoints(self.len)
        self.states = [None] * self.lifetime
        self.world_endpoints_act = [None] * self.lifetime
        self.im_endpoints_act = [None] * self.lifetime
        self.world_endpoints_obs = [None] * self.lifetime
        self.im_endpoints_obs = [None] * self.lifetime
        cur_state = my_initial_state
        for i in range(self.lifetime):
            self.states[i] = cur_state
            self.world_endpoints_act[i] = util.apply_homogeneous_transformation(
                SampledStickStates.state_to_trans_matrix(self.states[i]),
                self.local_endpoints
            )
            self.im_endpoints_act[i] = util.apply_homogeneous_transformation(
                c_mat,
                self.world_endpoints_act[i]
            )
            self.world_endpoints_obs[i] = np.copy(self.world_endpoints_act[i])
            self.world_endpoints_obs[i][:, 0] += world_endpoint_obs_x_pdf.rvs((2, ), random_state=rng)
            self.world_endpoints_obs[i][:, 1] += world_endpoint_obs_y_pdf.rvs((2, ), random_state=rng)
            self.im_endpoints_obs[i] = util.apply_homogeneous_transformation(
                c_mat,
                self.world_endpoints_obs[i]
            )

            cur_state = SampledStickStates.state_trans_mat @ cur_state

    # Returns a 2x2 matrix. Each column is an endpoint in local coordinates.
    @classmethod
    def len_to_local_endpoints(cls, length):
        return np.array([
            [-length / 2, 0],
            [ length / 2, 0],
        ])

    # Returns a 3x3 matrix which converts points in local coordinates to points in world coordinates.
    @classmethod
    def state_to_trans_matrix(cls, state):
        return np.array([
            [math.cos(state[state_ang_idx()]), -math.sin(state[state_ang_idx()]), state[state_p_x_idx()]],
            [math.sin(state[state_ang_idx()]),  math.cos(state[state_ang_idx()]), state[state_p_y_idx()]],
            [                               0,                                 0,                      1]
        ])


def frac_time_expr(node_id):
    return sp.Symbol('fr_time_{}'.format(node_id))


def frac_loc_expr(node_id):
    return sp.Symbol('fr_loc_{}'.format(node_id))

# dim should probably be x or y, or use whatever indexing style you want. Just be consistent.
def stick_endpoint_obs_expr(node_id, timestamp, point_id, dim):
    return sp.Symbol('ep_obs_{}_{}_{}_{}'.format(timestamp, node_id, point_id, dim))
