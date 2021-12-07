import enum
import math

import numpy as np
import PIL

import util


# The purpose of this file is to implement various drawing strategies for the sticks. For example, one might want to
# render sticks from multiple cameras to the same image, using a common scale and frame of reference. This should
# improve options and quality.
@enum.unique
class StickType(enum.Enum):
    ACTUAL = enum.auto()
    OBSERVED = enum.auto()
    INFERRED = enum.auto()


@enum.unique
class AxisOrientation(enum.Enum):
    HORIZONTAL = enum.auto()
    VERTICAL = enum.auto()


@enum.unique
class AxisStyleType(enum.Enum):
    TICK_MARK = enum.auto()
    GRID = enum.auto()


@enum.unique
class LineDashStyle(enum.Enum):
    SOLID = enum.auto()
    DOTTED = enum.auto()
    DASHED = enum.auto()


class Stroke:

    def __init__(self, stroke_width=1, stroke_color=np.array([1.0, 1.0, 1.0]), stroke_dash_style=LineDashStyle.SOLID):
        self.width = stroke_width
        self.color = stroke_color
        self.dash_style = stroke_dash_style
Stroke.default_stroke = Stroke()


# abstract base class
class StickDrawer:

    # *_color arguments should be either a 3-element list or a 3-element numpy vector representing RGB.
    def __init__(
            self,
            actual_stick_style,
            observed_stick_style,
            inferred_stick_style,
            initial_image_matrix=None,
            initial_drawing_camera_matrix=None,
    ):
        self.im_m = initial_image_matrix
        self.drawing_camera_m = initial_drawing_camera_matrix
        self.stick_styles = {
            StickType.ACTUAL: actual_stick_style,
            StickType.OBSERVED: observed_stick_style,
            StickType.INFERRED: inferred_stick_style,
        }

    def set_image_matrix(self, image_matrix):
        self.im_m = image_matrix

    def set_drawing_camera_matrix(self, drawing_camera_matrix):
        self.drawing_camera_m = drawing_camera_matrix

    # stick is assumed to be a 2x2 numpy matrix, in world coordinates, where each row is a point in R^2.
    def draw_stick(self, stick, stick_type):
        if self.drawing_camera_m is None or self.im_m is None:
            raise Exception('Set the image matrix and drawing camera before attempting to draw sticks.')
        draw_line(
            self.im_m,
            util.apply_homogeneous_transformation(self.drawing_camera_m, stick),
            self.stick_styles[stick_type]
        )

    # Assumed to be a nx2x2 numpy matrix, in world coordinates.
    def draw_sticks(self, sticks, stick_type):
        for stick_idx in range(sticks.shape[0]):
            self.draw_stick(sticks[stick_idx], stick_type)

    def draw_camera(self, camera_to_draw):
        if self.drawing_camera_m is None or self.im_m is None:
            raise Exception('Set the image matrix and drawing camera before attempting to draw a camera.')
        camera_to_draw.draw_self(self.im_m, self.drawing_camera_m)

    # Given the maximum and/or mimimum bounds of any number of cameras, computes a transformation that will allow any
    # such camera's bounds to be fully rendered within the image, with the added border.
    @classmethod
    def get_standard_camera_matrix(cls, im_w, im_h, max_c_t, min_c_b, min_c_l, max_c_r, min_border):
        px_per_unit = min(
            (im_w - 2 * min_border) / (max_c_r - min_c_l),
            (im_h - 2 * min_border) / (max_c_t - min_c_b)
        )
        return np.array([
                    [0,           -px_per_unit,  max_c_t * px_per_unit + min_border],
                    [px_per_unit,            0, -min_c_l * px_per_unit + min_border],
                    [0,                      0,                                   1]
        ])


# Represents a camera whose boundaries and/or units we would like to draw to the image.
class DrawnCamera:

    def __init__(self, boundary_top, boundary_bottom, boundary_left, boundary_right, style):

        self.boundary_top = boundary_top
        self.boundary_bottom = boundary_bottom
        self.boundary_left = boundary_left
        self.boundary_right = boundary_right

        self.boundary_top_left = np.array([self.boundary_left, self.boundary_top])
        self.boundary_top_right = np.array([self.boundary_right, self.boundary_top])
        self.boundary_bottom_left = np.array([self.boundary_left, self.boundary_bottom])
        self.boundary_bottom_right = np.array([self.boundary_right, self.boundary_bottom])

        self.style = style

    def draw_self(self, im_m, drawing_cam_m):
        self.style.draw_self(im_m, drawing_cam_m, self)


