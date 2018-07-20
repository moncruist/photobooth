#include "RaspberryCamera.h"

namespace phb {

int RaspberryCamera::init() {
    return 0;
}

int RaspberryCamera::deinit() {
    return 0;
}

cv::Mat RaspberryCamera::get_frame() {
    return cv::Mat();
}

}