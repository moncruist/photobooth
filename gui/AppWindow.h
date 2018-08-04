#ifndef PHOTOBOOTH_APPWINDOW_H
#define PHOTOBOOTH_APPWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <opencv2/opencv.hpp>

#include "FrameWidget.h"

namespace phb::gui {

class AppWindow : public QWidget {
    Q_OBJECT
public:
    AppWindow(QWidget* parent = nullptr);

    FrameWidget* frameOutput() const;

public slots:
    void updateFrame(cv::Mat frame);

private:
    FrameWidget* frameOutput_ {nullptr};
    QVBoxLayout* layout_ {nullptr};
};

}

#endif //PHOTOBOOTH_APPWINDOW_H
