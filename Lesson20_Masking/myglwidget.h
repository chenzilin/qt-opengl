#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <GL/glaux.h>
#include <QMessageBox>
#include <QApplication>

class MyGLWidget : public QGLWidget
{
    Q_OBJECT

public:
    explicit MyGLWidget(QWidget *parent = 0);
    ~MyGLWidget();
protected:
    void resizeGL(int w, int h);

    void initializeGL();

    void paintGL();

    void keyPressEvent(QKeyEvent *event);

    void timerEvent(QTimerEvent *event);
private:
    void loadGLTexture();
private:
    bool m_show_full_screen;

    GLuint m_texture[5];
    bool m_masking;
    bool m_scene;
    GLfloat m_roll;
};

#endif // MYGLWIDGET_H
