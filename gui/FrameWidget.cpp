#include <QPainter>
#include "FrameWidget.h"
#include "util/TimeUtil.h"
#include "logging.h"

namespace phb::gui {

FrameWidget::FrameWidget(QWidget *parent) : QGLWidget(parent) {

}

void FrameWidget::updateFrame(cv::Mat frame) {
    cv::Mat dest;
    cvtColor(frame, dest, CV_BGR2RGB);
    paint_ = QImage(dest.data, dest.cols, dest.rows, dest.step, QImage::Format_RGB888);
    update();
}

void FrameWidget::paintEvent(QPaintEvent *event) {
    QRectF target(0.0, 0.0, width(), height());
    QRectF source(0.0, 0.0, paint_.width(), paint_.height());

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, 1);
    painter.drawImage(target, paint_, source);
}

}