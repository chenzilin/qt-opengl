#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>
#include <QTextStream>
#include <math.h>

class MyGLWidget : public QGLWidget
{
    Q_OBJECT
public:
    //三角形本质上是由一些(两个以上)顶点组成的多边形，顶点同时也是我们的最基本的分类单位。
    //顶点包含了OpenGL真正感兴趣的数据。我们用3D空间中的坐标值(x,y,z)以及它们的纹理坐标(u,v)来定义三角形的每个顶点。
    typedef struct tagVERTEX						// 创建Vertex顶点结构
    {
        float x, y, z;							// 3D 坐标
        float u, v;							// 纹理坐标
    } VERTEX;								// 命名为VERTEX
    //一个sector(区段)包含了一系列的多边形，所以下一个目标就是triangle(我们将只用三角形，这样写代码更容易些)。
    typedef struct tagTRIANGLE						// 创建Triangle三角形结构
    {
        VERTEX vertex[3];						// VERTEX矢量数组，大小为3
    }TRIANGLE;// 命名为 TRIANGLE
    typedef struct tagSECTOR						// 创建Sector区段结构
    {
        int numtriangles;						// Sector中的三角形个数
        TRIANGLE* triangle;						// 指向三角数组的指针
    } SECTOR;								// 命名为SECTOR

    explicit MyGLWidget(QWidget *parent = 0);
    ~MyGLWidget();
protected:
    void resizeGL(int w, int h);

    void initializeGL();

    void paintGL();

    void keyPressEvent(QKeyEvent *event);
private:
    void setupWorld();

    void readStr(QTextStream *stream, QString &string);

    void loadTexture();
private:
    bool m_show_full_screen;

    SECTOR m_sector1;

    GLfloat m_yrot;
    GLfloat m_xpos;
    GLfloat m_zpos;
    GLfloat m_heading;
    GLfloat m_walkbias;
    GLfloat m_walkbiasangle;
    GLfloat m_lookupdown;

    GLuint	m_filter;
    GLuint	m_texture[3];

    bool m_blend;
};

#endif // MYGLWIDGET_H
