import json
import numpy as np
import scipy.stats
import sys

# Relative path imports
sys.path.append('../lib')
import util
import discretization
import discrete_prob

def c_t_dist_fn(alpha, beta):
    return scipy.stats.uniform(alpha, beta - alpha)

def c_r_fn(c_l, c_t, c_b, im_w, im_h):
    # units-per-pixel: (c_t - c_b) / im_h
    return c_l + im_w * ((c_t - c_b) / im_h)

def c_mat_fn(c_l, c_r, c_t, c_b, im_w, im_h):
    return np.array([[                 0, -im_h / (c_t - c_b),  c_t * im_h / (c_t - c_b)],
                     [im_w / (c_r - c_l),                   0, -c_l * im_w / (c_r - c_l)],
                     [                 0,                   0,                         1]])

def s_0_p_x_mean_fn(c_l, c_r):
    return (c_l + c_r) / 2

def s_0_p_x_stddev_fn(c_l, c_r):
    return (c_r - c_l) / 16

def s_0_p_x_dist_fn(mean, stddev):
    return scipy.stats.norm(loc = mean, scale = stddev)

def s_0_v_x_dist_fn(mean, stddev):
    return scipy.stats.norm(loc = mean, scale = stddev)

def s_0_p_y_mean_fn(c_t, c_b):
    return (c_t + c_b) / 2

def s_0_p_y_stddev_fn(c_t, c_b):
    return (c_t - c_b) / 16

def s_0_p_y_dist_fn(mean, stddev):
    return scipy.stats.norm(loc = mean, scale = stddev)

def s_0_v_y_dist_fn(mean, stddev):
    return scipy.stats.norm(loc = mean, scale = stddev)

def s_0_a_y_fn(gravity, c_fps):
    return gravity / c_fps

def s_0_fn(s_0_p_x, s_0_v_x, s_0_p_y, s_0_v_y, s_0_a_y):
    return np.array([
        s_0_p_x,
        s_0_v_x,
        s_0_p_y,
        s_0_v_y,
        s_0_a_y])

def s_i_fn(s_i_minus_1, transition):
    return transition @ s_i_minus_1

def cam_p_xy_stddev_fn(im_width, im_height):
    return (im_width * im_height) ** (1/2) / 64
    
def cam_p_xy_dist_fn(mean, stddev):
    return scipy.stats.norm(loc = mean, scale = stddev)

def cam_i_dist_fn(mean, stddev):
    return scipy.stats.norm(loc = mean, scale = stddev)

if __name__ == '__main__':

    num_samples = 10

    # Fixed nodes
    im_w = 640
    im_h = 480
    num_ims = 30

    c_l = 0.0
    c_b = 0.0
    c_t_a = 5.0
    c_t_b = 50.0

    c_fps = 30.0

    s_0_v_x_mean = 0.0
    s_0_v_x_stddev = 1.0
    s_0_v_y_mean = 0.0
    s_0_v_y_stddev = 1.0

    # This noise will emulate perturbations due to wind resistance.
    # TODO revisit these. They're not necessary for now.
    # s_n_p_x_stddev = 0.2
    # s_n_p_y_stddev = 0.2
    # s_n_v_x_stddev = 0.05
    # s_n_v_y_stddev = 0.05

    # This noise will emulate sensor error, like the bumping of the camera or 
    # per-pixel value noise.
    # cam_xy_stddev (should depend on camera width and height)
    cam_pixel_intensity_mean = 0.0
    # Image intensities are in the range [0, 1], to be converted to 0..255 later.
    cam_pixel_intensity_stddev = 0.1
    cam_pixel_intensity_dist = cam_i_dist_fn(cam_pixel_intensity_mean, cam_pixel_intensity_stddev)

    gravity = -9.8

    transition = np.array([
        [1, 1, 0, 0, 0],
        [0, 1, 0, 0, 0],
        [0, 0, 1, 1, 0],
        [0, 0, 0, 1, 1],
        [0, 0, 0, 0, 1]])

    cam_p_xy_stddev = cam_p_xy_stddev_fn(im_w, im_h)

    for i in range(num_samples):
        sample = {}
        c_t = c_t_dist_fn(c_t_a, c_t_b).rvs()
        sample['c_t'] = c_t
        c_r = c_r_fn(c_l, c_t, c_b, im_w, im_h)
        sample['c_r'] = c_r
        c_mat = c_mat_fn(c_l, c_r, c_t, c_b, im_w, im_h)
        sample['c_mat'] = c_mat.tolist()
        s_0_p_x_mean = s_0_p_x_mean_fn(c_l, c_r)
        sample['s_0_p_x_mean'] = s_0_p_x_mean
        s_0_p_x_stddev = s_0_p_x_stddev_fn(c_l, c_r)
        sample['s_0_p_x_stddev'] = s_0_p_x_stddev
        s_0_p_x = s_0_p_x_dist_fn(s_0_p_x_mean, s_0_p_x_stddev).rvs()
        sample['s_0_p_x'] = s_0_p_x
        s_0_v_x = s_0_v_x_dist_fn(s_0_v_x_mean, s_0_v_x_stddev).rvs()
        sample['s_0_v_x'] = s_0_v_x
        s_0_p_y_mean = s_0_p_y_mean_fn(c_t, c_b)
        sample['s_0_p_y_mean'] = s_0_p_y_mean
        s_0_p_y_stddev = s_0_p_y_stddev_fn(c_t, c_b)
        sample['s_0_p_y_stddev'] = s_0_p_y_stddev
        s_0_p_y = s_0_p_y_dist_fn(s_0_p_y_mean, s_0_p_y_stddev).rvs()
        sample['s_0_p_y'] = s_0_p_y
        s_0_v_y = s_0_v_y_dist_fn(s_0_v_y_mean, s_0_v_y_stddev).rvs()
        sample['s_0_v_y'] = s_0_v_y
        s_0_a_y = s_0_a_y_fn(gravity, c_fps)
        sample['s_0_a_y'] = s_0_a_y

        states = np.zeros((5, num_ims))
        states[:, 0] = s_0_fn(s_0_p_x, s_0_v_x, s_0_p_y, s_0_v_y, s_0_a_y)
        for j in range(1, num_ims):
            states[:, j] = s_i_fn(states[:, j - 1], transition)
        sample['states'] = states.tolist()
        images = np.zeros((im_h, im_w, num_ims))
        for j in range(num_ims):
            world_point = states[[0, 2], j]
            image_point = util.apply_homogeneous_transformation(c_mat, world_point)
            image_point_y_perturbed = cam_p_xy_dist_fn(image_point[0], cam_p_xy_stddev).rvs()
            image_point_x_perturbed = cam_p_xy_dist_fn(image_point[1], cam_p_xy_stddev).rvs()
            image_point_perturbed = np.array([round(image_point_y_perturbed), round(image_point_x_perturbed)], np.int64)
            print(image_point_perturbed)
            util.draw_px(images[:, :, j], image_point_perturbed, 1.0)
            #print(cam_pixel_intensity_dist.rvs((im_h, im_w)))
            images[:, :, j] += cam_pixel_intensity_dist.rvs((im_h, im_w))
            #print(images[:, :, j])
            util.save_matrix_image(images[:, :, j], 'image_{:06}_{:06}.png'.format(i, j))
        sample['images'] = images.tolist()
        with open('variables_{:06}.json'.format(i), 'w') as f:
            json.dump(sample, f, sort_keys=True, indent=2)