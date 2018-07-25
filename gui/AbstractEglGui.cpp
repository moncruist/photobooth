#include "AbstractEglGui.h"

namespace phb::gui {

AbstractEglGui::AbstractEglGui(GuiListener *listener)
    : listener_(listener) {

}

void AbstractEglGui::set_listener(GuiListener *listener) {
    listener_ = listener;
}

EGLDisplay AbstractEglGui::get_egl_display() const {
    return egl_display_;
}

EGLContext AbstractEglGui::get_egl_context() const {
    return egl_context_;
}

EGLSurface AbstractEglGui::get_egl_surface() const {
    return egl_surface_;
}

}