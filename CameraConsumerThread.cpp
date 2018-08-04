#include <cassert>
#include "util/TimeUtil.h"
#include "CameraConsumerThread.h"
#include "logging.h"

namespace phb {

CameraConsumerThread::CameraConsumerThread(camera::CameraInterface *interface, gui::FrameWidget *output)
    : cam_if_(interface), output_(output),
    face_cascade_(FACE_CASCADE_PATH),
    smile_cascade_(SMILE_CASCADE_PATH) {
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

            auto begin = util::now_ms();
            cv::Mat gray;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            std::vector<cv::Rect> faces;
            face_cascade_.detectMultiScale(gray, faces, 1.2, 5, CV_HAAR_SCALE_IMAGE, {50, 50});
            for (auto& face : faces) {
                cv::rectangle(frame, face, {0, 0, 255}, 2);

                cv::Mat roi(gray, face);
                std::vector<cv::Rect> smiles;
                smile_cascade_.detectMultiScale(roi, smiles, 1.7, 22, CV_HAAR_SCALE_IMAGE, {25, 25});
                for (auto& smile : smiles) {
                    cv::Rect smile_rec(face.x + smile.x, face.y + smile.y, smile.width, smile.height);
                    cv::rectangle(frame, smile_rec, {0, 255, 0}, 2);
                }
            }

            auto classified = util::now_ms() - begin;

            output_->updateFrame(frame);
            auto delta = util::now_ms() - begin;
            DBG() << "Total time: " << delta << ", classification time: " << classified << ", render: " << (delta - classified);

        } else {
            using namespace std::chrono;
            std::this_thread::sleep_for(5ms);
        }
    }
}

}