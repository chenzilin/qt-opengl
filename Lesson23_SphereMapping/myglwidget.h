#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <GL/glaux.h>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>

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

    void glDrawCube();
private:
    bool m_show_full_screen;
    bool m_light;

    GLfloat m_x_rotate;
    GLfloat m_y_rotate;
    GLfloat m_x_speed;
    GLfloat m_y_speed;
    GLfloat m_z;

    GLfloat m_light_ambient[4];
    GLfloat m_light_diffuse[4];
    GLfloat m_light_position[4];

    GLuint m_filter;
    GLuint m_texture[6];

    GLUquadricObj *m_quadratic;					// 二次几何体
    int	m_part1;						// 圆盘的起始角度
    int	m_part2;						// 圆盘的结束角度
    int	m_p1;						// 增量1
    int	m_p2;						// 增量1
    GLuint m_object;						// 二次几何体标示符
};

#endif // MYGLWIDGET_H
