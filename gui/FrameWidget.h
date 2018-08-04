#ifndef PHOTOBOOTH_FRAMEWIDGET_H
#define PHOTOBOOTH_FRAMEWIDGET_H

#include <QGLWidget>
#include <opencv2/opencv.hpp>

namespace phb::gui {

class FrameWidget : public QGLWidget {
public:
    FrameWidget(QWidget* parent = nullptr);

public slots:
    void updateFrame(cv::Mat frame);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QImage paint_;
};

}

#endif //PHOTOBOOTH_FRAMEWIDGET_H
