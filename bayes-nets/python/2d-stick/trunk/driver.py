import argparse
import os
import json
import multiprocessing as mp

import numpy as np
import numpy.random as random
import numpy.linalg as linalg
import matplotlib.pyplot as plt

import util
import drawing

import stick_bayes_net
import stick_sample
import stick_mh_sampler


def do_parse_args():

    parser = argparse.ArgumentParser(description='Represents a Bayesian Network for a stick in 2 dimensions. You can '
                                                 'either generate samples or run inference on them.')

    # For generate option
    parser.add_argument(
        '--generate', '-G',
        action='store_true',
        help='Using the Bayesian Network, generate a number of samples.'
    )
    parser.add_argument(
        '--im_w',
        default=320,
        type=int,
        help='The image width of the samples to generate'
    )
    parser.add_argument(
        '--im_h',
        default=240,
        type=int,
        help='The image height of the samples to generate'
    )
    parser.add_argument(
        '--num_ims', '-i',
        default=30,
        type=int,
        help='The number of images in the sequence to generate'
    )
    parser.add_argument(
        '--num_samples', '-n',
        default=1,
        type=int,
        help='The number of samples to generate'
    )

    # For inference option
    parser.add_argument(
        '--inference', '-I',
        action='store_true',
        help='Performs inference for the samples'
    )

    parser.add_argument(
        '--num_burn_in_samples',
        default=1000,
        type=int,
        help='The inference method used is Gibbs Sampling, which requires a burn-in time to get samples that '
             'accurately represent the posterior.'
    )

    parser.add_argument(
        '--num_inference_samples',
        default=1000,
        type=int,
        help='After running for num_burn_in_samples, num_inference_samples are collected to estimate the posterior '
             'distribution.'
    )

    parser.add_argument(
        '--num_chains',
        default=mp.cpu_count(),
        type=int,
        help='When running inference, how many Markov chains are used. Also increases performance by using multiple '
             'processes.'
    )

    # For both options
    parser.add_argument(
        '--seed', '-s',
        default=42,
        type=int,
        help='The initial seed for the random number generator.'
    )
    parser.add_argument(
        '--folder', '-f',
        default='samples/',
        help='The folder at which to generate and/or run inference on the samples. Each sample with have a separate '
             'sub-folder within this folder.'
    )

    return parser.parse_args()


# Here, we define the two endpoints of the stick in a local coordinate frame in which the stick's center of mass is
# located at (0, 0). Conveniently, the two end points will be the additive inverse of each other in this coordinate
# frame.


# Given a the position and angle of a stick's center of mass in world coordinates, returns the homogeneous matrix
# transformation which will convert points in the stick's relative coordinate space to world coordinates.



def get_actual_image_filename(sample_folder, sample_num, im_num):
    dirname = os.path.join(sample_folder, '{:06}'.format(sample_num), 'forward-sampling', 'actual-images')
    os.makedirs(dirname, exist_ok=True)
    return os.path.join(dirname, '{:06}_{:06}-actual.png'.format(sample_num, im_num))


def get_observed_image_filename(sample_folder, sample_num, im_num):
    dirname = os.path.join(sample_folder, '{:06}'.format(sample_num), 'forward-sampling', 'observed-images')
    os.makedirs(dirname, exist_ok=True)
    return os.path.join(dirname, '{:06}_{:06}-observed.png'.format(sample_num, im_num))


def get_forward_sampling_overlayed_image_filename(sample_folder, sample_num, im_num):
    dirname = os.path.join(sample_folder, '{:06}'.format(sample_num), 'forward-sampling', 'overlayed-images')
    os.makedirs(dirname, exist_ok=True)
    return os.path.join(dirname, '{:06}_{:06}-overlayed.png'.format(sample_num, im_num))


def get_forward_sampling_json_filename(sample_folder, sample_num):
    dirname = os.path.join(sample_folder, '{:06}'.format(sample_num), 'forward-sampling')
    os.makedirs(dirname, exist_ok=True)
    return os.path.join(dirname, '{:06}-forward-sample.json'.format(sample_num))


