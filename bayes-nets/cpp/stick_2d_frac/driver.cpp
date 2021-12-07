#include "i/i_float.h"
#include "i_cpp/i_image.h"

#include "sample.hpp"

namespace stick_2d_frac {

const kjb::Image::Pixel_type ACTUAL_COLOR =
{
    .r = 0.0,
    .g = 1.0,
    .b = 0.0,
    {
        .alpha = 1.0
    }
};
const kjb::Image::Pixel_type OBSERVED_COLOR =
{
    .r = 1.0,
    .g = 0.0,
    .b = 1.0,
    {
        .alpha = 1.0
    }
};
const kjb::Image::Pixel_type INFERRED_COLOR =
{
    .r = 0.0,
    .g = 1.0,
    .b = 1.0,
    {
        .alpha = 1.0
    }
};
const std::string SAMPLE_DIR = "samples/";
const std::string FORWARD_SAMPLING_SUBDIR = "forward_sampling/";
const std::string INFERENCE_SUBDIR = "inference/";
const std::string IMAGE_SUBDIR = "images/";
const unsigned NUM_IMS(60);
const unsigned IM_W(640);
const unsigned IM_H(480);
const double CAM_FPS(30.0);
const unsigned NUM_SAMPLES(10);

// Basically pass this into the ostream before a number, then again after the number,
// for easy, consistent formatting.
class ostream_tickler {
public:
    ostream_tickler(): toggle_(false) {}

    friend std::ostream & operator<<(std::ostream & s, const ostream_tickler & ot);
private:
    mutable bool toggle_;
};
std::ostream & operator<<(std::ostream & s, const ostream_tickler & ot)
{
    if(!ot.toggle_) {
        ot.toggle_ = true;
        return s << std::setw(6) << std::setfill('0') << std::right;
    } else {
        ot.toggle_ = false;
        return s << std::setw(0);
    }
}
const ostream_tickler OT;

void set_format_number(std::ostream & s) { s << std::setw(6) << std::setfill('0') << std::right; }

void clear_format_number(std::ostream & s) { s << std::setw(0); }

}

int main(int argc, const char ** argv) {
    // TODO read arguments instead of using globals
    using namespace stick_2d_frac;
    for(unsigned i = 0; i < NUM_SAMPLES; i++) {
        // Base path
        std::ostringstream path_stream;
        path_stream << SAMPLE_DIR << OT << i << OT << '/' << FORWARD_SAMPLING_SUBDIR;

        // For the archive file
        std::ostringstream ar_filename_stream(path_stream.str());
        ar_filename_stream << OT << i << OT << "_forward_sample.serializedcppobject";

        // Image path
        path_stream << IMAGE_SUBDIR;

        // Forward sample
        sample::Sample s(NUM_IMS, IM_W, IM_H, CAM_FPS);
        for(unsigned j = 0; j < NUM_IMS; j++) {
            // File names
            std::ostringstream act_im_filename_stream(path_stream.str());
            act_im_filename_stream << "actual/" << OT << i << OT << '_' << OT << j << OT << "_actual.tiff";
            std::ostringstream obs_im_filename_stream(path_stream.str());
            obs_im_filename_stream << "observed/" << OT << i << OT << '_' << OT << j << OT << "_observed.tiff";
            std::ostringstream both_im_filename_stream(path_stream.str());
            both_im_filename_stream << "both/" << OT << i << OT << '_' << OT << j << OT << "_both.tiff";
            kjb::Image im_actual(IM_W, IM_H, 0, 0, 0);
            kjb::Image im_observed(IM_W, IM_H, 0, 0, 0);
            kjb::Image im_both(IM_W, IM_H, 0, 0, 0);
            std::vector<kjb::Matrix> container;
            s.get_continuous_sample().get_stick_tree().get_all_active_hidden_lines
            (
                s.get_discrete_sample().get_frac_lifetime_tree(),
                j,
                container
            );
            for(const kjb::Matrix & line : container) {
                im_actual.draw_line_segment(line(0, 0), line(0, 1), line(1, 0), line(1, 1), 1, ACTUAL_COLOR);
                im_both.draw_line_segment(line(0, 0), line(0, 1), line(1, 0), line(1, 1), 1, ACTUAL_COLOR);
            }
            container.clear();
            s.get_continuous_sample().get_stick_tree().get_all_active_observed_lines
            (
                s.get_discrete_sample().get_frac_lifetime_tree(),
                j,
                container
            );
            for(const kjb::Matrix & line : container) {
                im_observed.draw_line_segment(line(0, 0), line(0, 1), line(1, 0), line(1, 1), 1, OBSERVED_COLOR);
                im_both.draw_line_segment(line(0, 0), line(0, 1), line(1, 0), line(1, 1), 1, OBSERVED_COLOR);
            }
            im_actual.write(act_im_filename_stream.str());
            im_observed.write(obs_im_filename_stream.str());
            im_both.write(both_im_filename_stream.str());
        }

    }



    return 0;
}

