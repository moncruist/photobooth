#include <opencv2/opencv.hpp>
#include <interface/mmal/util/mmal_default_components.h>
#include <interface/mmal/util/mmal_util_params.h>
#include <interface/mmal/util/mmal_util.h>
#include "RaspberryCamera.h"
#include "util/TimeUtil.h"
#include "logging.h"

#define WRAP_CALL_RETURN_FULL(call, st)  do {\
    st = call; \
    if (st != MMAL_SUCCESS) { \
        return st; \
    } \
} while(0)

#define WRAP_CALL_RETURN(call) WRAP_CALL_RETURN_FULL(call, status)

namespace phb::camera {

RaspberryCamera::RaspberryCamera(unsigned int width, unsigned int height, unsigned int frame_rate)
    : width_(width), height_(height), frame_rate_(frame_rate) {

}

RaspberryCamera::~RaspberryCamera() {
    deinit();
}

int RaspberryCamera::init() {
    std::lock_guard<decltype(mutex_)> guard(mutex_);
    int status = init_camera_component();
    if (status != MMAL_SUCCESS) {
        ERR() << "Failed to init camera: " << status;
        goto cleanup;
    }

    if ((status = init_preview_component()) != MMAL_SUCCESS) {
        goto cleanup;
    }

    video_->userdata = reinterpret_cast<struct MMAL_PORT_USERDATA_T*>(this);
    status = mmal_port_enable(video_, &RaspberryCamera::camera_video_callback);
    if (status != MMAL_SUCCESS) {
        ERR() << "Failed to enable video port: " << status;
        goto cleanup;
    }

    INFO() << "Start capture";
    status = start_capture();
    if (status != MMAL_SUCCESS) {
        ERR() << "Failed to start capture: " << status;
        goto cleanup;
    }

    initialized_ = true;
    return 0;

cleanup:
    deinit_locked();
    return -status;
}

int RaspberryCamera::deinit() {
    frame_number_ = 0;
    std::lock_guard guard(mutex_);
    return deinit_locked();
}

int RaspberryCamera::deinit_locked() {
    if (preview_connection_) {
        mmal_connection_destroy(preview_connection_);
        preview_connection_ = nullptr;
    }
    deinit_camera_component();
    deinit_preview_component();
    initialized_ = false;
    return 0;
}

bool RaspberryCamera::is_valid() {
    return initialized_;
}

cv::Mat RaspberryCamera::get_frame() {
    std::lock_guard guard(mutex_);
    return frame_;
}

int RaspberryCamera::init_camera_component() {
    int status;
    bool success = true;
    INFO() << "Initialize Raspberry Camera";
    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera_);
    if (status != MMAL_SUCCESS) {
        ERR() << "Failed to create component '" << MMAL_COMPONENT_DEFAULT_CAMERA << "': " << status;
        return status;
    }

    if (camera_->output_num < CAMERA_NECESSARY_PORTS_NUM) {
        ERR() << "Camera has invalid number of outputs: " << camera_->output_num;
        status = MMAL_EINVAL;
        return status;
    }

    viewfinder_ = camera_->output[CAMERA_PREVIEW_PORT];
    video_ = camera_->output[CAMERA_VIDEO_PORT];
    still_ = camera_->output[CAMERA_CAPTURE_PORT];

    for (int i = 0; i < camera_->output_num; i++) {
        status = disable_stereo_mode(camera_->output[i]);
        if (status != MMAL_SUCCESS) {
            success = false;
            break;
        }
    }

    if (!success) {
        return status;
    }

    WRAP_CALL_RETURN(select_camera(0));
    WRAP_CALL_RETURN(set_sensor_mode(0)); // set auto mode

    status = mmal_port_enable(camera_->control, &RaspberryCamera::camera_control_callback);
    if (status != MMAL_SUCCESS) {
        ERR() << "Failed to enable camera control port: " << status;
        return status;
    }

    WRAP_CALL_RETURN(setup_config());
    WRAP_CALL_RETURN(setup_viewfinder_format());
    WRAP_CALL_RETURN(setup_video_format());
    WRAP_CALL_RETURN(setup_capture_format());

    INFO() << "Enabling camera";
    status = mmal_component_enable(camera_);
    if (status != MMAL_SUCCESS) {
        ERR() << "Failed to enable camera: " << status;
        return status;
    }

    return MMAL_SUCCESS;
}

