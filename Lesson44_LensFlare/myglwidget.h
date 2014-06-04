#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>
#include <qmath.h>

class glVector
{
public:
    void operator =(glVector v);
    glVector operator+(glVector v);
    glVector operator*(GLfloat scalar);
    void Normalize(void);
    GLfloat Magnitude(void);
    GLfloat m_Mag;
    void operator *=(GLfloat scalar);
    glVector();
    virtual ~glVector();

    GLfloat k;
    GLfloat j;
    GLfloat i;
};

class glPoint
{
public:
    void operator+=(glPoint p);
    glPoint operator+(glPoint p);
    glVector operator-(glPoint p);
    void operator =(glVector v);
    void operator =(glPoint p);
    glPoint();
    virtual ~glPoint();

    GLfloat z;
    GLfloat y;
    GLfloat x;
};

class glCamera
{
public:
    //////////// CONSTRUCTORS /////////////////////////////////////////
    glCamera();
    virtual ~glCamera();

    //////////// FRUSTUM TESTING FUNCTIONS ////////////////////////////
    BOOL SphereInFrustum(glPoint p, GLfloat Radius);
    BOOL SphereInFrustum(GLfloat x, GLfloat y, GLfloat z, GLfloat Radius);
    BOOL PointInFrustum(GLfloat x, GLfloat y, GLfloat z);
    BOOL PointInFrustum(glPoint p);

    //////////// FUNCTIONS TO RENDER LENS FLARES //////////////////////
    void RenderLensFlare(void);
    void RenderStreaks(GLfloat r, GLfloat g, GLfloat b, GLfloat a, glPoint p, GLfloat scale);
    void RenderBigGlow(GLfloat r, GLfloat g, GLfloat b, GLfloat a, glPoint p, GLfloat scale);
    void RenderGlow(GLfloat r, GLfloat g, GLfloat b, GLfloat a, glPoint p, GLfloat scale);
    void RenderHalo(GLfloat r, GLfloat g, GLfloat b, GLfloat a, glPoint p, GLfloat scale);

    //////////// FUNCTIONS TO UPDATE THE FRUSTUM //////////////////////
    void UpdateFrustumFaster(void);
    void UpdateFrustum(void);

    //////////// FUNCTIONS TO CHANGE CAMERA ORIENTATION AND SPEED /////
    void ChangeVelocity(GLfloat vel);
    void ChangeHeading(GLfloat degrees);
    void ChangePitch(GLfloat degrees);
    void SetPrespective(void);

    //############################### NEW STUFF ##########################
    /////////// OCCLUSION TESTING FUNCTIONS ///////////////////////////
    bool IsOccluded(glPoint p);

    //////////// MEMBER VARIBLES //////////////////////////////////////
    glVector vLightSourceToCamera, vLightSourceToIntersect;
    glPoint ptIntersect, pt;
    GLsizei m_WindowHeight;
    GLsizei m_WindowWidth;
    GLuint m_StreakTexture;
    GLuint m_HaloTexture;
    GLuint m_GlowTexture;
    GLuint m_BigGlowTexture;
    GLfloat m_MaxPointSize;
    GLfloat m_Frustum[6][4];
    glPoint m_LightSourcePos;
    GLfloat m_MaxPitchRate;
    GLfloat m_MaxHeadingRate;
    GLfloat m_HeadingDegrees;
    GLfloat m_PitchDegrees;
    GLfloat m_MaxForwardVelocity;
    GLfloat m_ForwardVelocity;
    glPoint m_Position;
    glVector m_DirectionVector;
};

class glFont
{
public:
    GLuint GetListBase(void);
    GLuint GetTexture(void);
    void SetWindowSize(GLint width, GLint height);
    void glPrintf(GLint x, GLint y, GLint set, const char *Format, ...);
    void BuildFont(GLfloat Scale=1.0f);
    void SetFontTexture(GLuint tex);
    glFont();
    virtual ~glFont();

protected:
    GLdouble m_WindowWidth;
    GLdouble m_WindowHeight;
    GLuint m_ListBase;
    GLuint m_FontTexture;
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
    void loadTexture(const QString &filePath, GLuint &texId);

    void drawGLInfo();
private:
    bool m_show_full_screen;

    glFont m_gFont;
    glCamera m_gCamera;
    GLint m_cylList;
    qlonglong m_gStartTime;
    qlonglong m_gCurrentTime;
    qlonglong m_timeCounter;
    GLUquadricObj *m_qobj;
    int m_gFrames;
    double m_gFPS;
    bool m_infoOn;
};

#endif // MYGLWIDGET_H
