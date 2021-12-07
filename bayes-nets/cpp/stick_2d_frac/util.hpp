#ifndef UTIL_HPP
#define UTIL_HPP

#include <memory>

#include <m_cpp/m_matrix.h>

namespace stick_2d_frac { namespace util {

std::unique_ptr<kjb::Matrix> homo_col_vecs_to_non_homo_col_vecs(const kjb::Matrix & in);
std::unique_ptr<kjb::Matrix> non_homo_col_vecs_to_homo_col_vecs(const kjb::Matrix & in);

std::unique_ptr<kjb::Matrix> stick_length_to_local_endpoints_homo(double len);
double local_endpoints_homo_to_stick_length(const kjb::Matrix & in);

}}

#endif // UTIL_HPP
