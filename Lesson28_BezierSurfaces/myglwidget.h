#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/gl.h>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>
#include <math.h>

typedef struct point_3d
{
    // 3D点的结构
    double x, y, z;
} POINT_3D;

class MyGLWidget : public QGLWidget
{
    Q_OBJECT
public:
    typedef struct bpatch
    {
        // 贝塞尔面片结构
        POINT_3D anchors[4][4];// 由4x4网格组成
        GLuint dlBPatch;// 绘制面片的显示列表名称
        GLuint texture;// 面片的纹理
    } BEZIER_PATCH;

    explicit MyGLWidget(QWidget *parent = 0);
    ~MyGLWidget();
protected:
    void resizeGL(int w, int h);

    void initializeGL();

    void paintGL();

    void keyPressEvent(QKeyEvent *event);

    void timerEvent(QTimerEvent *event);
private:
    GLuint genBezier(BEZIER_PATCH patch, int divs);

    void initBezier();

    void loadGLTexture();
private:
    bool m_show_full_screen;
    BEZIER_PATCH m_mybezier;// 创建一个贝塞尔曲面结构
    bool m_showCPoints;// 是否显示控制点
    int m_divs;// 细分精度，控制曲面的显示精度
    GLfloat m_rotz;
};

#endif // MYGLWIDGET_H
