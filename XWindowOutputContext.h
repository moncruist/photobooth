#ifndef PHOTOBOOTH_XORGOUTPUT_H
#define PHOTOBOOTH_XORGOUTPUT_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <EGL/egl.h>

#include <string>
#include <mutex>
#include <chrono>

namespace phb {

class XWindowOutputContext;

class XWindowListenerInterface {
public:
    virtual ~XWindowListenerInterface() = default;

    virtual void on_update(XWindowOutputContext& context) = 0;
    virtual void on_draw(XWindowOutputContext& context) = 0;
    virtual void on_keyboard_event(XWindowOutputContext& context, char key) = 0;
};

class XWindowOutputContext {
public:
    XWindowOutputContext(const std::string& title, XWindowListenerInterface* listener = nullptr);
    ~XWindowOutputContext();

    void set_listener(XWindowListenerInterface* listener);

    Display* get_x_display() const;
    Window get_x_window() const;
    EGLDisplay get_egl_display() const;
    EGLContext get_egl_context() const;
    EGLSurface get_egl_surface() const;

    void run();

private:
    void init_window(const std::string& title);
    void init_egl();

    void deinit_window();
    void deinit_egl();

    bool handle_user_events();
    static int64_t now_ms();

    static int xlib_error_handler(Display* display, XErrorEvent* event);

    std::mutex mutex_;
    XWindowListenerInterface* listener_ {nullptr};
    Display* x_display_ {nullptr};
    Window window_ {None};
    Atom wm_delete_msg_ {None};
    EGLDisplay egl_display_ {EGL_NO_DISPLAY};
    EGLContext egl_context_ {EGL_NO_SURFACE};
    EGLSurface egl_surface_ {EGL_NO_CONTEXT};
};

}

#endif //PHOTOBOOTH_XORGOUTPUT_H
