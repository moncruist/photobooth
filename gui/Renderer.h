#ifndef PHOTOBOOTH_RENDERER_H
#define PHOTOBOOTH_RENDERER_H

#include <opencv2/opencv.hpp>
#include <GLES2/gl2.h>
#include "camera/CameraInterface.h"
#include "AbstractEglGui.h"

namespace phb::gui {

class Renderer : public GuiListener {
public:
    void set_camera(camera::CameraInterface* camera);
    bool on_init(AbstractEglGui &context) override;
    void on_update(AbstractEglGui &context) override;
    void on_draw(AbstractEglGui &context) override;
    void on_keyboard_event(AbstractEglGui &context, char key) override;

private:
    GLuint load_shader(GLenum type, const char* shader_src);
    GLuint program_object_ {0};

    camera::CameraInterface* camera_ {nullptr};
    int64_t frame_number_ {0};
    cv::Mat frame_;
    GLuint frame_texture_ {0};
};

}

#endif //PHOTOBOOTH_RENDERER_H
