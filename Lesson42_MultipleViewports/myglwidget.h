#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>
#include <QTime>

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
    void updateTex(int dmx, int dmy);

    void reset();
private:
    bool m_show_full_screen;
    //然后我们声明一些我们打算在整个程序中都要用到的全局变量。
    //mx和my纪录了当前所在迷宫中的房间。每个房间都被墙隔开(因此房间都是2个单元大小的部分）。
    //with和height是用来建立纹理需要的。它也是迷宫的宽和高。让迷宫和贴图的大小一致的原因是使迷宫中的象素和纹理中的象素一一对应。
    //我倾向于把宽和高都设成256,尽管这要花更长的时间来建立迷宫。
    //如果你的显卡能支持处理大型贴图。可以试着以2次幂增加这个值(256, 512, 1023）。确保这个值不至于太大。
    //如果这个主窗口的宽度有1024个象素，并且每个视口的大小都是主窗口的一半，相应的你应该设置你的贴图宽度也是窗口宽度的一半。
    //如果你使贴图宽度为1024象素，但你的视口大小只有512,空间不足于容纳贴图中所有得象素，这样每两个象素就会重叠在一起。贴图的高度也作同样处理:高度是窗口高度的1/2.
    //当然你还必须四舍五入到2的幂。
    int m_mx;
    int m_my;
    //r[4]保存了4个随机的红色分量值，g[4]保存了4个随机的绿色分量值，b[4]保存了4个随机的兰色分量值。
    //这些值赋给各个视口不同的颜色。第一个视口颜色为r[0],g[0],b[0]。
    //请注意每一个颜色都是一个字节的值，而不是常用的浮点值。我这里用字节是因为产生0-255的随机值比产生0.0f-1.0f的浮点值更容易。
    //tex_data指向我们的贴图数据。
    unsigned char m_r[4];
    unsigned char m_g[4];
    unsigned char m_b[4];// 随机的颜色
    unsigned char *m_tex_data;//保存纹理数据
    //xrot,yrot和zrot是旋转3d物体用到的变量。
    //最后，我们声明一个二次曲面物体，这样我们可以用gluCylinder和gluSphere来画圆柱和球体，这比手工绘制这些物体容易多了。
    GLfloat m_xrot;
    GLfloat m_yrot;
    GLfloat m_zrot;// 旋转物体
    GLUquadricObj *m_quadric;// 二次几何体对象
};

#endif // MYGLWIDGET_H
