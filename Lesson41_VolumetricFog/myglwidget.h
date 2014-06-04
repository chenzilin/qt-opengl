#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
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
private:
    int extension_Init();

    void loadGLTexture();
private:
    bool m_show_full_screen;
    GLfloat m_camz;// 摄像机在Z方向的深度
    GLuint m_texture[1];// 纹理
};

#endif // MYGLWIDGET_H
