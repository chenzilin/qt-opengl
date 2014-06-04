#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>
#include <qmath.h>

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
    GLuint emptyTexture();

    void reduceToUnit(float vector[3]);

    void calcNormal(float v[3][3], float out[3]);

    void viewOrtho();

    void viewPerspective();

    void renderToTexture();

    void processHelix();

    void drawBlur(int times, float inc);
private:
    float m_angle;// 用来旋转那个螺旋
    float m_vertexes[4][3];// 为3个设置的顶点保存浮点信息
    float m_normal[3];// 存放法线数据的数组
    GLuint m_blurTexture;// 存放纹理编号的一个无符号整型
};

#endif // MYGLWIDGET_H
