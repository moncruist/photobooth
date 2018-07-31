#ifndef PHOTOBOOTH_RASPBERRYCAMERA_H
#define PHOTOBOOTH_RASPBERRYCAMERA_H

#include <interface/mmal/mmal.h>

#include "CameraInterface.h"

namespace phb::camera {

class RaspberryCamera : public CameraInterface{
public:
    RaspberryCamera(unsigned int width, unsigned int height, unsigned int frame_rate);
    ~RaspberryCamera();
    int init() override;
    int deinit() override;
    bool is_valid() override;
    cv::Mat get_frame() override;

private:
    static constexpr size_t CAMERA_NECESSARY_PORTS_NUM  = 3;

    static constexpr size_t CAMERA_PREVIEW_PORT     = 0;
    static constexpr size_t CAMERA_VIDEO_PORT       = 1;
    static constexpr size_t CAMERA_CAPTURE_PORT     = 2;

    static constexpr size_t VIDEO_OUTPUT_BUFFERS_NUM    = 3;

    int select_camera(int camera);
    int disable_stereo_mode(MMAL_PORT_T* port);
    int set_sensor_mode(int mode);
    int setup_config();
    int setup_viewfinder_format();
    int setup_video_format();
    int setup_capture_format();

    static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

    unsigned int width_ {0};
    unsigned int height_ {0};
    unsigned int frame_rate_ {0};

    MMAL_COMPONENT_T* camera_ {nullptr};
    MMAL_PORT_T* viewfinder_ {nullptr};
    MMAL_PORT_T* video_ {nullptr};
    MMAL_PORT_T* still_ {nullptr};
};

}

#endif //PHOTOBOOTH_RASPBERRYCAMERA_H
