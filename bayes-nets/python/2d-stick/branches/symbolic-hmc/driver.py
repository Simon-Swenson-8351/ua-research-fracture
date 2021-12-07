import argparse
import os
import json
import pickle

import numpy as np
import matplotlib.pyplot as plt

import util
import drawing

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
        default=640,
        type=int,
        help='The image width of the samples to generate'
    )
    parser.add_argument(
        '--im_h',
        default=480,
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
        default=10,
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


def get_forward_sampling_pickle_filename(sample_folder, sample_num):
    dirname = os.path.join(sample_folder, '{:06}'.format(sample_num), 'forward-sampling')
    os.makedirs(dirname, exist_ok=True)
    return os.path.join(dirname, '{:06}-forward-sample.pkl'.format(sample_num))


def get_inference_pickle_filename(sample_folder, sample_num):
    dirname = os.path.join(sample_folder, '{:06}'.format(sample_num), 'inference')
    os.makedirs(dirname, exist_ok=True)
    return os.path.join(dirname, '{:06}-inference.pkl'.format(sample_num))


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
            stroke_color=build_stick_drawer.actual_color,
            stroke_dash_style=drawing.LineDashStyle.SOLID,
        ),
        observed_stick_style=drawing.Stroke(
            stroke_width=1,
            stroke_color=build_stick_drawer.observed_color,
            stroke_dash_style=drawing.LineDashStyle.SOLID,
        ),
        inferred_stick_style=drawing.Stroke(
            stroke_width=1,
            stroke_color=build_stick_drawer.inferred_color,
            stroke_dash_style=drawing.LineDashStyle.SOLID,
        ),
    )
build_stick_drawer.actual_color = np.array([0.0, 1.0, 0.0])
build_stick_drawer.observed_color = np.array([1.0, 0.0, 0.0])
build_stick_drawer.inferred_color = np.array([1.0, 0.0, 1.0])


def build_drawn_camera(boundary_top, boundary_bottom, boundary_left, boundary_right, color, draw_ticks=False):
    grid_line_stroke = drawing.Stroke(
        stroke_width=1,
        stroke_color=color,
        stroke_dash_style=drawing.LineDashStyle.DOTTED,
    )
    if draw_ticks:
        xstyle = drawing.AxisStyle(
            grid_line_stroke,
            1.0,
            drawing.AxisStyleType.GRID,
            drawing.AxisOrientation.HORIZONTAL,
        )
        ystyle = drawing.AxisStyle(
            grid_line_stroke,
            1.0,
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


def do_generate(parsed_args, rng):
    for sample_num in range(parsed_args.num_samples):
        do_generate_single(parsed_args, rng, sample_num)


def do_generate_single(parsed_args, rng, sample_num):
    sample_set = stick_sample.StickSample(parsed_args.num_ims, parsed_args.im_w, parsed_args.im_h, rng)
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
        int(sample_set.im_w),
        int(sample_set.im_h),
        float(sample_set.c_t),
        float(sample_set.c_b),
        float(sample_set.c_l),
        float(sample_set.c_r),
        10,
    )
    cam_style = build_drawn_camera(
        float(sample_set.c_t),
        float(sample_set.c_b),
        float(sample_set.c_l),
        float(sample_set.c_r),
        np.array([0.0, 0.0, 1.0]),
        draw_ticks=True,
    )
    im_drawer.set_drawing_camera_matrix(cam)

    for im_idx in range(parsed_args.num_ims):
        stick_pts_act = sample_set.stick_tree.endpoint_world_coords_act(im_idx)
        stick_pts_obs = sample_set.stick_tree.endpoint_world_coords_obs(im_idx)
        # Draw the line(s)
        im_drawer.set_image_matrix(ims[im_idx, 0])
        im_drawer.draw_camera(cam_style)
        im_drawer.draw_sticks(stick_pts_act, drawing.StickType.ACTUAL)

        im_drawer.set_image_matrix(ims[im_idx, 1])
        im_drawer.draw_camera(cam_style)
        im_drawer.draw_sticks(stick_pts_obs, drawing.StickType.OBSERVED)

        im_drawer.set_image_matrix(ims[im_idx, 2])
        im_drawer.draw_camera(cam_style)
        im_drawer.draw_sticks(stick_pts_act, drawing.StickType.ACTUAL)
        im_drawer.draw_sticks(stick_pts_obs, drawing.StickType.OBSERVED)
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
    sample_set.continuous_log_prob()
    with open(get_forward_sampling_pickle_filename(parsed_args.folder, sample_num), 'wb') as f:
        pickle.dump(sample_set, f)


