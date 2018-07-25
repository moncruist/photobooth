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

#include "AbstractEglGui.h"

namespace phb::gui {

class XlibEglGui : public AbstractEglGui {
public:
    XlibEglGui(const std::string& title, GuiListener* listener = nullptr);
    ~XlibEglGui();

    Display* get_x_display() const;
    Window get_x_window() const;

    void run() override ;

private:
    void init_window(const std::string& title);
    void init_egl();

    void deinit_window();
    void deinit_egl();

    bool handle_user_events();
    static int64_t now_ms();

    static int xlib_error_handler(Display* display, XErrorEvent* event);

    Display* x_display_ {nullptr};
    Window window_ {None};
    Atom wm_delete_msg_ {None};
};

}

#endif //PHOTOBOOTH_XORGOUTPUT_H
