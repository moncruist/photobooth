#include <QPainter>
#include "FrameWidget.h"

namespace phb::gui {

FrameWidget::FrameWidget(QWidget *parent) : QWidget(parent) {

}

void FrameWidget::updateFrame(cv::Mat frame) {
    cv::Mat dest;
//    cvtColor(frame, dest, CV_BGR2RGB);
    paint_ = QImage(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    update();
}

void FrameWidget::paintEvent(QPaintEvent *event) {
    QRectF target(0.0, 0.0, width(), height());
    QRectF source(0.0, 0.0, paint_.width(), paint_.height());

    QPainter painter(this);
    painter.drawImage(target, paint_, source);
}

}