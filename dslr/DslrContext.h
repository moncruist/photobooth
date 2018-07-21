#ifndef PHOTOBOOTH_DSLRCONTEXT_H
#define PHOTOBOOTH_DSLRCONTEXT_H

#include <memory>
#include <gphoto2/gphoto2-context.h>

namespace phb::dslr {

class DslrContext {
public:
    DslrContext();
    ~DslrContext();

    GPContext* get_context() const;

    static std::shared_ptr<DslrContext> make_context();
private:
    GPContext* context_{nullptr};
};

}


#endif //PHOTOBOOTH_DSLRCONTEXT_H
