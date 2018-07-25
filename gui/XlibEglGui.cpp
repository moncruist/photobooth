#include <stdexcept>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <chrono>
#include "XlibEglGui.h"
#include "logging.h"

namespace phb::gui {

XlibEglGui::XlibEglGui(const std::string& title, GuiListener* listener)
    : AbstractEglGui(listener) {
    init_window(title);
    try {
        init_egl();
    } catch (...) {
        deinit_window();
        throw;
    }
}

XlibEglGui::~XlibEglGui() {
    deinit_egl();
    deinit_window();
}

void XlibEglGui::init_window(const std::string& title) {
    INFO() << "Initializing X window";
    // Establish connection to Xorg server
    x_display_ = XOpenDisplay(NULL);
    if (x_display_ == nullptr) {
        ERR() << "Failed to open display";
        throw std::runtime_error("Failed to open display");
    }

    XSetErrorHandler(&XlibEglGui::xlib_error_handler);

    // Create window
    Window root = DefaultRootWindow(x_display_);

    XSetWindowAttributes attrs;
    memset(&attrs, 0, sizeof(attrs));

    attrs.event_mask = ExposureMask | PointerMotionMask | KeyPressMask;
    attrs.override_redirect = False;
    window_ = XCreateWindow(x_display_, root,
                            0, 0, 1920, 1080, 0,
                            CopyFromParent, InputOutput,
                            CopyFromParent,
                            CWEventMask | CWOverrideRedirect, &attrs);

    // Setting hints for window manager
    XWMHints hints {};
    hints.input = True;         // rely on window manager
    hints.flags = InputHint;    // input
    XSetWMHints(x_display_, window_, &hints);

    // Show window
    XMapWindow(x_display_, window_);
    XStoreName(x_display_, window_, title.c_str());

    wm_delete_msg_ = XInternAtom(x_display_, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(x_display_, window_, &wm_delete_msg_, 1);

    Atom wm_state = XInternAtom(x_display_, "_NET_WM_STATE", False);
    XEvent xev {};
    xev.type = ClientMessage;
    xev.xclient.window = window_;
    xev.xclient.message_type = wm_state;
    xev.xclient.format       = 32;
    xev.xclient.data.l[0]    = 1;
    xev.xclient.data.l[1]    = False;
    XSendEvent (x_display_, root, False,
                SubstructureNotifyMask,
                &xev );
}

void XlibEglGui::init_egl() {
    INFO() << "Initializing EGL";

    EGLint contextAttrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };

    egl_display_ = eglGetDisplay(static_cast<EGLNativeDisplayType>(x_display_));
    if (egl_display_ == EGL_NO_DISPLAY) {
        const char* msg = "Failed to get EGL display";
        ERR() << msg;
        throw std::runtime_error(msg);
    }

    EGLint major_version, minor_version;
    if (!eglInitialize(egl_display_, &major_version, &minor_version)) {
        const char* msg = "Failed to initialize EGL";
        ERR() << msg;
        throw std::runtime_error(msg);
    }

    INFO() << "EGL version: " << major_version << "." << minor_version;

    EGLint num_configs;
    if (!eglGetConfigs(egl_display_, nullptr, 0, &num_configs)) {
        const char* msg = "Failed to get EGL configs";
        ERR() << msg;
        deinit_egl();
        throw std::runtime_error(msg);
    }

    std::vector<EGLint> eglAttrs {
        EGL_RED_SIZE,       5,
        EGL_GREEN_SIZE,     6,
        EGL_BLUE_SIZE,      5,
        EGL_ALPHA_SIZE,     EGL_DONT_CARE,
        EGL_DEPTH_SIZE,     EGL_DONT_CARE,
        EGL_STENCIL_SIZE,   EGL_DONT_CARE,
        EGL_SAMPLE_BUFFERS, 0,
        EGL_NONE
    };

    EGLConfig config;
    if (!eglChooseConfig(egl_display_, eglAttrs.data(), &config, 1, &num_configs)) {
        const char* msg = "Failed to choose EGL config";
        ERR() << msg;
        deinit_egl();
        throw std::runtime_error(msg);
    }

    egl_surface_ = eglCreateWindowSurface(egl_display_, config, window_, nullptr);
    if (egl_surface_ == EGL_NO_SURFACE) {
        const char* msg = "Failed to create EGL surface";
        ERR() << msg;
        deinit_egl();
        throw std::runtime_error(msg);
    }

    egl_context_ = eglCreateContext(egl_display_, config, EGL_NO_CONTEXT, contextAttrs);
    if (egl_context_ == EGL_NO_CONTEXT) {
        const char* msg = "Failed to create EGL context";
        ERR() << msg;
        deinit_egl();
        throw std::runtime_error(msg);
    }

    if (!eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_)) {
        const char* msg = "Failed to select EGL";
        ERR() << msg;
        deinit_egl();
        throw std::runtime_error(msg);
    }
}

void XlibEglGui::deinit_window() {
    if (x_display_) {
        XDestroyWindow(x_display_, window_);
        XSetErrorHandler(nullptr);
        XCloseDisplay(x_display_);
        x_display_ = nullptr;
    }
}

void XlibEglGui::deinit_egl() {
    if (egl_display_) {
        if (egl_context_) {
            eglDestroyContext(egl_display_, egl_context_);
            egl_context_ = EGL_NO_CONTEXT;
        }

        if (egl_surface_) {
            eglDestroySurface(egl_display_, egl_surface_);
            egl_surface_ = EGL_NO_SURFACE;
        }

        eglTerminate(egl_display_);
        egl_display_ = EGL_NO_DISPLAY;
    }
}

int XlibEglGui::xlib_error_handler(Display *display, XErrorEvent *event) {
    static std::array<char, 256> msg_buf;
    XGetErrorText(display, event->error_code, msg_buf.data(), msg_buf.size());

    ERR() << "XLib error: " << static_cast<int>(event->error_code) << "(" << msg_buf.data() << ")";
    return 0;
}

Display* XlibEglGui::get_x_display() const {
    return x_display_;
}

Window XlibEglGui::get_x_window() const {
    return window_;
}

void XlibEglGui::run() {
    auto prev_frame = now_ms();
    int64_t total_time = 0;
    unsigned int frames = 0;

    while (handle_user_events()) {
        if (listener_) {
            listener_->on_update(*this);
            listener_->on_draw(*this);
        }

        eglSwapBuffers(egl_display_, egl_surface_);
        auto current_time = now_ms();
        auto delta = current_time - prev_frame;

        prev_frame = current_time;

        total_time += delta;
        frames++;

        if (total_time > 10000) {
            DBG() << frames << " frames rendered in " << total_time << " milliseconds. FPS=" << static_cast<double>(frames) / static_cast<double>(total_time);
            total_time -= 10000;
            frames = 0;
        }
    }
}

bool XlibEglGui::handle_user_events() {
    XEvent event;
    KeySym key;
    char text;

    bool should_run = true;

    while (XPending(x_display_)) {
        XNextEvent(x_display_, &event);

        switch (event.type) {
            case KeyPress:
                if (XLookupString(&event.xkey, &text, 1, &key, 0) == 1) {
                    if (listener_) {
                        listener_->on_keyboard_event(*this, text);
                    }
                }
                break;

            case DestroyNotify:
                should_run = false;
                break;

            case ClientMessage:
                if (event.xclient.data.l[0] == wm_delete_msg_) {
                    should_run = false;
                }
                break;

            default:
                break;
        }
    }

    return should_run;
}

int64_t XlibEglGui::now_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

}