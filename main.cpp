#include <opencv2/opencv.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>
#include <fstream>
#include <thread>
#include <QString>
#include <QApplication>
#include "gui/AppWindow.h"
#include "CameraConsumerThread.h"

#ifdef RPI_BOARD
#include <bcm_host.h>
#include "camera/RaspberryCamera.h"
#else
#include "camera/OpenCvCamera.h"
#endif

#include "dslr/DslrCamera.h"
#include "dslr/DslrCameraInfo.h"
#include "logging.h"


static void init_log()
{
    static const std::string COMMON_FMT("[%TimeStamp%][%Severity%]%Message%");

    boost::log::register_simple_formatter_factory< boost::log::trivial::severity_level, char >("Severity");

    // Output message to console
    boost::log::add_console_log(
        std::cout,
        boost::log::keywords::format = COMMON_FMT,
        boost::log::keywords::auto_flush = false
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
    try {
        auto file = cam.capture();

        INFO() << "File name: " << file.name;
        std::ofstream out(file.name, std::ios_base::binary | std::ios_base::out);
        char* buf_ptr = reinterpret_cast<char*>(file.data.data());
        out.write(buf_ptr, file.data.size());
        out.close();
    } catch (std::runtime_error &e) {
        ERR() << "Error: " << e.what();
    }

    std::unique_ptr<phb::camera::CameraInterface> camera;
#ifdef RPI_BOARD
    camera = std::make_unique<phb::camera::RaspberryCamera>(1920, 1080, 30);
#else
    camera = std::make_unique<phb::camera::OpenCvCamera>(0);
#endif

    if(camera->init() != 0) {  // check if we succeeded
        return -1;
    }

    QApplication app(argc, argv);
    phb::gui::AppWindow window;
    window.setFixedSize(1920, 1080);
    window.show();

    phb::CameraConsumerThread consumerThread(camera.get(), window.frameOutput());
    consumerThread.start();
    app.exec();

    consumerThread.stop();
    consumerThread.join();

    camera->deinit();
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
