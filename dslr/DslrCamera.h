#ifndef PHOTOBOOTH_DSLRMANAGER_H
#define PHOTOBOOTH_DSLRMANAGER_H

#include <gphoto2/gphoto2.h>
#include <memory>
#include <cstdint>
#include <vector>
#include <string>
#include "DslrContext.h"

namespace phb::dslr {

struct CapturedFile {
    std::string name;
    std::vector<uint8_t> data;
};

class DslrCamera {
public:
    explicit DslrCamera(std::shared_ptr<DslrContext> context);
    ~DslrCamera();

    CapturedFile capture();

private:
    std::shared_ptr<DslrContext> context_;
    Camera* camera_ {nullptr};
};

}

#endif //PHOTOBOOTH_DSLRMANAGER_H
