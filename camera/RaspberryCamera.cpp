#include <interface/mmal/util/mmal_default_components.h>
#include "RaspberryCamera.h"
#include "logging.h"

namespace phb::camera {

int RaspberryCamera::init() {
    MMAL_STATUS_T status;

    INFO() << "Initialize Raspberry Camera";
    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera_);
    if (status != MMAL_SUCCESS) {
        ERR() << "Failed to create component '" << MMAL_COMPONENT_DEFAULT_CAMERA << "': " << status;
        return -1;
    }

    bool success = true;
    for (int i = 0; i < camera_->output_num; i++) {
        status = disable_stereo_mode(camera_->output[i]);
        if (status != MMAL_SUCCESS) {
            success = false;
        }
    }

    if (!success) {
        ERR() << "Failed to disable stereo";
        return -1;
    }

    MMAL_PARAMETER_INT32_T camera_num = {
            {MMAL_PARAMETER_CAMERA_NUM, sizeof(camera_num)},
            0
    };

    status = mmal_port_parameter_set(camera_->control, &camera_num.hdr);
    if (status != MMAL_SUCCESS) {
        ERR() << "Failed to select camera: " << status;
        return -1;
    }



    return 0;
}

int RaspberryCamera::deinit() {
    if (camera_) {
        mmal_component_destroy(camera_);
        camera_ = nullptr;
    }
    return 0;
}

bool RaspberryCamera::is_valid() {
    return false;
}

cv::Mat RaspberryCamera::get_frame() {
    return cv::Mat();
}

MMAL_STATUS_T RaspberryCamera::disable_stereo_mode(MMAL_PORT_T* port)
{
    MMAL_PARAMETER_STEREOSCOPIC_MODE_T stereo = {
            {MMAL_PARAMETER_STEREOSCOPIC_MODE, sizeof(stereo)},
            MMAL_STEREOSCOPIC_MODE_NONE, MMAL_FALSE, MMAL_FALSE
    };

    return mmal_port_parameter_set(port, &stereo.hdr);
}

}
