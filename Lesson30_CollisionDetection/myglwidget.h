#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>
#include <QSound>
#include "tvector.h"
#include "tray.h"

struct Plane
{
    TVector _Position;
    TVector _Normal;
};

//Cylinder structure
struct Cylinder
{
    TVector _Position;
    TVector _Axis;
    double _Radius;
};

//Explosion structure
struct Explosion
{
    TVector _Position;
    float   _Alpha;
    float   _Scale;
};

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
    int findBallCol(TVector& point, double& TimePoint, double Time2, int& BallNr1, int& BallNr2);

    void idle();

    int testIntersionPlane(const Plane& plane,const TVector& position,const TVector& direction,
                           double& lamda, TVector& pNormal);

    int testIntersionCylinder(const Cylinder& cylinder,const TVector& position,const TVector& direction,
                              double& lamda, TVector& pNormal,TVector& newposition);

    void initVars();

    void loadGLTexture();
private:
    bool m_show_full_screen;
    TVector m_arrayVel[10];//holds velocity of balls
    TVector m_arrayPos[10];//position of balls
    TVector m_oldPos[10];//old position of balls
    Explosion m_explosionArray[20];//holds max 20 explosions at once
    TVector m_pos;//initial position of camera
    GLuint m_texture[4];
    GLuint m_dlist;//stores texture objects and display list
    int m_hook_toball1;
    int m_sounds;//hook camera on ball, and sound on/off
    float m_camera_rotation;//holds rotation around the Y axis
    double m_time;//timestep of simulation
    int m_nrOfBalls;//sets the number of balls
    GLUquadricObj *m_cylinder_obj;//Quadratic object to render the cylinders
    Plane m_pl1;
    Plane m_pl2;
    Plane m_pl3;
    Plane m_pl4;
    Plane m_pl5;                //the 5 planes of the room
    Cylinder m_cyl1;
    Cylinder m_cyl2;
    Cylinder m_cyl3;                  //the 2 cylinders of the room
};

#endif // MYGLWIDGET_H
