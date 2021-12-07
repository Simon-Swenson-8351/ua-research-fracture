import pystan
import numpy as np

if __name__ == '__main__':
    model_filename = '0d-obj-2d-space-2d-proj-no-fracture.stan'
    model_data = {
        'im_width': 800,
        'im_height': 600,
        'num_ims': 50,
        'gravity': -9.8,
        'cam_bot': 0.0,
        'cam_left': 0.0,
        'cam_fps': 30.0,
        'run_estimation': 0,
        'cam_top_alpha': 1.0,
        'cam_top_beta': 10.0,
        'p_x_0_stddev': 25.0,
        'v_x_0_mean': 0.0,
        'v_x_0_stddev': 1.0,
        'p_y_0_stddev': 25.0,
        'v_y_0_mean': 0.0,
        'v_y_0_stddev': 1.0,
    }
    
    model = pystan.StanModel(file=model_filename)
    sample = model.sampling(data=model_data, chains=1, iter=1)

    print(sample)