class CameraStyle:

    # *_color arguments should be either a 3-element list or a 3-element numpy vector representing RGB.
    def __init__(
            self,
            boundary_style,
            unit_marker_style,  # Set this to None to not draw any unit marks.
    ):
        self.boundary_style = boundary_style
        self.unit_marker_style = unit_marker_style

    def draw_self(self, im_m, drawing_cam_m, drawn_cam):
        if self.unit_marker_style is not None:
            self.unit_marker_style.draw_self(im_m, drawing_cam_m, drawn_cam)
        if self.boundary_style is not None:
            self.boundary_style.draw_self(im_m, drawing_cam_m, drawn_cam)


class CameraBoundaryStyle:

    def __init__(
            self,
            stroke,
            draw_top_boundary=False,
            draw_bottom_boundary=True,
            draw_left_boundary=True,
            draw_right_boundary=False,
    ):
        self.draw_top_boundary = draw_top_boundary
        self.draw_bottom_boundary = draw_bottom_boundary
        self.draw_left_boundary = draw_left_boundary
        self.draw_right_boundary = draw_right_boundary
        self.stroke = stroke

    def draw_self(self, im_m, drawing_cam_m, drawn_cam):
        corners = util.apply_homogeneous_transformation(
            drawing_cam_m,
            np.array([
                drawn_cam.boundary_top_left,
                drawn_cam.boundary_top_right,
                drawn_cam.boundary_bottom_right,
                drawn_cam.boundary_bottom_left,
            ])
        )
        if self.draw_top_boundary:
            draw_line(im_m, np.stack([corners[0], corners[1]]), self.stroke)
        if self.draw_right_boundary:
            draw_line(im_m, np.stack([corners[1], corners[2]]), self.stroke)
        if self.draw_bottom_boundary:
            draw_line(im_m, np.stack([corners[2], corners[3]]), self.stroke)
        if self.draw_left_boundary:
            draw_line(im_m, np.stack([corners[3], corners[0]]), self.stroke)


class UnitMarkerStyle:

    def __init__(
        self,
        vertical_axis_style,
        horizontal_axis_style
    ):
        self.vertical_axis_style = vertical_axis_style
        self.horizontal_axis_style = horizontal_axis_style

    def draw_self(self, im_m, drawing_cam_m, drawn_cam):
        if self.vertical_axis_style is not None:
            self.vertical_axis_style.draw_self(im_m, drawing_cam_m, drawn_cam)
        if self.horizontal_axis_style is not None:
            self.horizontal_axis_style.draw_self(im_m, drawing_cam_m, drawn_cam)


