#include "RaspberryCamera.h"

namespace phb::camera {

int RaspberryCamera::init() {
    return 0;
}

int RaspberryCamera::deinit() {
    return 0;
}

bool RaspberryCamera::is_valid() {
    return false;
}

cv::Mat RaspberryCamera::get_frame() {
    return cv::Mat();
}

}