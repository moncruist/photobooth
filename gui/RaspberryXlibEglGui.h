#ifndef PHOTOBOOTH_VIDEOCOREEGLGUI_H
#define PHOTOBOOTH_VIDEOCOREEGLGUI_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_brcm.h>

#include <string>
#include <memory>
#include <vector>

#include "AbstractEglGui.h"

namespace phb::gui {

class RaspberryXlibEglGui : public AbstractEglGui {
public:
    RaspberryXlibEglGui(const std::string& title, GuiListener* listener = nullptr);
    ~RaspberryXlibEglGui();

    void run() override;

    Display* get_x_display() const;
    Window get_x_window() const;

private:
    static constexpr size_t WIDTH = 1920;
    static constexpr size_t HEIGHT = 1080;

    void init_window(const std::string& title);
    void init_egl();

    void deinit_window();
    void deinit_egl();

    bool handle_user_events();
    void displayGLbuffer();
    static int64_t now_ms();

    static int xlib_error_handler(Display* display, XErrorEvent* event);

    Display* x_display_ {nullptr};
    Window window_ {None};
    Atom wm_delete_msg_ {None};
    GC x_gc_ {nullptr};
    std::vector<uint8_t> x_image_buf_;
    std::vector<unsigned int> gl_pixbuf_;
    XImage* x_image_ {nullptr};
};

}

#endif //PHOTOBOOTH_VIDEOCOREEGLGUI_H
