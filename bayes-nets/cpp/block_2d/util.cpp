#include <cstdio>
#include <chrono>

#include "config.hpp"
#include "sample.hpp"
#include "util.hpp"

bool operator==(const kjb::Normal_distribution &a, const kjb::Normal_distribution &b)
{
    return (a.mean() == b.mean())
            && (a.standard_deviation() == b.standard_deviation());
}
std::istringstream &operator>>(std::istringstream &ss, std::pair<unsigned,unsigned> &o)
{
    ss >> o.first;
    assert(ss.peek() == '-');
    char c;
    ss >> c;
    ss >> o.second;
    return ss;
}

namespace fracture
{
namespace util
{

template<class T>
Vec_aggregator<T>::Vec_aggregator(const std::vector<T> & vec, Aggregator ag_fn, unsigned elements_per_bin)
{
    size_t num_bins = size_t(vec.size() / elements_per_bin) + (vec.size() % elements_per_bin ? 1 : 0);
    ag_vec.reserve(num_bins);
    for(size_t target_idx = 0; target_idx < num_bins; target_idx++)
    {
        size_t first = size_t(elements_per_bin * target_idx);
        size_t last;
        if(target_idx == num_bins - 1)
        {
            last = vec.size();
        }
        else
        {
            last = size_t(elements_per_bin * (target_idx + 1));
        }
        ag_vec.push_back(ag_fn(vec, first, last));
    }
}

std::string pad_unsigned(unsigned num, int pad_len, char pad_char)
{
    std::ostringstream oss;
    oss << std::setw(pad_len) << std::setfill(pad_char) << std::right << num;
    return oss.str();
}

unsigned int read_uint_from_urandom()
{
    std::ifstream urandom_f;
    urandom_f.open("/dev/urandom", std::ios_base::in);
    unsigned int r;
    urandom_f.read((char *)(&r), sizeof(unsigned int));
    return r;
}

long get_sys_time_nano()
{
    auto tp = std::chrono::high_resolution_clock::now();
    auto tp_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(tp);
    auto tp_ns_since_epoch = tp_ns.time_since_epoch();
    return tp_ns_since_epoch.count();
}

bool double_eq(double a, double b, double delta)
{
    if(a < b)
    {
        if(a + delta > b) return true;
    }
    else
    {
        if(a - delta < b) return true;
    }
}

void draw_poly_edges(kjb::Image &im, const kjb::Matrix &poly, const kjb::Image::Pixel_type &color)
{
    for(unsigned k = 0; k < unsigned(poly.get_num_cols()); k++)
    {
        unsigned next = (k + 1) % unsigned(poly.get_num_cols());
        im.draw_line_segment(
                int(poly(0, int(k)) / poly(2, int(k))),
                int(poly(1, int(k)) / poly(2, int(k))),
                int(poly(0, int(next)) / poly(2, int(next))),
                int(poly(1, int(next)) / poly(2, int(next))),
                0,
                color);
    }
}

void transfer_image_contents(kjb::Image &out, const kjb::Image &in, const kjb::Image::Pixel_type &color)
{
    assert(out.get_num_cols() == in.get_num_cols() && out.get_num_rows() == in.get_num_rows());
    for(unsigned x = 0; x < out.get_num_cols(); x++)
    {
        for(unsigned y = 0; y < out.get_num_rows(); y++)
        {
            const kjb::Image::Pixel_type &pixel = in(y, x);
            if(pixel.r != 0.0 || pixel.g != 0.0 || pixel.b != 0.0)
            {
                out(y, x) = color;
            }
        }
    }
}

kjb::Matrix homo_col_vecs_to_non_homo_col_vecs(const kjb::Matrix & in)
{
    kjb::Matrix r(in.get_num_rows() - 1, in.get_num_cols());
    kjb::Vector last_row = in.get_row(in.get_num_rows() - 1);
    for(int i = 0; i < in.get_num_rows() - 1; i++)
    {
        for(int j = 0; j < in.get_num_cols(); j++)
        {
            r(i, j) = in(i, j) / in(in.get_num_rows() - 1, j);
        }
    }
    return r;
}

kjb::Matrix non_homo_col_vecs_to_homo_col_vecs(const kjb::Matrix & in)
{
    kjb::Matrix r(in);
    r.resize(in.get_num_rows() + 1, in.get_num_cols(), 1.0);
    return r;
}

kjb::Matrix_d<3,2> stick_length_to_local_endpoints_homo(double len)
{
    kjb::Matrix_d<3,2> r(0.0);
    r(0, 0) = -len / 2.0; r(0, 1) = len / 2.0;
    //               0.0; //              0.0;
    r(2, 0) =        1.0; r(2, 1) =       1.0;
    return r;
}

double local_endpoints_homo_to_stick_length(const kjb::Matrix & in)
{
    return in(0, 1) * 2.0;
}

boost::filesystem::path get_sample_path(const std::string & data_dir, unsigned data_idx)
{
    boost::filesystem::path sample_path(data_dir);
    if(!boost::filesystem::exists(sample_path))
    {
        boost::filesystem::create_directory(sample_path);
    }
    boost::filesystem::path sample_instance_path = sample_path / pad_unsigned(data_idx);
    if(!boost::filesystem::exists(sample_instance_path))
    {
        boost::filesystem::create_directory(sample_instance_path);
    }
    return sample_instance_path;
}

boost::filesystem::path get_data_archive_path(const std::string & data_dir, unsigned data_idx)
{
    boost::filesystem::path sample_path = get_sample_path(data_dir, data_idx);
    std::ostringstream fname;
    fname << pad_unsigned(data_idx) << "_data.bar";
    return sample_path / fname.str();
}

boost::filesystem::path get_data_image_path(
        const std::string & data_dir,
        unsigned data_idx,
        unsigned img_idx,
        Data_image_type it)
{
    boost::filesystem::path im_path = get_sample_path(data_dir, data_idx);
    im_path /= DATA_IMAGE_DIRNS[it];
    if(!boost::filesystem::exists(im_path))
    {
        boost::filesystem::create_directory(im_path);
    }
    std::ostringstream cur_im_pname;
    cur_im_pname << pad_unsigned(data_idx) << "_" << pad_unsigned(img_idx) << ".tiff";
    return im_path / cur_im_pname.str();
}

void save_center_of_mass_image(
        unsigned im_w,
        unsigned im_h,
        const std::vector<const kjb::Vector_d<3> *> &coms,
        const boost::filesystem::path &p)
{
    kjb::Image im(im_h, im_w, cfg::COLOR_BACKGROUND.r, cfg::COLOR_BACKGROUND.g, cfg::COLOR_BACKGROUND.b);
    for(const kjb::Vector_d<3> *pt : coms)
    {
        im.draw_point((*pt)[0] / (*pt)[2], (*pt)[1] / (*pt)[2], 1, cfg::COLOR_FOREGROUND);
    }
    im.write(p.string());
}

void save_endpoint_image(
        unsigned im_w,
        unsigned im_h,
        const std::vector<const kjb::Matrix_d<3,4> *> &polygons,
        const boost::filesystem::path &p)
{
    kjb::Image im(im_h, im_w, cfg::COLOR_BACKGROUND.r, cfg::COLOR_BACKGROUND.g, cfg::COLOR_BACKGROUND.b);
    for(const kjb::Matrix_d<3,4> *block : polygons)
    {
        util::draw_poly_edges(
                im,
                util::static_matrix_to_dynamic_matrix<3,4>(*block),
                cfg::COLOR_FOREGROUND);
    }
    im.write(p.string());
}

static std::string flex_vars_to_string(std::vector<unsigned> flex_vars)
{
    std::ostringstream ss;
    for(unsigned i = 0; i < flex_vars.size(); i++)
    {
        if(i != 0) ss << "_";
        ss << pad_unsigned(flex_vars[i]);
    }
    return ss.str();
}

boost::filesystem::path get_inference_archive_path(
        const std::string & data_dir,
        unsigned data_idx,
        Inference_type it,
        const std::vector<unsigned> & flex_vars)
{
    boost::filesystem::path sample_path = get_sample_path(data_dir, data_idx);
    std::ostringstream ar_fname;
    ar_fname << pad_unsigned(data_idx) << "_" << INFERENCE_TYPE_STR[it] << "_" << flex_vars_to_string(flex_vars) << ".bar";
    return sample_path / ar_fname.str();
}

boost::filesystem::path get_inference_image_path(
        const std::string & data_dir,
        unsigned data_idx,
        Inference_type it,
        const std::vector<unsigned> & flex_vars,
        unsigned img_idx,
        Inference_image_type iit)
{
    boost::filesystem::path path = get_sample_path(data_dir, data_idx);
    std::ostringstream oss;
    oss << INFERENCE_TYPE_STR[it] << "_" << flex_vars_to_string(flex_vars);
    path /= oss.str();
    if(!boost::filesystem::exists(path))
    {
        boost::filesystem::create_directory(path);
    }
    path /= INFERENCE_IMAGE_DIRNS[iit];
    if(!boost::filesystem::exists(path))
    {
        boost::filesystem::create_directory(path);
    }
    oss.str("");
    oss.clear();
    // v11 did not like this line.
    // oss = std::ostringstream();
    oss << pad_unsigned(data_idx) << "_" << pad_unsigned(img_idx) << ".tiff";
    return path / oss.str();
}

std::vector<unsigned> setup_mh_flex_vars(unsigned std_exp, unsigned interchain_num, unsigned intrachain_idx)
{
    std::vector<unsigned> r(3);
    r[0] = std_exp;
    r[1] = interchain_num;
    r[2] = intrachain_idx;
    return r;
}

boost::filesystem::path get_csv_path(const std::string & data_dir, unsigned data_idx, util::Inference_type it, const std::vector<unsigned> &flex_vars)
{
    boost::filesystem::path sample_instance_path = get_sample_path(data_dir, data_idx);
    std::ostringstream fname;
    fname << pad_unsigned(data_idx) << "_" << INFERENCE_TYPE_STR[it];
    sample_instance_path /= fname.str();
    return sample_instance_path;
}

void write_csv_header(std::ofstream & f)
{
    bool written = false;
    for(unsigned i = 0; i < CRA_COUNT; i++)
    {
        if(!RECORD_ATTRS_TO_WRITE[i])
        {
            continue;
        }
        if(written)
        {
            f << ",";
        }
        f << "\"" << CRA_STRS[i] << "\"";
        written = true;
    }
    f << "\n";
}

std::string err_str(const char *file, unsigned line)
{
    std::ostringstream str;
    str << file << ":" << line;
    return str.str();
}

}
}