def get_inference_json_filename(sample_folder, sample_num):
    dirname = os.path.join(sample_folder, '{:06}'.format(sample_num), 'inference')
    os.makedirs(dirname, exist_ok=True)
    return os.path.join(dirname, '{:06}-inference.json'.format(sample_num))


def get_inference_plot_filename(sample_folder, sample_num):
    dirname = os.path.join(sample_folder, '{:06}'.format(sample_num), 'inference')
    os.makedirs(dirname, exist_ok=True)
    return os.path.join(dirname, '{:06}-inference-samples.png'.format(sample_num))


def get_inference_image_filename(sample_folder, sample_num, im_num):
    dirname = os.path.join(sample_folder, '{:06}'.format(sample_num), 'inference', 'inferred-images')
    os.makedirs(dirname, exist_ok=True)
    return os.path.join(dirname, '{:06}_{:06}-inferred.png'.format(sample_num, im_num))


def get_inference_overlayed_image_filename(sample_folder, sample_num, im_num):
    dirname = os.path.join(sample_folder, '{:06}'.format(sample_num), 'inference', 'overlayed-images')
    os.makedirs(dirname, exist_ok=True)
    return os.path.join(dirname, '{:06}_{:06}-overlayed.png'.format(sample_num, im_num))


def offset_camera_matrix(c_mat, offset_x, offset_y):
    # since image coordinates are indexed y first, we need to put y first here.
    return np.array([
        [1, 0, offset_y],
        [0, 1, offset_x],
        [0, 0,        1],
    ]) @ c_mat


def build_stick_drawer():
    return drawing.StickDrawer(
        actual_stick_style=drawing.Stroke(
            stroke_width=1,
            stroke_color=np.array([0.0, 1.0, 0.0]),
            stroke_dash_style=drawing.LineDashStyle.SOLID,
        ),
        observed_stick_style=drawing.Stroke(
            stroke_width=1,
            stroke_color=np.array([1.0, 0.0, 0.0]),
            stroke_dash_style=drawing.LineDashStyle.SOLID,
        ),
        inferred_stick_style=drawing.Stroke(
            stroke_width=1,
            stroke_color=np.array([1.0, 0.0, 1.0]),
            stroke_dash_style=drawing.LineDashStyle.SOLID,
        ),
    )


def build_drawn_camera(boundary_top, boundary_bottom, boundary_left, boundary_right, color, draw_ticks=False):
    grid_line_stroke = drawing.Stroke(
        stroke_width=1,
        stroke_color=color,
        stroke_dash_style=drawing.LineDashStyle.DOTTED,
    )
    if draw_ticks:
        xstyle = drawing.AxisStyle(
            grid_line_stroke,
            5.0,
            drawing.AxisStyleType.GRID,
            drawing.AxisOrientation.HORIZONTAL,
        )
        ystyle = drawing.AxisStyle(
            grid_line_stroke,
            5.0,
            drawing.AxisStyleType.GRID,
            drawing.AxisOrientation.VERTICAL,
        )
        marker_style = drawing.UnitMarkerStyle(ystyle, xstyle)
    else:
        marker_style = None
    camera_boundary_stroke = drawing.Stroke(
        stroke_width=1,
        stroke_color=color,
        stroke_dash_style=drawing.LineDashStyle.SOLID,
    )
    camera_boundary_style = drawing.CameraBoundaryStyle(camera_boundary_stroke, True, True, True, True)
    camera_style = drawing.CameraStyle(camera_boundary_style, marker_style)
    return drawing.DrawnCamera(boundary_top, boundary_bottom, boundary_left, boundary_right, camera_style)


def do_generate(parsed_args, rng, bn):
    for sample_num in range(parsed_args.num_samples):
        do_generate_single(parsed_args, rng, bn, sample_num)


