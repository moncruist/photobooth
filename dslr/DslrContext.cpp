#include "DslrContext.h"

namespace phb::dslr {

DslrContext::DslrContext() {
    context_ = gp_context_new();
}

DslrContext::~DslrContext() {
    if (context_) {
        gp_context_unref(context_);
    }
}

GPContext* DslrContext::get_context() const {
    return context_;
}

std::shared_ptr<DslrContext> DslrContext::make_context() {
    return std::make_shared<DslrContext>();
}

}