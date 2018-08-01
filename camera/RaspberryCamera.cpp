#include <interface/mmal/util/mmal_default_components.h>
#include <interface/mmal/util/mmal_util_params.h>
#include "RaspberryCamera.h"
#include "logging.h"

#define WRAP_CALL_GOTO_FULL(call, st, err)  do {\
    st = call; \
    if (st != MMAL_SUCCESS) { \
        goto err; \
    } \
} while(0)

#define WRAP_CALL_RETURN_FULL(call, st)  do {\
    st = call; \
    if (st != MMAL_SUCCESS) { \
        return st; \
    } \
} while(0)

#define WRAP_CALL_GOTO(call) WRAP_CALL_GOTO_FULL(call, status, cleanup)
#define WRAP_CALL_RETURN(call) WRAP_CALL_RETURN_FULL(call, status)

namespace phb::camera {

RaspberryCamera::RaspberryCamera(unsigned int width, unsigned int height, unsigned int frame_rate)
    : width_(width), height_(height), frame_rate_(frame_rate) {

}

RaspberryCamera::~RaspberryCamera() {
    deinit();
}

int RaspberryCamera::init() {
    int status = init_camera_component();
    if (status != MMAL_SUCCESS) {
        ERR() << "Failed to init camera: " << status;
        goto cleanup;
    }

    return 0;

cleanup:
    INFO() << "Cleaning up camera";
    mmal_component_destroy(camera_);
    viewfinder_ = nullptr;
    video_ = nullptr;
    still_ = nullptr;
    return -status;
}

int RaspberryCamera::deinit() {
    if (camera_) {
        INFO() << "Deinitialize Raspberry camera";
        mmal_component_destroy(camera_);
        camera_ = nullptr;
    }
    viewfinder_ = nullptr;
    video_ = nullptr;
    still_ = nullptr;
    return 0;
}

bool RaspberryCamera::is_valid() {
    return false;
}

cv::Mat RaspberryCamera::get_frame() {
    return cv::Mat();
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
    }
    return status;
}

int RaspberryCamera::init_preview_component() {
    return 0;
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
    format->encoding_variant = MMAL_ENCODING_I420;
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
    }

    if (video_->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM) {
        video_->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;
    }
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

}