def do_generate_single(parsed_args, rng, bn, sample_num):
    sample_set = bn.new_sample_forward(rng)
    c_mat = sample_set.c_mat
    # Images
    # the second dimension represents each type of image for a given timestamp.
    # we generate three different images, one with the actual stick's coordinates, one with the noisy, observed
    # coordinates, and one with the two overlayed.
    # index 0 is actual
    # index 1 is observed
    # index 2 is both overlayed
    ims = np.zeros((parsed_args.num_ims, 3, parsed_args.im_h, parsed_args.im_w, 3))
    im_drawer = build_stick_drawer()
    cam = drawing.StickDrawer.get_standard_camera_matrix(
        sample_set.im_w,
        sample_set.im_h,
        sample_set.c_t,
        sample_set.c_b,
        sample_set.c_l,
        sample_set.c_r,
        10,
    )
    cam_style = build_drawn_camera(
        sample_set.c_t,
        sample_set.c_b,
        sample_set.c_l,
        sample_set.c_r,
        np.array([0.0, 0.0, 1.0]),
        draw_ticks=True,
    )
    im_drawer.set_drawing_camera_matrix(cam)

    for im_idx in range(parsed_args.num_ims):
        act = sample_set.stick_states.as_lines(im_idx)
        # Observed endpoints in the sample set are not in world coordinates, since the noise is added in image space.
        # Thus, we must invert that transformation first.
        obs = sample_set.observed_endpoints[im_idx]
        obs_old_shape = obs.shape
        obs = np.reshape(obs, (obs_old_shape[0] * obs_old_shape[1], obs_old_shape[2]))
        obs = util.apply_homogeneous_transformation(linalg.inv(sample_set.c_mat), obs)
        obs = np.reshape(obs, obs_old_shape)

        im_drawer.set_image_matrix(ims[im_idx, 0])
        im_drawer.draw_camera(cam_style)
        im_drawer.draw_sticks(act, drawing.StickType.ACTUAL)

        im_drawer.set_image_matrix(ims[im_idx, 1])
        im_drawer.draw_camera(cam_style)
        im_drawer.draw_sticks(obs, drawing.StickType.OBSERVED)

        im_drawer.set_image_matrix(ims[im_idx, 2])
        im_drawer.draw_camera(cam_style)
        im_drawer.draw_sticks(act, drawing.StickType.ACTUAL)
        im_drawer.draw_sticks(obs, drawing.StickType.OBSERVED)

        drawing.save_matrix_image(
            ims[im_idx, 0],
            get_actual_image_filename(parsed_args.folder, sample_num, im_idx)
        )
        drawing.save_matrix_image(
            ims[im_idx, 1],
            get_observed_image_filename(parsed_args.folder, sample_num, im_idx)
        )
        drawing.save_matrix_image(
            ims[im_idx, 2],
            get_forward_sampling_overlayed_image_filename(parsed_args.folder, sample_num, im_idx)
        )
    with open(get_forward_sampling_json_filename(parsed_args.folder, sample_num), 'w') as f:
        json.dump(sample_set.to_dict(), f, sort_keys=True, indent=2)


def do_inference(parsed_args, rng, bn):
    for sample_num in range(parsed_args.num_samples):
        do_inference_single(parsed_args, rng, bn, sample_num)


