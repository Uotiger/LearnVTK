#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>
#include "MainRenderWindow.h"

int main(int argc, char* argv[]) {
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    QApplication app(argc, argv);

    MainRenderWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