def do_inference(parsed_args, rng):
    for sample_num in range(parsed_args.num_samples):
        do_inference_single(parsed_args, rng, bn, sample_num)


def do_inference_single(parsed_args, rng, sample_num):
    sample_set = None
    with open(get_forward_sampling_pickle_filename(parsed_args.folder, sample_num), 'rb') as f:
        sample_set = stick_sample.StickSample.from_dict(bn.version, json.load(f))
    sampler = stick_mh_sampler.StickMHSampler(
        bn,
        sample_set,
        parsed_args.num_burn_in_samples,
        parsed_args.num_inference_samples
    )
    sampler.run_all(rng)
    saved_samples = sampler.saved_samples
    best_sample = sampler.best_sample

    xs = []
    ys = []
    for cur_sample_idx in range(len(saved_samples)):
        xs += [saved_samples[cur_sample_idx].s_0_p_x]
        ys += [saved_samples[cur_sample_idx].s_0_p_y]
    print('best_metropolis_sample.s_0_p_x = {}'.format(best_sample.s_0_p_x))
    print('best_metropolis_sample.s_0_p_y = {}'.format(best_sample.s_0_p_y))
    print('best_metropolis_sample.log_prob = {}'.format(best_sample.continuous_log_prob))
    plt.figure()
    plt.scatter(xs, ys)
    plt.savefig(get_inference_plot_filename(parsed_args.folder, sample_num))
    saved_samples_dicts = [None] * len(saved_samples)
    for i in range(len(saved_samples)):
        saved_samples_dicts[i] = saved_samples[i].to_dict()
    with open(get_inference_json_filename(parsed_args.folder, sample_num), 'w') as f:
        json.dump(saved_samples_dicts, f, sort_keys=True, indent=2)

    # Do some calculations for drawing the inferred sticks
    c_mat = best_sample.c_mat
    # These are the images that will be output
    # Index i, 0 will just be the plain inferred result
    # Index i, 1 will be the inferred result overlayed with the the other two results
    ims = np.zeros((sample_set.num_ims, 2, sample_set.im_h, sample_set.im_w, 3))
    for im_idx in range(sample_set.num_ims):
        ims[im_idx, 1] = util.load_matrix_image(get_forward_sampling_overlayed_image_filename(parsed_args.folder, sample_num, im_idx))
        stick_pts = sample_set.stick_states.as_lines(im_idx)
        old_shape = stick_pts.shape
        # Reshape to one point per row
        stick_pts = np.reshape(
            stick_pts,
            (stick_pts.shape[0] * stick_pts.shape[1], stick_pts.shape[2])
        )
        stick_pxs = util.apply_homogeneous_transformation(
                c_mat,
                stick_pts)
        stick_pxs = np.reshape(stick_pxs, old_shape)
        for stick_idx in range(stick_pxs.shape[0]):
            util.draw_line_non_homo(ims[im_idx, 0], stick_pxs[stick_idx], do_inference_single.inferred_color)
            util.draw_line_non_homo(ims[im_idx, 1], stick_pxs[stick_idx], do_inference_single.inferred_color)
        util.save_matrix_image(
            ims[im_idx, 0],
            get_inference_image_filename(parsed_args.folder, sample_num, im_idx)
        )
        util.save_matrix_image(
            ims[im_idx, 1],
            get_inference_overlayed_image_filename(parsed_args.folder, sample_num, im_idx)
        )


def main():
    parsed_args = do_parse_args()

    rng = np.random.RandomState(parsed_args.seed)

    if parsed_args.generate:
        do_generate(parsed_args, rng)
    if parsed_args.inference:
        do_inference(parsed_args, rng)


if __name__ == '__main__':
    main()
