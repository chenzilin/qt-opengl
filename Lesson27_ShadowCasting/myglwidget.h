#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>
#include <QTextStream>

class MyGLWidget : public QGLWidget
{
    Q_OBJECT

public:
    //下面定义一个3D顶点结构
    struct sPoint
    {
        GLfloat x, y, z;
    };
    //定义一个平面结构
    // 平面方程为: ax + by + cz + d = 0
    struct sPlaneEq
    {
        GLfloat a, b, c, d;
    };
    //下面定义一个用来投影的三角形的结构
    //3个整形索引指定了模型中三角形的三个顶点
    //第二个变量指定了三角形面的法线
    //平面方程描述了三角所在的平面
    //临近的3个顶点索引，指定了与这个三角形相邻的三个顶点
    //最后一个变量指定这个三角形是否投出阴影
    // 描述一个模型表面的结构
    struct sPlane
    {
        unsigned int p[3];			// 3个整形索引指定了模型中三角形的三个顶点
        sPoint normals[3];			// 第二个变量指定了三角形面的法线
        unsigned int neigh[3]; 		// 与本三角形三个边相邻的面的索引
        sPlaneEq PlaneEq;			// 平面方程描述了三角所在的平面
        bool visible;			// 最后一个变量指定这个三角形是否投出阴影?
    };
    //最后我们用下面的结构描述一个产生阴影的物体。
    struct glObject
    {
        GLuint nPlanes, nPoints;
        sPoint points[100];
        sPlane planes[200];
    };
    explicit MyGLWidget(QWidget *parent = 0);
    ~MyGLWidget();
protected:
    void resizeGL(int w, int h);

    void initializeGL();

    void paintGL();

    void keyPressEvent(QKeyEvent *event);

    void timerEvent(QTimerEvent *event);
private:
    bool readObject(QString fileName, glObject* o);

    void setConnectivity(glObject *o);

    void drawObject(glObject o);

    void calculatePlane(glObject o, sPlane *plane);

    void castShadow(glObject *o, float *lp);

    void drawGLScene();

    void drawGLRoom();

    bool initGLObjects();
private:
    bool m_show_full_screen;
    glObject m_obj;
    GLfloat m_xrot;
    GLfloat m_yrot;
    GLfloat m_xspeed;
    GLfloat m_yspeed;
    float m_lightPos[4];// Light Position
    float m_lightAmb[4];// Ambient Light Values
    float m_lightDif[4];// Diffuse Light Values
    float m_lightSpc[4];// Specular Light Values
    float m_matAmb[4];// Material - Ambient Values
    float m_matDif[4];// Material - Diffuse Values
    float m_matSpc[4];// Material - Specular Values
    float m_matShn[1];// Material - Shininess
    float m_objPos[3];// Object Position
    GLUquadricObj *m_q;// Quadratic For Drawing A Sphere
    float m_spherePos[3];
};

#endif // MYGLWIDGET_H
