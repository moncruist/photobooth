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
    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> running_ {false};

    camera::CameraInterface* cam_if_;
    gui::FrameWidget* output_;
};

}

#endif //PHOTOBOOTH_CAMERACONSUMERTHREAD_H