int RaspberryCamera::init_preview_component() {
    INFO() << "Initialize Raspberry Dummy Preview component";

    int status = 0;
    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_RENDERER, &dummy_preview_);
    if (status != MMAL_SUCCESS) {
        ERR() << "Failed to create preview component: " << status;
        return status;
    }

    status = mmal_component_enable(dummy_preview_);
    if (status != MMAL_SUCCESS) {
        ERR() << "Failed to enable preview component: " << status;
        return status;
    }

    if (dummy_preview_->input_num == 0) {
        ERR() << "No input ports in preview component";
        return MMAL_EINVAL;
    }

    preview_input_port_ = dummy_preview_->input[0];

    return MMAL_SUCCESS;
}

void RaspberryCamera::deinit_camera_component() {
    if (!camera_) {
        return;
    }

    INFO() << "Cleaning up camera";
    stop_capture();
    if (video_ && camera_pool_) {
        mmal_port_disable(video_);
        mmal_port_pool_destroy(video_, camera_pool_);
    }

    mmal_component_destroy(camera_);
    viewfinder_ = nullptr;
    video_ = nullptr;
    still_ = nullptr;
    camera_ = nullptr;
    camera_pool_ = nullptr;
}

void RaspberryCamera::deinit_preview_component() {
    if (!dummy_preview_) {
        return;
    }

    INFO() << "Cleaning up preview";
    mmal_component_destroy(dummy_preview_);
    dummy_preview_ = nullptr;
    preview_input_port_ = nullptr;
}

int RaspberryCamera::select_camera(int camera) {
    MMAL_PARAMETER_INT32_T camera_num = {
            {MMAL_PARAMETER_CAMERA_NUM, sizeof(camera_num)},
            camera
    };

    int status = mmal_port_parameter_set(camera_->control, &camera_num.hdr);
    if (status != MMAL_SUCCESS) {
        ERR() << "Failed to select camera: " << status;
    }
    return status;
}

int RaspberryCamera::disable_stereo_mode(MMAL_PORT_T* port)
{
    MMAL_PARAMETER_STEREOSCOPIC_MODE_T stereo = {
            {MMAL_PARAMETER_STEREOSCOPIC_MODE, sizeof(stereo)},
            MMAL_STEREOSCOPIC_MODE_NONE, MMAL_FALSE, MMAL_FALSE
    };

    int status = mmal_port_parameter_set(port, &stereo.hdr);
    if (status != MMAL_SUCCESS) {
        ERR() << "Failed to disable stereo for port: " << status;
    }
    return status;
}

int RaspberryCamera::set_sensor_mode(int mode) {
    int status = mmal_port_parameter_set_uint32(camera_->control, MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG, mode);
    if (status != MMAL_SUCCESS) {
        ERR() << "Failed to set sensor mode: " << status;
    }
    return status;
}

int RaspberryCamera::setup_config() {
    MMAL_PARAMETER_CAMERA_CONFIG_T cam_config = {
            {MMAL_PARAMETER_CAMERA_CONFIG, sizeof(cam_config)}
    };
    cam_config.max_stills_w = width_;
    cam_config.max_stills_h = height_;
    cam_config.stills_yuv422 = 0;
    cam_config.one_shot_stills = 0;
    cam_config.max_preview_video_w = width_;
    cam_config.max_preview_video_h = height_;
    cam_config.num_preview_video_frames = 3 + vcos_max(0, (frame_rate_ - 30) / 10);
    cam_config.stills_capture_circular_buffer_height = 0;
    cam_config.fast_preview_resume = 0;
    cam_config.use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RAW_STC;
    int status = mmal_port_parameter_set(camera_->control, &cam_config.hdr);
    if (status != MMAL_SUCCESS) {
        ERR() << "Failed to set camera config: " << status;
    }
    return status;
}

int RaspberryCamera::setup_viewfinder_format() {
    // Set the encode format on the Preview port
    // HW limitations mean we need the preview to be the same size as the required recorded output
    MMAL_ES_FORMAT_T* format = viewfinder_->format;
    format->encoding = MMAL_ENCODING_OPAQUE;
    format->es->video.width = VCOS_ALIGN_UP(width_, 32);
    format->es->video.height = VCOS_ALIGN_UP(height_, 16);
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = width_;
    format->es->video.crop.height = height_;
    format->es->video.frame_rate.num = 0;
    format->es->video.frame_rate.den = 1;

    int status = mmal_port_format_commit(viewfinder_);
    if (status != MMAL_SUCCESS) {
        ERR() << "Failed to set viewfinder format: " << status;
    }
    return status;
}

