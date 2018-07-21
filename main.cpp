#include <opencv2/opencv.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>
#include <fstream>

#ifdef RPI_BOARD
#include <bcm_host.h>
#endif

#include "dslr/DslrCamera.h"
#include "dslr/DslrCameraInfo.h"
#include "logging.h"

static void init_log()
{
    static const std::string COMMON_FMT("[%TimeStamp%][%Severity%]:  %Message%");

    boost::log::register_simple_formatter_factory< boost::log::trivial::severity_level, char >("Severity");

    // Output message to console
    boost::log::add_console_log(
        std::cout,
        boost::log::keywords::format = COMMON_FMT,
        boost::log::keywords::auto_flush = true
    );

    // Output message to file, rotates when file reached 1mb or at midnight every day. Each log file
    // is capped at 1mb and total is 20mb
//    boost::log::add_file_log (
//        boost::log::keywords::file_name = "sample_%3N.log",
//        boost::log::keywords::rotation_size = 1 * 1024 * 1024,
//        boost::log::keywords::max_size = 20 * 1024 * 1024,
//        boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
//        boost::log::keywords::format = COMMON_FMT,
//        boost::log::keywords::auto_flush = true
//    );

    boost::log::add_common_attributes();
}

int main(int argc, char *argv[]) {

    using namespace phb::dslr;
#ifdef RPI_BOARD
    bcm_host_init();
#endif

    init_log();

    auto ctx = DslrContext::make_context();
    auto list = DslrCameraInfo::get_available_cameras(ctx);
    DslrCamera cam{ctx};
    auto file = cam.capture();

    INFO() << "File name: " << file.name;
    std::ofstream out(file.name, std::ios_base::binary | std::ios_base::out);
    char *buf_ptr = reinterpret_cast<char*>(file.data.data());
    out.write(buf_ptr, file.data.size());
    out.close();
    return 0;

    cv::VideoCapture cap(0); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
        return -1;

    cv::Mat edges;
    cv::namedWindow("edges",1);
    for(;;)
    {
        cv::Mat frame;
        cap >> frame; // get a new frame from camera
        cv::cvtColor(frame, edges, cv::COLOR_BGR2GRAY);
        GaussianBlur(edges, edges, cv::Size(7,7), 1.5, 1.5);
        Canny(edges, edges, 0, 30, 3);
        imshow("edges", edges);
        if(cv::waitKey(30) >= 0) break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
