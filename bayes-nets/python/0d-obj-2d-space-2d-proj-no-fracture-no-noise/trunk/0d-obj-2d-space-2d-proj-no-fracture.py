import json
import numpy as np
import scipy.stats
import PIL.Image

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
    return (c_r - c_l) ** (1/4)

def s_0_p_x_dist_fn(mean, stddev):
    return scipy.stats.norm(loc = mean, scale = stddev)

def s_0_v_x_dist_fn(mean, stddev):
    return scipy.stats.norm(loc = mean, scale = stddev)

def s_0_p_y_mean_fn(c_t, c_b):
    return (c_t + c_b) / 2

def s_0_p_y_stddev_fn(c_t, c_b):
    return (c_t - c_b) ** (1/4)

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

# Coordinate is a column vector, row, then column
def draw_px(im, coord, intensity):
    if     coord[0] < 0 \
        or coord[0] >= im.shape[0] \
        or coord[1] < 0 \
        or coord[1] >= im.shape[1] \
        or intensity < 0.0 \
        or intensity > 1.0:

        return

    im[coord[0], coord[1]] = intensity


def draw_pt_non_homo(im, non_homo_world_coord, cam_mat, intensity):
    draw_pt_homo(im, non_homo_coord_to_homo_coord(non_homo_world_coord), cam_mat, intensity)

def draw_pt_homo(im, homo_world_coord, cam_mat, intensity):
    im_coord = homo_coord_to_non_homo_coord(cam_mat @ homo_world_coord)
    im_coord_temp = np.zeros((2, 1), np.int64)
    im_coord_temp[0] = round(im_coord[0])
    im_coord_temp[1] = round(im_coord[1])
    im_coord = im_coord_temp

    draw_px(im, im_coord, intensity)

def non_homo_coord_to_homo_coord(non_homo_coord):
    result = np.zeros((non_homo_coord.shape[0] + 1))
    result[0:non_homo_coord.shape[0]] = non_homo_coord
    result[non_homo_coord.shape[0]] = 1.0
    return result

def homo_coord_to_non_homo_coord(homo_coord):
    result = np.zeros((homo_coord.shape[0] - 1))
    result = homo_coord[0:result.shape[0]];
    result = result / homo_coord[result.shape[0]]
    return result

# Matrix is assumed to be row, column indexed
def save_matrix_image(matrix, image_filename):
    scaled = matrix + matrix.min()
    if matrix.max() != 0:
        scaled = scaled / matrix.max() * 255.0
    scaled = scaled.astype(np.uint8)
    PIL.Image.fromarray(scaled).save(image_filename)

if __name__ == '__main__':

    num_samples = 10

    # Fixed nodes
    im_w = 640
    im_h = 480
    num_ims = 30

    c_l = 0.0
    c_b = 0.0
    c_t_a = 10.0
    c_t_b = 100.0

    c_fps = 30.0

    s_0_v_x_mean = 0.0
    s_0_v_x_stddev = 1.0
    s_0_v_y_mean = 0.0
    s_0_v_y_stddev = 1.0

    gravity = -9.8

    transition = np.array([
        [1, 1, 0, 0, 0],
        [0, 1, 0, 0, 0],
        [0, 0, 1, 1, 0],
        [0, 0, 0, 1, 1],
        [0, 0, 0, 0, 1]])

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
            draw_pt_non_homo(images[:, :, j], world_point, c_mat, 1.0)
            save_matrix_image(images[:, :, j], 'image_{:06}_{:06}.png'.format(i, j))
        sample['images'] = images.tolist()
        with open('variables_{:06}.json'.format(i), 'w') as f:
            json.dump(sample, f, sort_keys=True, indent=2)