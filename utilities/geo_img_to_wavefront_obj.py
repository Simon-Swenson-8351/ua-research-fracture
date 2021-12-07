# Arguments:
#   1 - Input .img file. Best if it includes some factors of 2. You need at 
#       least max(Chunk level, Geometry collpse), or some truncating will occur.
#   2 - Chunk level
#   3 - Chunk x (0 index)
#   4 - Chunk y (0 index)
#   5 - Geometry collapse. The end mesh will be roughly 2^n times rougher in 
#       each x, y direction.
#   6 - Power of 2 hint. For a given project, should be the maximum ever of 
#       chunk level or geometry collapse. 
#   7 - latitude of the elevation data. Used to calculate meters between data points.
#   8 - decimal degrees latitude difference between data points
#   9 - decimal degrees longitude difference between data points

import sys
from osgeo import gdal
import numpy
import math

arg_img_filename = sys.argv[1]
arg_chunk_level = int(sys.argv[2])
arg_chunk_x = int(sys.argv[3])
arg_chunk_y = int(sys.argv[4])
arg_geometry_collapse = int(sys.argv[5])
arg_power_hint = int(sys.argv[6])
arg_latitude = float(sys.argv[7])
arg_decimal_lat_per_data_point = float(sys.argv[8])
arg_decimal_long_per_data_point = float(sys.argv[9])

earth_radius = 6378137.0
earth_circumference = 2 * math.pi * earth_radius

ns_meters_per_data_point = arg_decimal_lat_per_data_point * earth_circumference / 360
# Need to find the circumference at the latitude given. Hence use cos.
ew_meters_per_data_point = arg_decimal_long_per_data_point * (earth_circumference * math.cos(math.radians(arg_latitude))) / 360

power_hint = 2 ** arg_power_hint
stride = 2 ** arg_geometry_collapse
num_chunks = 2 ** arg_chunk_level

elevation_data = gdal.Open(arg_img_filename)

elevation_data_array = elevation_data.ReadAsArray()
x_extent_new = (elevation_data_array.shape[1] // power_hint) * power_hint
y_extent_new = (elevation_data_array.shape[0] // power_hint) * power_hint

x_chunk_size = x_extent_new // num_chunks
y_chunk_size = y_extent_new // num_chunks

x_start = arg_chunk_x * x_chunk_size
x_end = (arg_chunk_x + 1) * x_chunk_size
y_start = arg_chunk_y * y_chunk_size
y_end = (arg_chunk_y + 1) * y_chunk_size

# vertices
# j will handle the augmentation vertices, so it needs double the length.
# That's the stride / 2 term.
# Want our vertices to be printed out like so, for easy indexing later:
# 01  02  03  04
#   05  06  07
# 08  09  10  11
#
# We end up thinking about it like this, in terms of implementation:
# 01 02 03 04
# 05 06 07 XX
# 08 09 10 11
# 12 13 14 XX
# 15 16 17 18
even = True
for j in numpy.arange(y_start, y_end + stride, stride / 2):
    for i in range(x_start, x_end + stride, stride):
        # Reverse direction of j, otherwise the model will need to be flipped.
        # y_loc = -j + y_start + y_end
        if even:
            print('v ' + str(i * ew_meters_per_data_point) + ' ' + str(elevation_data_array[int(j), i]) + ' ' + str(j * ns_meters_per_data_point))
        elif i != x_end:
            cur_elev = (elevation_data_array[int(j - stride / 2), i] \
                + elevation_data_array[int(j - stride / 2), i + stride] \
                + elevation_data_array[int(j + stride / 2), i] \
                + elevation_data_array[int(j + stride / 2), i + stride]) \
                / 4
            print('v ' + str((i + stride / 2) * ew_meters_per_data_point) + ' ' + str(float(cur_elev)) + ' ' + str(j * ns_meters_per_data_point))
    even = not even

print('')

for j in range(y_start + stride, y_end + stride, stride):
    for i in range(x_start + stride, x_end + stride, stride):
        # If we'd like to think of the i, j coordinates as starting at (0, 0) on 
        # the top-left, then going (1, 0), across, etc. instead of (512, 0) to 
        # (528, 0), etc this transform is needed. It helps later.
        j_local = (j - y_start) // stride
        i_local = (i - x_start) // stride
        points_across = len(range(x_start, x_end + stride, stride))
        vertex_idx_bot_right = (2 * points_across - 1) * j_local + i_local + 1
        vertex_idx_top_left = vertex_idx_bot_right - 2 * points_across
        vertex_idx_top_right = vertex_idx_top_left + 1
        vertex_idx_center = vertex_idx_bot_right - points_across
        vertex_idx_bot_left = vertex_idx_bot_right - 1
        # Top
        print('f ' + str(vertex_idx_top_left) + ' ' + str(vertex_idx_center) + ' ' + str(vertex_idx_top_right))
        # Right
        print('f ' + str(vertex_idx_top_right) + ' ' + str(vertex_idx_center) + ' ' + str(vertex_idx_bot_right))
        # Bottom
        print('f ' + str(vertex_idx_bot_right) + ' ' + str(vertex_idx_center) + ' ' + str(vertex_idx_bot_left))
        # Left
        print('f ' + str(vertex_idx_bot_left) + ' ' + str(vertex_idx_center) + ' ' + str(vertex_idx_top_left))