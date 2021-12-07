#include "boost/archive/archive_exception.hpp"

#include "config.hpp"
#include "sample.hpp"
#include "util.hpp"

int main(int argc, char *argv[])
{
    using namespace fracture;
    using namespace block_2d;
    // TODO implement a separate argument parser
    cfg::Arguments_data_gen args(argc, argv);

    Sample s;
    try
    {
        // use the new data record format if possible.
        Data_record_20191031 dr;
        load_data_record(args.data_folder_, args.dataset_idx_, dr);
        s = dr.sample_;
    }
    catch (const boost::archive::archive_exception &e)
    {
        // fallback to the old sample file type.
        load_sample(args.data_folder_, args.dataset_idx_, s);
    }

    for(unsigned i = 0; i < s.get_num_ims(); i++)
    {
        util::save_center_of_mass_image(
                s.get_camera().get_image_width(),
                s.get_camera().get_image_height(),
                s.get_image_center_of_mass(i),
                util::get_data_image_path(
                        args.data_folder_,
                        args.dataset_idx_,
                        i,
                        util::DIT_CENTER_OF_MASS));
        util::save_endpoint_image(
                s.get_camera().get_image_width(),
                s.get_camera().get_image_height(),
                s.get_image_polygon_actual(i),
                util::get_data_image_path(
                        args.data_folder_,
                        args.dataset_idx_,
                        i,
                        util::DIT_ACTUAL_GEOM));
        util::save_endpoint_image(
                s.get_camera().get_image_width(),
                s.get_camera().get_image_height(),
                s.get_image_polygon_observed(i),
                util::get_data_image_path(
                        args.data_folder_,
                        args.dataset_idx_,
                        i,
                        util::DIT_OBSERVED_GEOM));
    }
}
