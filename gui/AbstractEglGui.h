#ifndef PHOTOBOOTH_OUTPUTINTERFACE_H
#define PHOTOBOOTH_OUTPUTINTERFACE_H

#include <EGL/egl.h>

namespace phb::gui {

class AbstractEglGui;

class GuiListener {
public:
    virtual ~GuiListener() = default;

    virtual bool on_init(AbstractEglGui& context) = 0;
    virtual void on_update(AbstractEglGui& context) = 0;
    virtual void on_draw(AbstractEglGui& context) = 0;
    virtual void on_keyboard_event(AbstractEglGui& context, char key) = 0;
};

class AbstractEglGui {
public:
    explicit AbstractEglGui(GuiListener* listener = nullptr);
    virtual ~AbstractEglGui() = default;

    void set_listener(GuiListener* listener);

    EGLDisplay get_egl_display() const;
    EGLContext get_egl_context() const;
    EGLSurface get_egl_surface() const;

    virtual int get_width() = 0;
    virtual int get_height() = 0;

    virtual void run() = 0;

protected:
    GuiListener* listener_ {nullptr};
    EGLDisplay egl_display_ {EGL_NO_DISPLAY};
    EGLContext egl_context_ {EGL_NO_SURFACE};
    EGLSurface egl_surface_ {EGL_NO_CONTEXT};

};

}

#endif //PHOTOBOOTH_OUTPUTINTERFACE_H
