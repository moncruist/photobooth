#include <cassert>
#include "DslrCameraInfo.h"
#include "logging.h"

namespace phb::dslr {

DslrCameraInfo::DslrCameraInfo(const char* name, const char* value) {
    if (name) {
        name_ = name;
    }
    if (value) {
        value_ = value;
    }
}

std::string DslrCameraInfo::get_name() const {
    return name_;
}

std::string DslrCameraInfo::get_value() const {
    return value_;
}

std::vector<DslrCameraInfo> DslrCameraInfo::get_available_cameras(std::shared_ptr<DslrContext> context) {
    std::vector<DslrCameraInfo> result;

    CameraList *list {nullptr};
    gp_list_new(&list);

    gp_camera_autodetect(list, context->get_context());
    result.reserve(gp_list_count(list));

    for (int i = 0; i < gp_list_count(list); i++) {
        const char *name, *value;
        gp_list_get_name(list, i, &name);
        gp_list_get_value(list, i, &value);

        result.push_back({name, value});
    }
    gp_list_free(list);


    return std::vector<DslrCameraInfo>();
}

}