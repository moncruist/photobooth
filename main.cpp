#include <opencv2/opencv.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>

#ifdef RPI_BOARD
#include <bcm_host.h>
#endif

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

#ifdef RPI_BOARD
    bcm_host_init();
#endif

    init_log();

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
