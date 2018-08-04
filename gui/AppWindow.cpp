#include "AppWindow.h"

namespace phb::gui {

AppWindow::AppWindow(QWidget *parent) : QWidget(parent) {
    layout_ = new QVBoxLayout();

    frameOutput_ = new FrameWidget(this);
    layout_->addWidget(frameOutput_);

    setLayout(layout_);
}

FrameWidget *AppWindow::frameOutput() const {
    return frameOutput_;
}

void AppWindow::updateFrame(cv::Mat frame) {
    frameOutput_->updateFrame(frame);
}

}