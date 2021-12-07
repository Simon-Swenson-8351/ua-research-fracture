from pgmpy.models import BayesianModel

if __name__ == '__main__':
    cam_top = 2.0
    cam_bot = 0.0
    cam_left = -1.0
    cam_right = 1.0

    gravity = -9.8

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
        ]