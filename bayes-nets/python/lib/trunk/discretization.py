import numpy as np
import util


class DiscretizedFunction:

    # function_table being a regular grid of function values
    # index_to_arg_list being a list of size n, where n is the number of 
    # inputs to the function, where each element is a sorted list of values.
    def __init__(self, output_table, index_to_input_list):
        self.output_table = output_table
        self.index_to_input = index_to_input_list
        self.input_to_index = [None] * len(index_to_input_list)
        for i in range(len(index_to_input_list)):
            cur_idx_to_val = index_to_input_list[i]
            cur_val_to_idx = {}
            for j in range(len(cur_idx_to_val)):
                cur_val_to_idx[cur_idx_to_val[j]] = j
            self.input_to_index[i] = cur_val_to_idx

    # fn being a function that takes a n-dimensional array
    # output_shape being a tuple represeting the output space (e.g. (2, 3) means the output will be in R^2x3)
    # inputs being a ragged array representing samples along each
    # dimension.
    @classmethod
    def from_nd_fn(cls, fn, output_shape, inputs):

        input_shape = util.get_shape_from_ragged_array(inputs)
        total_shape = tuple(list(input_shape) + list(output_shape))
        outputs = np.zeros(total_shape)

        for i in np.ndindex(input_shape):
            outputs[i] = fn(util.get_values_from_ragged_array(inputs, i))

        return cls(outputs, inputs)

    # Rounds the proposal to the nearest valid input value
    def closest_input_value(self, proposed_input_value_list):
        # r is exclusive
        # since we're returning the closest value rather than exact value, some 
        # assumptions in a traditional binary search are wrong, and it gets a 
        # little more complicated. Specifically, we always need to ensure we include 
        # the midpoint in the next iteration, because val could be between the 
        # midpoint and the next value in the list.
        def binary_search_closest(cur_list, l, r, val):

            if r - l == 1:
                return l
            if r - l == 2:
                if val - cur_list[l] < cur_list[l + 1] - val:
                    return l
                else:
                    return l + 1
            mid = (l + r - 1) // 2
            if cur_list[mid] < val:
                # Take the upper half, including the midpoint
                return binary_search_closest(cur_list, mid, r, val)
            else: # cur_list[mid] >= val
                # Take the lower half, including the midpoint
                return binary_search_closest(cur_list, l, mid + 1, val)
            
        result = [None] * len(proposed_input_value_list)
        for i in range(len(proposed_input_value_list)):
            cur_value_list = self.index_to_input[i]
            result[i] = cur_value_list[binary_search_closest( \
                cur_value_list, \
                0, \
                len(cur_value_list), \
                proposed_input_value_list[i])]
        return result

    # input_value_list is assumed to be a list of known, good values already.
    # if not, call closest_input_value first.
    def evaluate(self, input_value_list):
        indices = [None] * len(input_value_list)
        for i in range(len(input_value_list)):
            indices[i] = self.input_to_index[i][input_value_list[i]]
        return self.output_table[tuple(indices)]

    # Returns another DiscretizedFunction, where each value is multiplied by the 
    # volume determined by the rectangle computed from the distance between 
    # adjacent values.
    # new_input_locations is one of 'low', 'center', 'high'.
    def integrate(self, new_input_locations='low'):

        # A volume is defined by two endpoints, so we change the output_shape to 
        # avoid a fencepost error
        output_shape = tuple(np.array(self.output_table.shape) - 1)
        outputs = np.zeros(output_shape)

        for i in np.ndindex(outputs.shape):
            # Assuming local linearity, we compute the function value as the 
            # arithmetic mean of all corner points of the volume.
            cur_slice = self.output_table
            cur_vol = 1.0
            for j in range(len(i)):
                cur_slice = cur_slice[i[j]:(i[j] + 2)]
                cur_vol *= self.index_to_input[j][i[j] + 1] - self.index_to_input[j][i[j]]
            outputs[i] = np.sum(cur_slice) / cur_slice.size * cur_vol

        inputs = [None] * len(self.index_to_input)
        for i in range(len(inputs)):
            cur_idx_to_val = np.array(self.index_to_input[i])
            if new_input_locations == 'center':
                inputs[i] = (cur_idx_to_val[:-1] + cur_idx_to_val[1:]) / 2
            elif new_input_locations == 'low':
                inputs[i] = cur_idx_to_val[:-1]
            elif new_input_locations == 'high':
                inputs[i] = cur_idx_to_val[1:]

        return DiscretizedFunction(outputs, inputs)
