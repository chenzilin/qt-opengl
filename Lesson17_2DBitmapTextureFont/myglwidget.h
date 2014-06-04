#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>
#include <QtEndian>
#include <math.h>

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

    void buildFont(); // 创建我们的字符显示列表

    void glPrint(GLint x, GLint y, char *string, int set);
private:
    bool m_show_full_screen;

    GLuint m_base;
    GLuint m_texture[2];
    GLfloat m_cnt1;
    GLfloat m_cnt2;
};

#endif // MYGLWIDGET_H
