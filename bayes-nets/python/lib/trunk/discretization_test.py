import numpy as np
import scipy.stats as stats
import matplotlib.pyplot as plt
import discretization

if __name__ == '__main__':
    stddev = 0.125
    while stddev <= 10:
        num_samples = 16
        window = 4 * stddev
        step = 2 * window / num_samples
        inputs = [[-window, -1/2 * window, -1/4 * window, -1/8 * window, -1/16 * window, -1/32 * window, 0, 1/32 * window, 1/16 * window, 1/8 * window, 1/4 * window, 1/2 * window, window]]
        norm_pdf = stats.norm(scale=stddev).pdf
        discretized_norm_pdf = discretization.DiscretizedFunction.from_nd_fn(norm_pdf, inputs)
        # NOTE: The PMF looks weird because of the differing step sizes. Since 
        # the steps get closer together near the middle, their corresponding 
        # rectangles (masses) have a smaller area.
        discretized_norm_pmf = discretized_norm_pdf.integrate()

        plt.figure()
        plt.plot(discretized_norm_pdf.index_to_value[0], discretized_norm_pdf.function_table)
        plt.show()
        plt.figure()
        plt.plot(discretized_norm_pmf.index_to_value[0], discretized_norm_pmf.function_table)
        plt.show()

        inputs = [np.arange(-window, window + step / 2, step)]
        norm_pdf = stats.norm(scale = stddev).pdf
        discretized_norm_pdf = discretization.DiscretizedFunction.from_nd_fn(norm_pdf, inputs)
        discretized_norm_pmf = discretized_norm_pdf.integrate()

        plt.figure()
        plt.plot(discretized_norm_pdf.index_to_value[0], discretized_norm_pdf.function_table)
        plt.show()
        plt.figure()
        plt.plot(discretized_norm_pmf.index_to_value[0], discretized_norm_pmf.function_table)
        plt.show()

        print(discretized_norm_pmf.index_to_value)
        print(discretized_norm_pmf.closest_input_value([-2 * window]))
        print(discretized_norm_pmf.closest_input_value([0.0001]))
        print(discretized_norm_pmf.closest_input_value([1.0]))

        stddev *= 2
