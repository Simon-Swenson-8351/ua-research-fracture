import numpy as np
import sympy as sp


def get_shape_from_ragged_array(ragged_array):

    result = np.zeros(len(ragged_array), np.uint64)
    for i in range(len(ragged_array)):
        result[i] = len(ragged_array[i])
    return tuple(result)


# If indices is not the same length as ragged_array, the indices are assumed to correspond to the first k dimensions,
# that is, the first k lists in ragged_array.
def get_values_from_ragged_array(ragged_array, indices):
    result = np.zeros(len(indices))
    for i in range(len(indices)):
        result[i] = ragged_array[i][indices[i]]
    return result


# non_homo_coords should be an nxm array, n points in R^m.
def non_homo_coords_to_homo_coords(non_homo_coords, sample_dim=0):
    if len(non_homo_coords.shape) == 1:
        # Only one coordinate
        new_shape = (non_homo_coords.shape[0] + 1)
        result = np.zeros(new_shape)
        result[:-1] = non_homo_coords
        result[-1] = 1
    elif sample_dim == 0:
        new_shape = (non_homo_coords.shape[0], non_homo_coords.shape[1] + 1)
        result = np.zeros(new_shape)
        result[:, :-1] = non_homo_coords
        result[:, -1] = 1
    else:
        new_shape = (non_homo_coords.shape[0] + 1, non_homo_coords.shape[1])
        result = np.zeros(new_shape)
        result[:-1, :] = non_homo_coords
        result[-1, :] = 1
    return result


def non_homo_vec_expr_to_homo_vec_expr(non_homo_vec_expr, sample_dim=0):
    r_l = non_homo_vec_expr.tolist()
    if sample_dim == 0:
        for i in range(len(r_l)):
            r_l[i].append(1)
    elif sample_dim == 1:
        r_l.append([1] * non_homo_vec_expr.shape[1])
    else:
        _wrong_sample_dim(sample_dim)
    return sp.Matrix(r_l)


# homo_coords should be an nx(m+1) array, representing n points which will be converted to vectors in R^m.
def homo_coords_to_non_homo_coords(homo_coords, sample_dim=0):
    if len(homo_coords.shape) == 1:
        result = np.copy(homo_coords[:-1])
        result = result / homo_coords[-1]
    elif sample_dim == 0:
        result = np.copy(homo_coords[:, :-1])
        result = result / homo_coords[:, -1]
    else:
        result = np.copy(homo_coords[:-1, :])
        result = result / homo_coords[-1, :]
    return result


def homo_vec_expr_to_non_homo_vec_expr(homo_vec_expr, sample_dim=0):
    if sample_dim == 0:
        r_l = [[None] * (homo_vec_expr.shape[1] - 1)] * homo_vec_expr.shape[0]
        for i in range(homo_vec_expr.shape[0]):
            for j in range(homo_vec_expr.shape[1] - 1):
                r_l[i][j] = homo_vec_expr[i, j] / homo_vec_expr[i, homo_vec_expr.shape[1] - 1]
    elif sample_dim == 1:
        r_l = [[None] * homo_vec_expr.shape[1]] * (homo_vec_expr.shape[0] - 1)
        for i in range(homo_vec_expr.shape[0] - 1):
            for j in range(homo_vec_expr.shape[1]):
                r_l[i][j] = homo_vec_expr[i, j] / homo_vec_expr[homo_vec_expr.shape[0] - 1, j]
    else:
        _wrong_sample_dim(sample_dim)
    return sp.Matrix(r_l)


# The input is a n x m matrix. If sample_dim is 0, then we interpret n as the number of samples and m as the dimension
# of each sample. If sample_dim is 1, then vise versa.
def apply_homogeneous_transformation(matrix, non_homo_coords, sample_dim=0):
    homo_coords = non_homo_coords_to_homo_coords(non_homo_coords, sample_dim=sample_dim)
    if sample_dim == 0:
        homo_coords = np.transpose(homo_coords)
    result = homo_coords_to_non_homo_coords(matrix @ homo_coords, sample_dim=1)
    if sample_dim == 0:
        result = np.transpose(result)
    return result


def apply_homogeneous_transformation_sym(matrix_expr, non_homo_vec_expr, sample_dim=0):
    homo_vec_expr = non_homo_vec_expr_to_homo_vec_expr(non_homo_vec_expr, sample_dim=sample_dim)
    if sample_dim == 0:
        homo_vec_expr = homo_vec_expr.T
    result = homo_vec_expr_to_non_homo_vec_expr(matrix_expr * homo_vec_expr, sample_dim=1)
    if sample_dim == 0:
        result = result.T
    return result


def _wrong_sample_dim(sample_dim):
    raise Exception('Expected sample_dim in {{0, 1}}, got {}', sample_dim)