# Thos could easily be done using subclassing, but the two styles turned out to be similar enough that I just used a
# lambda instead.
class AxisStyle:

    def __init__(
            self,
            tick_stroke,
            unit_delta,
            axis_style_type,
            orientation,  # Orientation refers to the orientation of the AXIS, not the tick marks (the two are opposite)
    ):
        self.stroke = tick_stroke
        self.delta = unit_delta
        self.style_type = axis_style_type
        self.orientation = orientation

        # Setting self.get_endpoints as a lambda is similar to subclassing, but a bit simpler and less verbose in this
        # case.
        if self.style_type is AxisStyleType.TICK_MARK:
            if self.orientation is AxisOrientation.HORIZONTAL:
                self.get_endpoints = self.__get_endpoints_horizontal_tick_mark
            elif self.orientation is AxisOrientation.VERTICAL:
                self.get_endpoints = self.__get_endpoints_vertical_tick_mark
            else:
                raise Exception('Unsupported AxisOrientation {}'.format(orientation))
        elif self.style_type is AxisStyleType.GRID:
            if self.orientation is AxisOrientation.HORIZONTAL:
                self.get_endpoints = self.__get_endpoints_horizontal_grid
            elif self.orientation is AxisOrientation.VERTICAL:
                self.get_endpoints = self.__get_endpoints_vertical_grid
            else:
                raise Exception('Unsupported AxisOrientation {}'.format(orientation))
        else:
            raise Exception('Unsupported AxisStyleType {}'.format(axis_style_type))

    def draw_self(self, im_m, drawing_cam_m, drawn_cam):
        if self.orientation is AxisOrientation.HORIZONTAL:
            tick_range = np.arange(drawn_cam.boundary_left, drawn_cam.boundary_right, self.delta)
        elif self.orientation is AxisOrientation.VERTICAL:
            tick_range = np.arange(drawn_cam.boundary_left, drawn_cam.boundary_right, self.delta)
        else:
            raise Exception('Unsupported AxisOrientation {}'.format(self.orientation))
        # Horizontal lines
        for i in tick_range:
            pts = self.get_endpoints(drawn_cam, i)
            pxs = util.apply_homogeneous_transformation(drawing_cam_m, pts)
            draw_line(im_m, pxs, self.stroke)

    def __get_endpoints_horizontal_tick_mark(self, drawn_cam, position):
        return np.array([
            [position, drawn_cam.boundary_bottom - self.delta / 5],
            [position, drawn_cam.boundary_bottom + self.delta / 5],
        ])

    def __get_endpoints_vertical_tick_mark(self, drawn_cam, position):
        return np.array([
            [drawn_cam.boundary_left - self.delta / 5, position],
            [drawn_cam.boundary_left + self.delta / 5, position],
        ])

    def __get_endpoints_horizontal_grid(self, drawn_cam, position):
        return np.array([
            [position, drawn_cam.boundary_bottom],
            [position, drawn_cam.boundary_top],
        ])

    def __get_endpoints_vertical_grid(self, drawn_cam, position):
        return np.array([
            [drawn_cam.boundary_left, position],
            [drawn_cam.boundary_right, position],
        ])


# Coordinate is a column vector, row, then column
# color is a 3-entry numpy list
def draw_px(im, coord, color):
    if   coord[0] < 0 \
      or coord[0] >= im.shape[0] \
      or coord[1] < 0 \
      or coord[1] >= im.shape[1] \
      or color[0] < 0.0 \
      or color[0] > 1.0 \
      or color[1] < 0.0 \
      or color[1] > 1.0 \
      or color[2] < 0.0 \
      or color[2] > 1.0:

        return

    im[coord[0], coord[1]] = color


# endpoints should be a 2x2 array, one point (x, y), in image space, per row.
def draw_line(im, endpoints, stroke=Stroke.default_stroke):
    # Find out if the x distance or y distance is greater
    num_points = math.ceil(max(abs(endpoints[0, 0] - endpoints[1, 0]), abs(endpoints[0, 1] - endpoints[1, 1])) + 1)
    delta = (endpoints[1] - endpoints[0]) / num_points
    unit = 2 * stroke.width - 1
    for i in range(num_points):
        if stroke.dash_style is LineDashStyle.DOTTED and i % 2 * unit == 0:
            continue
        if stroke.dash_style is LineDashStyle.DASHED and i % 5 * unit < 2 * unit - 1:
            continue
        pt = np.round(endpoints[0] + i * delta).astype(np.uint64)
        x_range_min = min(max(int(pt[1]) - stroke.width + 1, 0), im.shape[1] - 1)
        x_range_max = min(max(int(pt[1]) + stroke.width, 0), im.shape[1] - 1)
        y_range_min = min(max(int(pt[0]) - stroke.width + 1, 0), im.shape[0] - 1)
        y_range_max = min(max(int(pt[0]) + stroke.width, 0), im.shape[0] - 1)
        x_range = range(x_range_min, x_range_max)
        y_range = range(y_range_min, y_range_max)
        im[y_range, x_range] = stroke.color


# Matrix is assumed to be row, column indexed
# Values are scaled from [0, 1] to 0..255. Values outside [0, 1] are clamped to
# the nearest value in the range.
def save_matrix_image(matrix, image_filename):
    scaled = matrix
    scaled[scaled < 0.0] = 0.0
    scaled[scaled > 1.0] = 1.0
    scaled = np.round(scaled * 255.0)
    scaled = scaled.astype(np.uint8)
    PIL.Image.fromarray(scaled).save(image_filename)


# The image is assumed to be 8-bit grayscale.
def load_matrix_image(image_filename):
    with open(image_filename, 'rb') as f:
        result = np.array(PIL.Image.open(f))
        return result
