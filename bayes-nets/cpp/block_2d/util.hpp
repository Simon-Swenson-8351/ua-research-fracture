#ifndef UTIL_HPP
#define UTIL_HPP

#include <memory>
#include <sstream>
#include <fstream>
#include <iostream>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <i_cpp/i_image.h>
#include <m_cpp/m_matrix_d.h>
#include <m_cpp/m_matrix_d.impl.h>
#include <prob_cpp/prob_distribution.h>

bool operator==(const kjb::Normal_distribution &a, const kjb::Normal_distribution &b);
std::istringstream &operator>>(std::istringstream &ss, std::pair<unsigned,unsigned> &o);

namespace fracture
{
namespace util
{

enum Data_image_type
{
    DIT_ACTUAL_GEOM,
    DIT_CENTER_OF_MASS,
    DIT_OBSERVED_GEOM,

    // INSERT OTHER ENTRIES ABOVE
    DIT_COUNT
};
const std::string DATA_IMAGE_DIRNS[DIT_COUNT] = {
    "im_act_geom",
    "im_act_com",
    "im_obs_geom"
};

enum Inference_image_type
{
    IIT_GEOM,
    IIT_CENTER_OF_MASS,
    IIT_COMBINED,

    // INSERT OTHER ENTRIES ABOVE
    IIT_COUNT
};
const std::string INFERENCE_IMAGE_DIRNS[IIT_COUNT] = {
    "im_geom",
    "im_com",
    "im_combined"
};

const std::string COMBINED_IMAGE_DIRNS = "combined";

enum Inference_type
{
    IT_METROPOLIS,
    IT_HMC,
    IT_GRAD_DESC,

    // INSERT OTHER ENTRIES ABOVE
    IT_COUNT
};
const std::string INFERENCE_TYPE_STR[IT_COUNT] = {
    "metropolis",
    "hmc",
    "gradient_descent"
};

enum Csv_record_attrs
{
    CRA_ROW_NUM,
    CRA_CAM_TOP,
    CRA_INIT_X_POS,
    CRA_INIT_Y_POS,
    CRA_INIT_WIDTH,
    CRA_INIT_HEIGHT,
    CRA_RIGHT_MOMENTUM,
    CRA_LEFT_ANGULAR_MOMENTUM,
    CRA_FRAC_LOC,
    CRA_LOG_PROB,
    CRA_ACCEPTED,
    CRA_FORWARD_SAMPLE_LOG_PROB,

    // INSERT OTHER ENTRIES ABOVE
    CRA_COUNT
};
const std::string CRA_STRS[CRA_COUNT] = {
    "Row Number",
    "Camera Top",
    "Initial X",
    "Initial Y",
    "Initial Width",
    "Initial Height",
    "Right Momentum",
    "Left Angular Momentum",
    "Fracture Location",
    "Log Probability",
    "Accepted?",
    "Forward Sample Log Probability"
};
// Just an array representing which of the above records to write
const bool RECORD_ATTRS_TO_WRITE[CRA_COUNT] = {
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true
};

template<class T>
class Vec_aggregator
{
    typedef T(*Aggregator)(const std::vector<T> &, size_t, size_t);
public:
    Vec_aggregator(const std::vector<T> & vec, Aggregator ag_fn, unsigned elements_per_bin);
    const std::vector<T> & get() const { return ag_vec; }
private:
    std::vector<T> ag_vec;
};

// Represents an instance variable or value that is calculated from other values and can be
// cached. A is typically the enclosing class, B is the type of the value to cache. B must
// have a default constructor. It is not the responsibility of this class to observe whether
// the cached value would become dirty or not, which would typically happen when other
// instance variables upon which the value depends change.
template<typename A, typename B>
class Cached_calculable
{
public:
    typedef void(*calculation)(A *, B &);
    Cached_calculable(A * enclosing, calculation c);

    // If the argument is not a nullptr, it will be changed to true/false depending on
    // whether the cached value was used. This can be used to mark dependents for
    // recalculation as well.
    const B & get(bool *recalculated = nullptr) const;

    // marks the cached value as dirty, causing the calculation to be run again.
    void soil() const;

    bool operator==(const Cached_calculable<A, B> other) const;
private:
    A *enclosing_;
    calculation c_;
    mutable bool clean_;
    mutable B val_;
};

template<typename A, typename B>
Cached_calculable<A, B>::Cached_calculable(A * enclosing, calculation c) :
        enclosing_(enclosing),
        c_(c),
        clean_(false)
{}

template<typename A, typename B>
const B &Cached_calculable<A, B>::get(bool *recalculated) const
{
    bool r = false;
    if(!clean_)
    {
        c_(enclosing_, val_);
        clean_ = true;
        r = true;
    }
    if(recalculated) *recalculated = r;
    return val_;

    return get(recalculated);
}

template<typename A, typename B>
void Cached_calculable<A, B>::soil() const
{
    clean_ = false;
}

template<typename A, typename B>
bool Cached_calculable<A, B>::operator==(const Cached_calculable<A, B> other) const
{
    return get(nullptr) == other.get(nullptr);
}

class Range
{
public:
    static constexpr int START_IDX_DEF = 0;
    static constexpr int STOP_IDX_DEF = 1;
    static constexpr int STRIDE_DEF = 1;
public:
    Range() :
            incl_start_idx_(START_IDX_DEF),
            excl_stop_idx_(STOP_IDX_DEF),
            stride_(STRIDE_DEF)
    {}
    Range(int stop_idx) :
            incl_start_idx_(START_IDX_DEF),
            excl_stop_idx_(stop_idx),
            stride_(STRIDE_DEF)
    {}
    Range(int start_idx, int stop_idx, int stride = STRIDE_DEF) :
        incl_start_idx_(start_idx),
        excl_stop_idx_(stop_idx),
        stride_(STRIDE_DEF)
    {}
public:
    const int incl_start_idx_;
    const int excl_stop_idx_;
    const int stride_;
};

class Range_or_value
{
public:
    Range_or_value(unsigned value) :
            is_range_(false),
            value_(value)
    {
    }
    Range_or_value(int range_start, int range_stop, int stride = 1) :
            is_range_(true),
            range_(range_start, range_stop, stride)
    {}

