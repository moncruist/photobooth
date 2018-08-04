#ifndef PHOTOBOOTH_CAMERACONSUMERTHREAD_H
#define PHOTOBOOTH_CAMERACONSUMERTHREAD_H

#include <memory>
#include <thread>
#include <atomic>
#include "camera/CameraInterface.h"
#include "gui/FrameWidget.h"

namespace phb {

class CameraConsumerThread {
public:
    CameraConsumerThread(camera::CameraInterface* interface, gui::FrameWidget* output);

    void start();
    void stop();

    void join();

private:
    void run();
private:

#ifdef __APPLE__
    static constexpr const char* FACE_CASCADE_PATH = "/usr/local/Cellar/opencv/3.4.2/share/OpenCV/haarcascades/haarcascade_frontalface_default.xml";
    static constexpr const char* SMILE_CASCADE_PATH = "/usr/local/Cellar/opencv/3.4.2/share/OpenCV/haarcascades/haarcascade_smile.xml";
#else
    static constexpr const char* FACE_CASCADE_PATH = "/usr/local/share/OpenCV/haarcascades/haarcascade_frontalface_default.xml";
    static constexpr const char* SMILE_CASCADE_PATH = "/usr/local/share/OpenCV/haarcascades/haarcascade_smile.xml";
#endif

    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> running_ {false};

    camera::CameraInterface* cam_if_;
    gui::FrameWidget* output_;

    cv::CascadeClassifier face_cascade_;
    cv::CascadeClassifier smile_cascade_;
};

}

#endif //PHOTOBOOTH_CAMERACONSUMERTHREAD_H
