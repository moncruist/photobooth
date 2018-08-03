#ifndef PHOTOBOOTH_APPWINDOW_H
#define PHOTOBOOTH_APPWINDOW_H

#include <QWidget>

namespace phb::gui {

class AppWindow : public QWidget {
    Q_OBJECT
public:
    AppWindow(QWidget* parent = nullptr);
};

}

#endif //PHOTOBOOTH_APPWINDOW_H
