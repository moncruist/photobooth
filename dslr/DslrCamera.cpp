#include <gphoto2/gphoto2-list.h>
#include <gphoto2/gphoto2-camera.h>
#include "DslrCamera.h"
#include "logging.h"

namespace phb::dslr {

DslrCamera::DslrCamera(std::shared_ptr<DslrContext> context) {
    context_ = context;

    gp_camera_new(&camera_);
    gp_camera_init(camera_, context->get_context());
}

DslrCamera::~DslrCamera() {
    gp_camera_exit(camera_, context_->get_context());
    gp_camera_unref(camera_);
}

CapturedFile DslrCamera::capture() {
    CameraFilePath path;
    CameraFileInfo info;
    memset(&path, 0, sizeof(path));
    memset(&info, 0, sizeof(info));

    int status = 0;

    status = gp_camera_capture(camera_, GP_CAPTURE_IMAGE, &path, context_->get_context());
    if (status != 0) {
        ERR() << "gp_camera_capture failed: " << status;
        throw std::runtime_error("Failed to capture");
    }

    status = gp_camera_file_get_info(camera_, path.folder, path.name, &info, context_->get_context());
    DBG() << "Capture: " << path.folder << " " << path.name << " " << info.file.size;

    CameraFile* cam_file = nullptr;
    status = gp_file_new(&cam_file);

    status = gp_camera_file_get(camera_, path.folder, path.name, GP_FILE_TYPE_NORMAL, cam_file, context_->get_context());
    if (status != 0) {
        ERR() << "gp_camera_file_get failed: " << status;
        throw std::runtime_error("Failed to capture");
    }

    const char *cam_file_data = nullptr;
    unsigned long cam_file_size;
    status = gp_file_get_data_and_size(cam_file, &cam_file_data, &cam_file_size);
    if (status != 0) {
        ERR() << "gp_file_get_data_and_size failed: " << status;
        throw std::runtime_error("Failed to capture");
    }

    std::vector<uint8_t> file_buf;
    file_buf.resize(info.file.size);
    memcpy(&file_buf[0], cam_file_data, cam_file_size);

    status = gp_camera_file_delete(camera_, path.folder, path.name, context_->get_context());
    gp_file_free(cam_file);

    return {path.name, std::move(file_buf)};
}

}