#ifndef PHOTOBOOTH_RASPBERRYCAMERA_H
#define PHOTOBOOTH_RASPBERRYCAMERA_H

#include <mutex>
#include <interface/mmal/mmal.h>
#include <interface/mmal/util/mmal_connection.h>

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

    int deinit_locked();

    int init_camera_component();
    int init_preview_component();
    void deinit_camera_component();
    void deinit_preview_component();

    int select_camera(int camera);
    int disable_stereo_mode(MMAL_PORT_T* port);
    int set_sensor_mode(int mode);
    int setup_config();
    int setup_viewfinder_format();
    int setup_video_format();
    int setup_capture_format();

    int start_capture();
    int stop_capture();

    void update_frame(cv::Mat frame);

    static void camera_control_callback(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer);
    static void camera_video_callback(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer);

    std::mutex mutex_;

    std::atomic<int64_t> frame_number_ {0};
    std::atomic<bool> initialized_ {false};
    cv::Mat frame_;

    unsigned int width_ {0};
    unsigned int height_ {0};
    unsigned int frame_rate_ {0};

    MMAL_COMPONENT_T* camera_ {nullptr};
    MMAL_POOL_T* camera_pool_ {nullptr};
    MMAL_PORT_T* viewfinder_ {nullptr};
    MMAL_PORT_T* video_ {nullptr};
    MMAL_PORT_T* still_ {nullptr};

    MMAL_COMPONENT_T* dummy_preview_ {nullptr};
    MMAL_PORT_T* preview_input_port_ {nullptr};
    MMAL_CONNECTION_T *preview_connection_ {nullptr};
};

}

#endif //PHOTOBOOTH_RASPBERRYCAMERA_H
