#ifndef PHOTOBOOTH_CAMERA_H
#define PHOTOBOOTH_CAMERA_H

#include <opencv2/opencv.hpp>

namespace phb::camera {

class CameraInterface {
public:
    virtual ~CameraInterface() = default;
    virtual int init() = 0;
    virtual int deinit() = 0;
    virtual bool is_valid() = 0;
    virtual int64_t frame_number() = 0;
    virtual cv::Mat get_frame() = 0;
};

}

#endif //PHOTOBOOTH_CAMERA_H
