#include <stdexcept>
#include <chrono>
#include <vector>


#include "RaspberryXlibEglGui.h"
#include "logging.h"

namespace phb::gui {

RaspberryXlibEglGui::RaspberryXlibEglGui(const std::string& title, GuiListener* listener)
    : AbstractEglGui(listener) {
    init_window(title);
    try {
        init_egl();
    } catch (...) {
        deinit_window();
        throw;
    }
}

RaspberryXlibEglGui::~RaspberryXlibEglGui() {
    deinit_egl();
    deinit_window();
}

void RaspberryXlibEglGui::init_window(const std::string& title) {
    INFO() << "Initializing X window";
    // Establish connection to Xorg server
    x_display_ = XOpenDisplay(NULL);
    if (x_display_ == nullptr) {
        ERR() << "Failed to open display";
        throw std::runtime_error("Failed to open display");
    }

    XSetErrorHandler(&RaspberryXlibEglGui::xlib_error_handler);

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

    x_gc_ = DefaultGC(x_display_, 0);

    XWindowAttributes win_attrs;
    XGetWindowAttributes(x_display_, window_, &win_attrs);

    x_image_buf_.resize(win_attrs.width * win_attrs.height * 4);
    gl_pixbuf_.resize(win_attrs.width * win_attrs.height);

    char* buf_ptr = reinterpret_cast<char*>(x_image_buf_.data());
    x_image_ = XCreateImage(x_display_,
                            DefaultVisual(x_display_, DefaultScreen(x_display_)),
                            DefaultDepth(x_display_, DefaultScreen(x_display_)),
                            ZPixmap, 0, buf_ptr,
                            win_attrs.width, win_attrs.height, 32, 0);

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

void RaspberryXlibEglGui::init_egl() {
    INFO() << "Initializing EGL";

    egl_display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
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
        EGL_RED_SIZE,       8,
        EGL_GREEN_SIZE,     8,
        EGL_BLUE_SIZE,      8,
        EGL_ALPHA_SIZE,     8,
        EGL_SURFACE_TYPE,
        EGL_PIXMAP_BIT | EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    EGLConfig config;
    if (!eglChooseConfig(egl_display_, eglAttrs.data(), &config, 1, &num_configs)) {
        const char* msg = "Failed to choose EGL config";
        ERR() << msg;
        deinit_egl();
        throw std::runtime_error(msg);
    }

    EGLint rt;
    EGLint pixel_format = EGL_PIXEL_FORMAT_ARGB_8888_BRCM;
    eglGetConfigAttrib(egl_display_, &config, EGL_RENDERABLE_TYPE, &rt);

    if (rt & EGL_OPENGL_ES_BIT) {
        pixel_format |= EGL_PIXEL_FORMAT_RENDER_GLES_BRCM;
        pixel_format |= EGL_PIXEL_FORMAT_GLES_TEXTURE_BRCM;
    }
    if (rt & EGL_OPENGL_ES2_BIT) {
        pixel_format |= EGL_PIXEL_FORMAT_RENDER_GLES2_BRCM;
        pixel_format |= EGL_PIXEL_FORMAT_GLES2_TEXTURE_BRCM;
    }
    if (rt & EGL_OPENVG_BIT) {
        pixel_format |= EGL_PIXEL_FORMAT_RENDER_VG_BRCM;
        pixel_format |= EGL_PIXEL_FORMAT_VG_IMAGE_BRCM;
    }
    if (rt & EGL_OPENGL_BIT) {
        pixel_format |= EGL_PIXEL_FORMAT_RENDER_GL_BRCM;
    }

    EGLint pixmap[5];
    pixmap[0] = 0;
    pixmap[1] = 0;
    pixmap[2] = WIDTH;
    pixmap[3] = HEIGHT;
    pixmap[4] = pixel_format;

    eglCreateGlobalImageBRCM(WIDTH, HEIGHT, pixmap[4], 0, WIDTH*4, pixmap);

    egl_surface_ = eglCreatePixmapSurface(egl_display_, config, pixmap, nullptr);
    if (egl_surface_ == EGL_NO_SURFACE) {
        const char* msg = "Failed to create EGL surface";
        ERR() << msg;
        deinit_egl();
        throw std::runtime_error(msg);
    }

    EGLint contextAttrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
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

void RaspberryXlibEglGui::deinit_window() {
    if (x_display_) {
        XDestroyWindow(x_display_, window_);
        XSetErrorHandler(nullptr);
        XCloseDisplay(x_display_);
        x_display_ = nullptr;
    }
}

void RaspberryXlibEglGui::deinit_egl() {
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

#define SHIFTGL_R       ( 0)
#define SHIFTGL_G       ( 8)
#define SHIFTGL_B       (16)
#define SHIFTGL_A       (24)

#define RVALGL(l)       ((int)(((l)>>SHIFTGL_R)&0xff))
#define GVALGL(l)       ((int)(((l)>>SHIFTGL_G)&0xff))
#define BVALGL(l)       ((int)(((l)>>SHIFTGL_B)&0xff))
#define AVALGL(l)       ((int)(((l)>>SHIFTGL_A)&0xff))

#define CPACKGL(r,g,b,a)  (((r)<<SHIFTGL_R) | ((g)<<SHIFTGL_G) | ((b)<<SHIFTGL_B) | ((a)<<SHIFTGL_A))

#define SHIFTX_R         (16)
#define SHIFTX_G         ( 8)
#define SHIFTX_B         ( 0)
#define SHIFTX_A         (24)

#define CPACKX(r,g,b,a)  (((r)<<SHIFTX_R) | ((g)<<SHIFTX_G) | ((b)<<SHIFTX_B) | ((a)<<SHIFTX_A))

void RaspberryXlibEglGui::displayGLbuffer()
{
    glFinish();
    glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, gl_pixbuf_.data());
    int y;
    unsigned int *pixptr = gl_pixbuf_.data();
    for(size_t y=0; y < HEIGHT; y++) {
        int dsty = HEIGHT -1 -y;          // flip y for X windows
        unsigned int *dptr = (unsigned int *)(&(x_image_->data[0]));
        dptr = dptr + (dsty*WIDTH);
        int x = WIDTH;
        while(x--) {
            int p = *pixptr++;
            *dptr++ = CPACKX(RVALGL(p), GVALGL(p), BVALGL(p), 255);
        }
    }

    XPutImage(x_display_, window_, x_gc_, x_image_, 0, 0, 0, 0, WIDTH, HEIGHT);
}

int RaspberryXlibEglGui::xlib_error_handler(Display *display, XErrorEvent *event) {
    static std::array<char, 256> msg_buf;
    XGetErrorText(display, event->error_code, msg_buf.data(), msg_buf.size());

    ERR() << "XLib error: " << static_cast<int>(event->error_code) << "(" << msg_buf.data() << ")";
    return 0;
}

Display* RaspberryXlibEglGui::get_x_display() const {
    return x_display_;
}

Window RaspberryXlibEglGui::get_x_window() const {
    return window_;
}

void RaspberryXlibEglGui::run() {
    auto prev_frame = now_ms();
    int64_t total_time = 0;
    unsigned int frames = 0;

    if (listener_) {
        if (!listener_->on_init(*this)) {
            ERR() << "Failed to init renderer";
            return;
        }
    }

    while (handle_user_events()) {
        if (listener_) {
            listener_->on_update(*this);
            listener_->on_draw(*this);
        }

        eglSwapBuffers(egl_display_, egl_surface_);
        displayGLbuffer();
        auto current_time = now_ms();
        auto delta = current_time - prev_frame;

        prev_frame = current_time;

        total_time += delta;
        frames++;

        if (total_time > 10000) {
            double fps = static_cast<double>(frames) * 1000 / static_cast<double>(total_time);
            DBG() << frames << " frames rendered in " << total_time << " milliseconds. FPS=" << fps;
            total_time -= 10000;
            frames = 0;
        }
    }
}

bool RaspberryXlibEglGui::handle_user_events() {
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

int64_t RaspberryXlibEglGui::now_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}


}
