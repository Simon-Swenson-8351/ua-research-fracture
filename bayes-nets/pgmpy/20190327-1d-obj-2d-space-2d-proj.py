import numpy as np
import pymc3 as pm

if __name__ == '__main__':

    num_fractures = 4

    frames_between_fractures = [2, 4, 8]

    # Translation: 1.0 kg / meter
    stick_len_density = 5.0

    init_stick_len_mean = 1.0
    init_stick_len_var = 0.2

    camera_pos_mean = np.zeros(2)
    camera_pos_var = 0.2

    camera_width_mean = 2.0
    camera_width_var = 0.4

    camera_height_mean = 2.0
    camera_height_var = 0.4

    # Not a big deal if the sample ends up being negative, since it just means 
    # that the force vector goes in the opposite direction of the angle
    # SI unit: Newton, kg m / s^2. In our case, 
    force_mean = 1.0
    force_std_dev = 1.0

    

    with pm.Model() as model:
        # Start with 1-d shape embedded in 2-d space.
        stick_len_0 = pm.Normal(mu = 1.0, st = 2.0)
        stick_pos_0 = [pm.Constant('stick_pos_0_x', value = 0), pm.Constant('stick_pos_0_y', value = 0)]
        stick_ang_vel_0 = pm.Constant('stick_ang_vel_0', value = 0)
        for i in range(num_fractures):
            fracture_loc = pm.Uniform(stick_len_0 / 2)

def split_stick(num_splits, parent_stick_len, parent_stick_loc, parent_stick_rot):
    if num_splits == 0:
        return
    fracture_loc = pm.Uniform()