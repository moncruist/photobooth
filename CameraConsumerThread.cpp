#include <cassert>
#include "CameraConsumerThread.h"
#include "logging.h"

namespace phb {

CameraConsumerThread::CameraConsumerThread(camera::CameraInterface *interface, gui::FrameWidget *output)
    : cam_if_(interface), output_(output) {
    assert(interface != nullptr);
    assert(output != nullptr);
    assert(interface->is_valid());
}

void CameraConsumerThread::start() {
    if (running_) {
        WARN() << "Already running";
        return;
    }

    running_ = true;
    thread_ = std::make_unique<std::thread>(std::bind(&CameraConsumerThread::run, this));
}

void CameraConsumerThread::stop() {
    running_ = false;
}

void CameraConsumerThread::join() {
    thread_->join();
}

void CameraConsumerThread::run() {
    int64_t prev_frame = 0;
    while (running_) {
        auto current_frame = cam_if_->frame_number();
        if (current_frame == 0 || prev_frame != current_frame) {
            prev_frame = current_frame;
            auto frame = cam_if_->get_frame();
            output_->updateFrame(frame);
        } else {
            using namespace std::chrono;
            std::this_thread::sleep_for(5ms);
        }
    }
}

}