# This script is designed to take in a single .gpx XML file (containing a stream 
# of GPS coordinates) and a directory containing pictures. The pictures are 
# assumed to be sorted based on the order in time in which they were taken. The 
# pictures are additionally assumed to be an equal distance apart. Finally, the 
# pictures are assumed to be fully within the start time and end time of the GPS 
# data stream.

# First argument: gpx file
# Second argument: directory containing only images
# Third argument: Time at which the first image appears. This is used to 
#   synchronize the two streams.
# Fourth argument: image filename extension.
# Fifth argument: direction. Either 1 or 0. As long as you're consistent, it 
#   doesn't matter whether 1 or 0 is north/east or south/west.

import os
import sys
import gpxpy
import gpxpy.gpx
import datetime
import json

gpx_filename = sys.argv[1]
image_dir = sys.argv[2]
camera_datetime = datetime.datetime.strptime(sys.argv[3], '%Y%m%d_%H%M%S')
image_extension = sys.argv[4]
direction = int(sys.argv[5])

gpx_file = None
with open(gpx_filename, 'r') as gpx_unparsed_file:
    gpx_file = gpxpy.parse(gpx_unparsed_file)

gps_coordinate_list = []

for track in gpx_file.tracks:
    for segment in track.segments:
        for point in segment.points:
            gps_coordinate_list.append(point)

image_filenames = []
for filename in os.listdir(image_dir):
    if filename.endswith('.' + image_extension):
        image_filenames.append(filename)
image_filenames.sort()

cur_time = camera_datetime
cur_gps_idx = 1
for filename in image_filenames:
    prev_gps = gps_coordinate_list[cur_gps_idx - 1]
    cur_gps = gps_coordinate_list[cur_gps_idx]
    while gps_coordinate_list[cur_gps_idx].time < cur_time:
        cur_gps_idx += 1
        prev_gps = gps_coordinate_list[cur_gps_idx - 1]
        cur_gps = gps_coordinate_list[cur_gps_idx]

    # Just copied wikipedia's formula for linear interpolation
    cur_lat = (prev_gps.latitude * (cur_gps.time - cur_time) + cur_gps.latitude * (cur_time - prev_gps.time))/(cur_gps.time - prev_gps.time)
    cur_long = (prev_gps.longitude * (cur_gps.time - cur_time) + cur_gps.longitude * (cur_time - prev_gps.time))/(cur_gps.time - prev_gps.time)
    json_obj = {'image_filename': filename, 'direction': direction, 'time': cur_time.strftime('%Y%m%d_%H%M%S'), 'latitude': cur_lat, 'longitude': cur_long}
    with open(os.path.join(image_dir, filename[:(-len(image_extension) - 1)] + '.json'), 'w') as json_file:
        print(json.dumps(json_obj, indent = '  ', separators=(',', ': ')))
        json.dump(json_obj, json_file, indent = '  ', separators=(',', ': '))
    cur_time = cur_time + datetime.timedelta(seconds = 1)