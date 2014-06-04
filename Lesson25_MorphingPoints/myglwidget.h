#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QTextStream>
#include <QKeyEvent>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>

class MyGLWidget : public QGLWidget
{
    Q_OBJECT

public:
    //下面的结构定义一个三维顶点
    typedef struct
    {
        float x, y, z;
    } VERTEX;
    //下面的结构使用顶点来描述一个三维物体
    typedef	struct	// 物体结构
    {
        int verts;// 物体中顶点的个数
        VERTEX *points;// 包含顶点数据的指针
    } OBJECT;

    explicit MyGLWidget(QWidget *parent = 0);
    ~MyGLWidget();
protected:
    void resizeGL(int w, int h);

    void initializeGL();

    void paintGL();

    void keyPressEvent(QKeyEvent *event);

    void timerEvent(QTimerEvent *event);
private:
    void objallocate(OBJECT *k,int n);

    void objfree(OBJECT *k);

    void readstr(QTextStream *stream, QString &string);

    void objload(QString name, OBJECT *k);

    VERTEX calculate(int i);
private:
    bool m_show_full_screen;
    //我们结下来添加几个旋转变量，用来记录旋转的信息。并使用cx,cy,cz设置物体在屏幕上的位置。
    //变量key用来记录当前的模型，step用来设置相邻变形之间的中间步骤。如step为200，则需要200次，才能把一个物体变为另一个物体。
    //最后我们用一个变量来设置是否使用变形。
    GLfloat	m_xrot;
    GLfloat m_yrot;
    GLfloat m_zrot; // X, Y & Z 轴的旋转角度
    GLfloat m_xspeed;
    GLfloat m_yspeed;
    GLfloat m_zspeed;// X, Y & Z 轴的旋转速度
    GLfloat m_cx;
    GLfloat m_cy;
    GLfloat m_cz;// 物体的位置
    int	m_key;// 物体的标识符
    int	m_step;
    int m_steps;// 变换的步数
    bool m_morph;// 是否使用变形
    //maxver用来记录各个物体中最大的顶点数，如一个物体使用5个顶点，另一个物体使用20个顶点，那么物体的顶点个数为20。
    //结下来定义了四个我们使用的模型物体，并把相邻模型变形的中间状态保存在helper中，sour保存原模型物体，dest保存将要变形的模型物体。
    int	m_maxver;// 最大的顶点数
    OBJECT m_morph1;
    OBJECT m_morph2;
    OBJECT m_morph3;
    OBJECT m_morph4;// 我们的四个物体
    OBJECT m_helper;
    OBJECT *m_sour;
    OBJECT *m_dest;// 帮助物体,原物体，目标物体
};

#endif // MYGLWIDGET_H