def do_inference_single(parsed_args, rng, bn, sample_num):
    sample_set = None
    with open(get_forward_sampling_json_filename(parsed_args.folder, sample_num), 'r') as f:
        sample_set = stick_sample.StickSample.from_dict(bn.version, json.load(f))
    num_chains = mp.cpu_count()
    sampler = stick_mh_sampler.StickMultiMHSampler(
        bn,
        sample_set,
        parsed_args.num_burn_in_samples,
        parsed_args.num_inference_samples,
        num_chains,  # My machine has 12 cores, so that seems like a good amount for multi-chaining.
    )
    # Start each chain off with a different seed value
    rngs = [None] * num_chains
    for i in range(num_chains):
        rngs[i] = random.RandomState(rng.randint(0xffffffff))
    sampler.run_all(rngs)
    saved_samples = sampler.saved_samples
    best_sample = sampler.best_sample

    xs = []
    ys = []
    for cur_sample_idx in range(len(saved_samples)):
        xs += [saved_samples[cur_sample_idx].s_0_p_x]
        ys += [saved_samples[cur_sample_idx].s_0_p_y]
    print('best_metropolis_sample.s_0_p_x = {}'.format(best_sample.s_0_p_x))
    print('best_metropolis_sample.s_0_p_y = {}'.format(best_sample.s_0_p_y))
    print('best_metropolis_sample.log_prob = {}'.format(best_sample.log_prob))
    plt.figure()
    plt.scatter(xs, ys)
    plt.savefig(get_inference_plot_filename(parsed_args.folder, sample_num))
    saved_samples_dicts = [None] * len(saved_samples)
    for i in range(len(saved_samples)):
        saved_samples_dicts[i] = saved_samples[i].to_dict()
    with open(get_inference_json_filename(parsed_args.folder, sample_num), 'w') as f:
        json.dump(saved_samples_dicts, f, sort_keys=True, indent=2)

    sample_set_inv_c_mat = linalg.inv(sample_set.c_mat)
    # These are the images that will be output
    # Index i, 0 will just be the plain inferred result
    # Index i, 1 will be the inferred result overlayed with the the other two results
    ims = np.zeros((sample_set.num_ims, 2, sample_set.im_h, sample_set.im_w, 3))
    im_drawer = build_stick_drawer()
    c_t_max = max(sample_set.c_t, best_sample.c_t)
    c_b_min = min(sample_set.c_b, best_sample.c_b)
    c_l_min = min(sample_set.c_l, best_sample.c_l)
    c_r_max = max(sample_set.c_r, best_sample.c_r)
    cam = drawing.StickDrawer.get_standard_camera_matrix(
        sample_set.im_w,
        sample_set.im_h,
        c_t_max,
        c_b_min,
        c_l_min,
        c_r_max,
        10,
    )
    cam_style_common = build_drawn_camera(
        c_t_max,
        c_b_min,
        c_l_min,
        c_r_max,
        np.array([0.0, 0.0, 1.0]),
        draw_ticks=True,
    )
    cam_style_actual = build_drawn_camera(
        sample_set.c_t,
        sample_set.c_b,
        sample_set.c_l,
        sample_set.c_r,
        np.array([0.0, 0.6, 0.0]),
    )
    cam_style_inferred = build_drawn_camera(
        best_sample.c_t,
        best_sample.c_b,
        best_sample.c_l,
        best_sample.c_r,
        np.array([0.6, 0.0, 0.6]),
    )
    im_drawer.set_drawing_camera_matrix(cam)
    for im_idx in range(sample_set.num_ims):
        act = sample_set.stick_states.as_lines(im_idx)

        obs = sample_set.observed_endpoints[im_idx]
        obs_old_shape = obs.shape
        obs = np.reshape(obs, (obs_old_shape[0] * obs_old_shape[1], obs_old_shape[2]))
        obs = util.apply_homogeneous_transformation(sample_set_inv_c_mat, obs)
        obs = np.reshape(obs, obs_old_shape)

        inf = best_sample.stick_states.as_lines(im_idx)

        im_drawer.set_image_matrix(ims[im_idx, 0])
        im_drawer.draw_camera(cam_style_common)
        im_drawer.draw_camera(cam_style_inferred)
        im_drawer.draw_sticks(inf, drawing.StickType.INFERRED)

        im_drawer.set_image_matrix(ims[im_idx, 1])
        im_drawer.draw_camera(cam_style_common)
        im_drawer.draw_camera(cam_style_actual)
        im_drawer.draw_camera(cam_style_inferred)
        im_drawer.draw_sticks(act, drawing.StickType.ACTUAL)
        im_drawer.draw_sticks(obs, drawing.StickType.OBSERVED)
        im_drawer.draw_sticks(inf, drawing.StickType.INFERRED)

        drawing.save_matrix_image(
            ims[im_idx, 0],
            get_inference_image_filename(parsed_args.folder, sample_num, im_idx)
        )
        drawing.save_matrix_image(
            ims[im_idx, 1],
            get_inference_overlayed_image_filename(parsed_args.folder, sample_num, im_idx)
        )
do_inference_single.inferred_color = np.array([1.0, 0.0, 1.0])


def main():
    parsed_args = do_parse_args()

    rng = random.RandomState(parsed_args.seed)

    bn = stick_bayes_net.BayesNet(parsed_args.num_ims, parsed_args.im_w, parsed_args.im_h)

    if parsed_args.generate:
        do_generate(parsed_args, rng, bn)
    if parsed_args.inference:
        do_inference(parsed_args, rng, bn)


if __name__ == '__main__':
    main()
