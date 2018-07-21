#ifndef PHOTOBOOTH_DSLRCAMERAINFO_H
#define PHOTOBOOTH_DSLRCAMERAINFO_H

#include <vector>
#include <string>
#include <gphoto2/gphoto2-camera.h>
#include "DslrContext.h"

namespace phb::dslr {

class DslrCameraInfo {
public:
    DslrCameraInfo(const char* name, const char* value);

    std::string get_name() const;
    std::string get_value() const;

    static std::vector<DslrCameraInfo> get_available_cameras(std::shared_ptr<DslrContext> context);

private:
    std::string name_;
    std::string value_;
};

}


#endif //PHOTOBOOTH_DSLRCAMERAINFO_H
