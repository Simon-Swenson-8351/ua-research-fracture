import numpy as np
from pgmpy.models import BayesianModel
from pgmpy.factors.discrete import DiscreteFactor
from pgmpy.factors.continuous import ContinuousFactor

if __name__ == '__main__':

    fracture_0d_2d_2d = BayesianModel()

    fracture_0d_2d_2d.add_nodes_from(['p_x_0', 'v_x_0', 'p_y_0', 'v_y_0', 'a_y_0'])
    
    fracture_0d_2d_2d.add_node('frac_time_1')

    # Assumes vector is of form [p_x, v_x, p_y, v_y, a_y]
    state_transition_matrix = np.array(\
        [[1, 1, 0, 0, 0],\
         [0, 1, 0, 0, 0],\
         [0, 0, 1, 1, 0],\
         [0, 0, 0, 1, 1],\
         [0, 0, 0, 0, 1]])

    gravity = -9.8

    cam_top = 20.0
    cam_bot = 0.0
    cam_left = -10.0
    cam_right = 10.0

    im_width = 400
    im_height = 400
    im_width_units_per_px = (cam_right - cam_left) / im_width
    im_height_units_per_px = (cam_top - cam_bot) / im_height

    p_x_init = 0.0
    p_y_init = 10.0
    v_x_init = 0.0
    v_y_init = 0.0
    a_x_init = 0.0
    a_y_init = gravity
    

    frac_time = 5.0

    # p_x - Position x
    # p_y - Position y
    # v_x - Velocity x
    # v_y - Velocity y
    # a_x - Acceleration x
    # a_y - Acceleration y

    # rot_vec_x
    # rot_vec_y
    # (Only need these for 3-d)

    # angle
    # angular_v
    # angular_a

    # endpoints - two points representing the endpoints of the stick in center-of-
    # mass relative coordinates
    initial_state = [
        0.0, \
        2.0, \
        0.0, \
        0.0, \
        0.0, \
        gravity, \
        0.0, \
        0.0, \
        0.0,
        ]