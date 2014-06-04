#include "myglwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGLFormat format = QGLFormat::defaultFormat();
    format.setSampleBuffers(true);
    MyGLWidget *w = new MyGLWidget(format, NULL);
    w->show();
    return a.exec();
}
