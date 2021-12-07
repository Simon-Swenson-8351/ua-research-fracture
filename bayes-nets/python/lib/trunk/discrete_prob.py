import numpy as np

import discretization
import util


# pmf being an array probability masses. This function returns the corresponding 
# index that was chosen.
# rng being a random.Random object
def sample_discrete_distribution(pmf_table, rng):
    rand_val = rng.random()
    accum = 0
    for i in np.ndindex(pmf_table.shape):
        accum += pmf_table[i]
        if accum > rand_val:
            break
    return i


# Represents an n-dimensional functions whose output values are suitable for 
# input values of another node. This means they must be evenly spaced, within a 
# certain range.
class DiscretePMF(discretization.DiscretizedFunction):

    # pmf_table, being a table of probability masses, is assumed to be normalized
    # index_to_value_list being a list of size n, where n is the number of 
    # inputs to the function, where each element is a sorted list of values.
    def __init__(self, pmf_table, index_to_input_list):
        discretization.DiscretizedFunction.__init__(self, pmf_table, index_to_input_list)

    @classmethod
    def from_DiscretizedFunction(cls, discretized_fn):
        tab = discretized_fn.output_table
        tab = tab / np.sum(tab)
        return cls(tab, discretized_fn.index_to_input)

    @classmethod
    def from_pdf(cls, fn, samples):
        return cls.from_DiscretizedFunction(
            discretization.DiscretizedFunction.from_nd_fn(
                fn,
                (),
                samples
            ).integrate()
        )

    @classmethod
    def from_scipy_pmf(cls, scipy_pmf, samples):
        tab = np.zeros(util.get_shape_from_ragged_array(samples))
        for i in np.ndindex(tab.shape):
            inputs = util.get_values_from_ragged_array(samples, i)
            tab[i] = scipy_pmf(inputs)
        tab = tab / np.sum(tab)
        return cls(tab, samples)

    def sample(self, rng):
        result = [None] * len(self.index_to_input)
        indices = sample_discrete_distribution(self.output_table, rng)
        for i in range(len(indices)):
            result[i] = self.index_to_input[i][indices[i]]
        return {'probability': self.output_table[tuple(indices)], 'index': indices, 'output': result}


# This class handles organization of a CPD table, various parents and their 
# possible values. It also works for PMF samples of single variables.
class DiscreteCPD(discretization.DiscretizedFunction):

    # cpd_table being an n + 1 dimensional table, where n is the number of 
    # parents.
    # dimension_to_parent_name being an n-entry list of strings, relating each 
    # dimension of cpd_table (index) to the name of a random variable, one of this 
    # variable's parents.
    # indices_to_values being a n + 1-entry ragged list, where the first index is the 
    # dimension into the CPD table of the corresponding variable. The second 
    # dimension relates 
    # indices of that variable to its actual values. The n + 1th entry represents 
    # the output values for this discrete CPD.
    def __init__(self, cpd_table, index_to_input_list):
        discretization.DiscretizedFunction.__init__(self, cpd_table, index_to_input_list)

    @classmethod
    def from_DiscretizedFunction(cls, discretized_fn):
        tbl = discretized_fn.output_table
        tbl = tbl / np.reshape(np.sum(tbl, axis=tbl.ndim - 1), tuple(list(tbl.shape[:-1]) + [1]))
        return cls(tbl, discretized_fn.index_to_input)

    @classmethod
    def from_pdf(cls, fn, samples):
        return cls.from_DiscretizedFunction(
            discretization.DiscretizedFunction.from_nd_fn(
                fn,
                (),
                samples
            ).integrate()
        )

    # pdf_factory takes values for variables upon which this random variable is conditioned and produces a pdf.
    # samples is an n-entry list, where the first n-1 entries are random variables upon which this random variable is
    # conditioned.
    @classmethod
    def from_pdf_factory(cls, pdf_factory, samples):
        # Have to deal with integration eating a sample point.
        output_table_shape = list(util.get_shape_from_ragged_array(samples))
        # Not exactly sure why I need the int cast, but, otherwise, it converts the last entry to a floating point.
        output_table_shape = tuple(output_table_shape[:-1] + [int(output_table_shape[-1] - 1)])
        output_table = np.zeros(output_table_shape)

        input_shape = output_table_shape[:-1]

        for i in np.ndindex(input_shape):
            conditioned_assignment = util.get_values_from_ragged_array(samples, i)
            cpd = pdf_factory(conditioned_assignment)
            discretized_cpd = discretization.DiscretizedFunction.from_nd_fn(cpd, (), [samples[-1]]).integrate()
            discretized_cpd_table = discretized_cpd.output_table
            discretized_cpd_table /= np.sum(discretized_cpd_table)
            output_table[i] = discretized_cpd_table
        return cls(output_table, samples[:-1] + [samples[-1][:-1]])

    # conditioned_values being a list of real numbers, the values upon which this CPD is to be clamped.
    def sample(self, conditioned_values, rng):
        clamped_cpd = self.get_clamped_cpd(conditioned_values)
        sample_idx = sample_discrete_distribution(clamped_cpd, rng)
        return {'probability': clamped_cpd[sample_idx], 'index': sample_idx, 'output': self.index_to_input[self.output_table.ndim - 1][sample_idx]}

    # conditioned_values being a list of real numbers
    def get_clamped_cpd(self, conditioned_values):
        cpd_indices = [None] * (self.output_table.ndim - 1)
        for i in range(len(conditioned_values)):
            cpd_indices[i] = self.input_to_index[i][conditioned_values[i]]

        return self.output_table[tuple(cpd_indices)]