    bool is_range_;
    Range range_;
    unsigned value_;
};

class Unhandled_enum_value_exception : std::exception {};

class Index_oob_exception : std::exception {};

unsigned int read_uint_from_urandom();
long get_sys_time_nano();

bool double_eq(double a, double b, double delta);

kjb::Matrix homo_col_vecs_to_non_homo_col_vecs(const kjb::Matrix & in);
kjb::Matrix non_homo_col_vecs_to_homo_col_vecs(const kjb::Matrix & in);

template<size_t rows, size_t cols>
kjb::Matrix static_matrix_to_dynamic_matrix(const kjb::Matrix_d<rows, cols> &in)
{
    std::cout << "static_matrix_to_dynamic_matrix<" << rows << ", " << cols << ">\n";
    kjb::Matrix r(rows, cols, 0.0);
    for(unsigned row = 0; row < rows; row++)
    {
        for(unsigned col = 0; col < cols; col++)
        {
            r(row,col)= in[row][col];
            std::cout << in[row][col] << " -> " << r(row, col) << "\n";
        }
    }
    return r;
}

kjb::Matrix_d<3,2> stick_length_to_local_endpoints_homo(double len);
double local_endpoints_homo_to_stick_length(const kjb::Matrix_d<3,2> & in);

void draw_poly_edges(kjb::Image &im, const kjb::Matrix &poly, const kjb::Image::Pixel_type &color);

void transfer_image_contents(kjb::Image &out, const kjb::Image &in, const kjb::Image::Pixel_type &color);

const size_t PAD_LEN_DEF = 12;
const char PAD_CHAR_DEF = '0';
std::string pad_unsigned(unsigned num, int pad_len = PAD_LEN_DEF, char pad_char = PAD_CHAR_DEF);

boost::filesystem::path get_sample_path(const std::string & data_dir, unsigned data_idx);

boost::filesystem::path get_data_archive_path(const std::string & data_dir, unsigned data_idx);
boost::filesystem::path get_data_image_path(
        const std::string & data_dir,
        unsigned data_idx,
        unsigned img_idx,
        Data_image_type dit);
boost::filesystem::path get_inference_archive_path(
        const std::string & data_dir,
        unsigned data_idx,
        Inference_type it,
        const std::vector<unsigned> & flex_vars);
boost::filesystem::path get_inference_image_path(
        const std::string & data_dir,
        unsigned data_idx,
        Inference_type it,
        const std::vector<unsigned> & flex_vars,
        unsigned img_idx,
        Inference_image_type iit);
boost::filesystem::path get_csv_path(const std::string & data_dir, unsigned data_idx, util::Inference_type it, const std::vector<unsigned> &flex_vars);

void save_center_of_mass_image(
        unsigned im_w,
        unsigned im_h,
        const std::vector<const kjb::Vector_d<3> *> &coms,
        const boost::filesystem::path &p);
void save_endpoint_image(
        unsigned im_w,
        unsigned im_h,
        const std::vector<const kjb::Matrix_d<3,4> *> &polygons,
        const boost::filesystem::path &p);

void write_csv_header(std::ofstream & f);

std::vector<unsigned> setup_mh_flex_vars(
        unsigned std_exp,
        unsigned interchain_num,
        unsigned intrachain_idx = 0);

std::string err_str(const char *file, unsigned line);

template<typename T>
T sto(const std::string &s, size_t *idx = nullptr);

template<>
inline float sto<float>(const std::string &s, size_t *idx)
{
    return std::stof(s, idx);
}

template<>
inline double sto<double>(const std::string &s, size_t *idx)
{
    return std::stod(s, idx);
}

template<>
inline long double sto<long double>(const std::string &s, size_t *idx)
{
    return std::stold(s, idx);
}

template<>
inline int sto<int>(const std::string &s, size_t *idx)
{
    return std::stoi(s, idx);
}

template<>
inline long sto<long>(const std::string &s, size_t *idx)
{
    return std::stol(s, idx);
}

template<>
inline unsigned long sto<unsigned long>(const std::string &s, size_t *idx)
{
    return std::stoul(s, idx);
}

template<>
inline long long sto<long long>(const std::string &s, size_t *idx)
{
    return std::stoll(s, idx);
}

template<>
inline unsigned long long sto<unsigned long long>(const std::string &s, size_t *idx)
{
    return std::stoull(s, idx);
}

// ranges
template<>
inline std::pair<unsigned, unsigned> sto<std::pair<unsigned, unsigned>>(const std::string &s, size_t *idx)
{
    std::pair<unsigned, unsigned> r;
    size_t split_pos = s.find('-');
    assert(split_pos != std::string::npos);
    r.first = unsigned(stoul(s));
    r.second = unsigned(stoul(s.substr(split_pos + 1, s.length() - split_pos - 1)));
    return r;
}


}
}

#endif // UTIL_HPP
