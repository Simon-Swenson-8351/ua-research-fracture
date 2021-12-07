# Basically takes a set of cameras, a point cloud, and input images, and outputs 
# a set of sparse images with the pixel at each point in the point cloud colored 
# in based off of the input images.

# arg 1 - input camera file (.json list of camera matrices)
# arg 2 - input point cloud file (.obj)
# arg 3 - input image directory
# arg 4 - output image directory

import numpy as np
import scipy.misc as misc
import json
import sys
import os
from PIL import Image

class AffineScalar:

    def __init__(self, x1, x2, y1, y2):

        self.m = (y1 - y2) / (x1 - x2)
        self.b = y1 - self.m * x1

    def transform(self, x):
        return self.m * x + self.b

    def inverse(self, y):
        return (y - self.b) / self.m

if __name__ == '__main__':

    input_camera_filename = sys.argv[1]
    input_point_cloud_filename = sys.argv[2]
    input_image_dir = sys.argv[3]
    output_image_dir = sys.argv[4]

    cameras = None
    with open(input_camera_filename) as input_camera_file:
        cameras = json.load(input_camera_file)

    # I think y is interpreted as up-down here.
    points = []
    with open(input_point_cloud_filename) as input_point_cloud_file:
        for line in input_point_cloud_file:
            spl = line.split(' ')
            if spl[0] == 'v':
                point_to_add = []
                point_to_add.append(float(spl[1]))
                point_to_add.append(float(spl[2]))
                point_to_add.append(float(spl[3]))
                point_to_add.append(1.0)
                points.append(point_to_add)
    points = np.array(points).transpose()


    red_pixel = np.array([255, 0, 0], dtype=np.uint8)

    for camera in cameras:
        input_img = Image.open(os.path.join(input_image_dir, camera['imageFileName']))
        input_img = np.array(input_img)
        height_width_ratio = input_img.shape[0] / input_img.shape[1]
        output_img = np.copy(input_img)
        camera_matrix = np.array(camera['cameraMatrix'])

        xInputMin = -1.0
        xInputMax = 1.0
        yInputMin = -1.0 * height_width_ratio
        yInputMax = 1.0 * height_width_ratio
        xScalar = AffineScalar(xInputMin, xInputMax, input_img.shape[1], 0)
        yScalar = AffineScalar(yInputMin, yInputMax, 0, input_img.shape[0])

        # Turns out .abc matrices actually go from camera space to world space, 
        # so need to invert them!
        transformed_points = np.linalg.inv(camera_matrix) @ points
        transformed_points[0:2] /= transformed_points[2]
        for i in range(transformed_points.shape[1]):
            # Here, we assume that the camera screen space is between -1.0 and 1.0.
            # Vertices in front of the camera have a negative w value.
            if      transformed_points[0, i] > xInputMin \
                and transformed_points[0, i] <= xInputMax \
                and transformed_points[1, i] >= yInputMin \
                and transformed_points[1, i] < yInputMax \
                and transformed_points[2, i] < 0.0:

                px = int(xScalar.transform(transformed_points[0, i]))
                py = int(yScalar.transform(transformed_points[1, i]))
                output_img[py, px] = red_pixel
                if py > 0:
                    output_img[py - 1, px] = red_pixel
                if py < input_img.shape[0] - 1:
                    output_img[py + 1, px] = red_pixel
                if px > 0:
                    output_img[py, px - 1] = red_pixel
                if px < input_img.shape[1] - 1:
                    output_img[py, px + 1] = red_pixel
        Image.fromarray(output_img).save(os.path.join(output_image_dir, camera['imageFileName'][:-4] + '_output.png'))