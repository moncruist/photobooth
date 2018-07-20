#ifndef PHOTOBOOTH_RASPBERRYCAMERA_H
#define PHOTOBOOTH_RASPBERRYCAMERA_H

#include <interface/mmal/mmal.h>

#include "CameraInterface.h"

namespace phb {

class RaspberryCamera : public CameraInterface{
public:
    int init() override;
    int deinit() override;
    cv::Mat get_frame() override;

private:
    bool check_camera();
};

}

#endif //PHOTOBOOTH_RASPBERRYCAMERA_H
