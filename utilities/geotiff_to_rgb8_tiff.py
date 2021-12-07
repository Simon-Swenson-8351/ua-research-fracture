import sys
from PIL import Image
import numpy as np

# RGB order
input_channel_filenames = [sys.argv[1], sys.argv[2], sys.argv[3]]

out_filename = sys.argv[4]

out_file_ary = None
progress_max = None
progress = 0

for channel in range(3):
    with Image.open(input_channel_filenames[channel]) as input_channel_img:
        input_channel_ary = np.asarray(input_channel_img)
        if type(out_file_ary).__name__ != 'ndarray':
            out_file_ary = np.zeros(shape = (input_channel_ary.shape[0], input_channel_ary.shape[1], 3), dtype = np.uint8)
            progress_max = 3 * input_channel_ary.shape[0] * input_channel_ary.shape[1]
        for y in range(input_channel_ary.shape[0]):
            for x in range(input_channel_ary.shape[1]):
                out_file_ary[y, x, channel] = np.uint8(input_channel_ary[y, x] // 256)
                progress = progress + 1
                if (progress - 1) * 100 // progress_max != progress * 100 // progress_max:
                    print(str(progress * 100 // progress_max) + '%')

out_file_im = Image.fromarray(out_file_ary, mode = 'RGB')
out_file_im.save(out_filename)