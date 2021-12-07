#include "util.hpp"

namespace stick_2d_frac { namespace util {

std::unique_ptr<kjb::Matrix> homo_col_vecs_to_non_homo_col_vecs(const kjb::Matrix & in)
{
    std::unique_ptr<kjb::Matrix> r = std::unique_ptr<kjb::Matrix>(
            new kjb::Matrix(in.get_num_rows() - 1, in.get_num_cols()));
    kjb::Vector last_row = in.get_row(in.get_num_rows() - 1);
    for(int i = 0; i < in.get_num_rows() - 1; i++)
    {
        for(int j = 0; j < in.get_num_cols(); j++)
        {
            (*r)(i, j) = in(i, j) / in(in.get_num_rows() - 1, j);
        }
    }
    return std::move(r);
}

std::unique_ptr<kjb::Matrix> non_homo_col_vecs_to_homo_col_vecs(const kjb::Matrix & in)
{
    std::unique_ptr<kjb::Matrix> r = std::unique_ptr<kjb::Matrix>(new kjb::Matrix(in));
    (*r).resize(in.get_num_rows() + 1, in.get_num_cols(), 1.0);
    return std::move(r);
}

std::unique_ptr<kjb::Matrix> stick_length_to_local_endpoints_homo(double len)
{
    std::unique_ptr<kjb::Matrix> r = std::unique_ptr<kjb::Matrix>(new kjb::Matrix(3, 2));
    (*r)(0, 0) = -len / 2.0; (*r)(0, 1) = len / 2.0;
    (*r)(1, 0) =        0.0; (*r)(1, 1) =       0.0;
    (*r)(2, 0) =        1.0; (*r)(2, 1) =       1.0;
    return std::move(r);
}

double local_endpoints_homo_to_stick_length(const kjb::Matrix & in)
{
    return in(0, 1) * 2.0;
}

}}
