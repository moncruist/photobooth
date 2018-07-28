#ifndef PHOTOBOOTH_RASPBERRYCAMERA_H
#define PHOTOBOOTH_RASPBERRYCAMERA_H

#include <interface/mmal/mmal.h>

#include "CameraInterface.h"

namespace phb::camera {

class RaspberryCamera : public CameraInterface{
public:
    int init() override;
    int deinit() override;
    bool is_valid() override;
    cv::Mat get_frame() override;

private:
    MMAL_STATUS_T disable_stereo_mode(MMAL_PORT_T* port);

    MMAL_COMPONENT_T* camera_ {nullptr};
};

}

#endif //PHOTOBOOTH_RASPBERRYCAMERA_H
