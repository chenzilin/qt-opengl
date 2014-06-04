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

    void timerEvent(QTimerEvent *event);
private:
    void loadGLTexture();

    void drawObject();

    void drawFloor();
private:
    bool m_show_full_screen;
    //下面用二次几何体创建一个球，并设置纹理
    GLUquadricObj *m_q;// 使用二次几何体创建球
    GLfloat	m_xrot;// X方向的旋转角度
    GLfloat	m_yrot;// Y方向的旋转角的
    GLfloat	m_xrotspeed;// X方向的旋转速度
    GLfloat	m_yrotspeed;// Y方向的旋转速度
    GLfloat	m_zoom;// 移入屏幕7个单位
    GLfloat	m_height;// 球离开地板的高度
    GLuint	m_textures[3];// 使用三个纹理
};

#endif // MYGLWIDGET_H
