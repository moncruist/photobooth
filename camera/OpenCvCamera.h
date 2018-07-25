#ifndef PHOTOBOOTH_OPENCVCAMERA_H
#define PHOTOBOOTH_OPENCVCAMERA_H

#include <memory>
#include <opencv2/opencv.hpp>
#include "CameraInterface.h"

namespace phb::camera {

class OpenCvCamera : public CameraInterface {
public:
    explicit OpenCvCamera(int index);
    ~OpenCvCamera();

    int init() override;
    int deinit() override;
    bool is_valid() override;
    cv::Mat get_frame() override;

private:
    int index_ {0};
    std::unique_ptr<cv::VideoCapture> camera_;
};

}

#endif //PHOTOBOOTH_OPENCVCAMERA_H
