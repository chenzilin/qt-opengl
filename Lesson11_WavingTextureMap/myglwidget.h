#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <GL/glaux.h>
#include <QMessageBox>
#include <QApplication>
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
private:
    bool m_show_full_screen;
    GLfloat m_x_rotate;
    GLfloat m_y_rotate;
    GLfloat m_z_rotate;
    GLuint m_texture[1];

    //我们将使用points数组来存放网格各顶点独立的x，y，z坐标。这里网格由45×45点形成，
    //换句话说也就是由44格×44格的小方格子依次组成了。
    float m_points[45][45][3]; // Points网格顶点数组
};

#endif // MYGLWIDGET_H