int RaspberryCamera::setup_video_format() {
    // Set the encode format on the video  port
    MMAL_ES_FORMAT_T* format = video_->format;
    format->encoding_variant = 0;
    format->encoding = MMAL_ENCODING_RGB24;
    format->es->video.width = VCOS_ALIGN_UP(width_, 32);
    format->es->video.height = VCOS_ALIGN_UP(height_, 16);
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = width_;
    format->es->video.crop.height = height_;
    format->es->video.frame_rate.num = frame_rate_;
    format->es->video.frame_rate.den = 1;

    int status = mmal_port_format_commit(video_);
    if (status != MMAL_SUCCESS) {
        ERR() << "Failed to set video format: " << status;
        return status;
    }

    status = mmal_port_parameter_set_boolean(video_, MMAL_PARAMETER_ZERO_COPY, MMAL_TRUE);
    if (status != MMAL_SUCCESS) {
        ERR() << "Failed to enable zero copy " << status;
        return status;
    }

    if (video_->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM) {
        video_->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;
    }

    camera_pool_ = mmal_port_pool_create(video_, video_->buffer_num, video_->buffer_size);
    return status;
}

int RaspberryCamera::setup_capture_format() {
    // Set the encode format on the still  port
    MMAL_ES_FORMAT_T* format = still_->format;
    format->encoding = MMAL_ENCODING_OPAQUE;
    format->encoding_variant = MMAL_ENCODING_I420;
    format->es->video.width = VCOS_ALIGN_UP(width_, 32);
    format->es->video.height = VCOS_ALIGN_UP(height_, 16);
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = width_;
    format->es->video.crop.height = height_;
    format->es->video.frame_rate.num = 0;
    format->es->video.frame_rate.den = 1;

    int status = mmal_port_format_commit(still_);
    if (status != MMAL_SUCCESS) {
        ERR() << "Failed to set capture format: " << status;
    }

    if (still_->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM) {
        still_->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;
    }
    return status;
}

int RaspberryCamera::start_capture() {
    int num = mmal_queue_length(camera_pool_->queue);
    for (int q=0; q<num;q++) {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(camera_pool_->queue);

        if (!buffer)
            ERR() << "Unable to get a required buffer " << q << " from pool queue";

        if (mmal_port_send_buffer(video_, buffer)!= MMAL_SUCCESS)
            ERR() << "Unable to send a buffer to camera video port: " << q;
    }

    return mmal_port_parameter_set_boolean(video_, MMAL_PARAMETER_CAPTURE, MMAL_TRUE);
}

int RaspberryCamera::stop_capture() {
    return mmal_port_parameter_set_boolean(video_, MMAL_PARAMETER_CAPTURE, MMAL_FALSE);
}

void RaspberryCamera::update_frame(cv::Mat frame) {
    std::lock_guard guard(mutex_);

    frame_ = std::move(frame);
    frame_number_++;
}

void RaspberryCamera::camera_control_callback(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer) {
    if (buffer->cmd == MMAL_EVENT_PARAMETER_CHANGED) {
        INFO() << "Something changed";
    }
    else if (buffer->cmd == MMAL_EVENT_ERROR) {
        ERR() << "No data received from sensor. Check all connections, including the Sunny one on the camera board";
    }
    else {
        ERR() << "Received unexpected camera control callback event, " << buffer->cmd;
    }

    mmal_buffer_header_release(buffer);
}

void RaspberryCamera::camera_video_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
    using namespace std::chrono;
    static int64_t frame_num = 0;
    static int64_t total_time = 0;
    static int64_t prev_time = 0;

    RaspberryCamera* raspicam = reinterpret_cast<RaspberryCamera*>(port->userdata);
    if (!raspicam) {
        ERR() << "Received a camera buffer callback with no state";
        // release buffer back to the pool
        mmal_buffer_header_release(buffer);
    }

    if (prev_time == 0) {
        prev_time = util::now_ms();
    }

    frame_num++;

    auto current_time = util::now_ms();
    auto delta = current_time - prev_time;
    prev_time = current_time;

    total_time += delta;
    if (total_time > 10000) {
        double fps = static_cast<double>(frame_num) * 1000 / static_cast<double>(total_time);
        DBG() << frame_num << " frames captured in " << total_time << " milliseconds. FPS=" << fps;
        total_time -= 10000;
        frame_num = 0;
    }

    mmal_buffer_header_mem_lock(buffer);
    cv::Mat new_frame(raspicam->width_, raspicam->height_, CV_8UC1, buffer->data);
    mmal_buffer_header_mem_unlock(buffer);
    // release buffer back to the pool
    mmal_buffer_header_release(buffer);

    // and send one back to the port (if still open)
    if (port->is_enabled) {
        MMAL_STATUS_T status;

        MMAL_BUFFER_HEADER_T* new_buffer = mmal_queue_get(raspicam->camera_pool_->queue);

        if (new_buffer) {
            status = mmal_port_send_buffer(port, new_buffer);
        }

        if (!new_buffer || status != MMAL_SUCCESS) {
            ERR() << "Unable to return a buffer to the camera port";
        }
    }

    raspicam->update_frame(std::move(new_frame));
}

}
