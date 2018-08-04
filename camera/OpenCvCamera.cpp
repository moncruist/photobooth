#include "OpenCvCamera.h"

namespace phb::camera {

OpenCvCamera::OpenCvCamera(int index)
        : index_(index) {
    camera_ = std::make_unique<cv::VideoCapture>();
}

OpenCvCamera::~OpenCvCamera() {
    deinit();
}

int OpenCvCamera::init() {
    if (!camera_->open(index_)) {
        return -1;
    }
    return 0;
}

int OpenCvCamera::deinit() {
    if (camera_->isOpened()) {
        camera_->release();
    }
    return 0;
}

bool OpenCvCamera::is_valid() {
    return camera_->isOpened();
}

cv::Mat OpenCvCamera::get_frame() {
    if (!is_valid()) {
        return cv::Mat();
    }

    cv::Mat frame;
    *camera_ >> frame;
    frame_number_++;
    return frame;
}

int64_t OpenCvCamera::frame_number() {
    return frame_number_;
}

}