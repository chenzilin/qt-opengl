#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <GL/glaux.h>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>
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

    bool initMultitexture();

    void initLights();

    void doMesh1TexelUnits();

    void doMesh2TexelUnits();

    void doMeshNoBumps();

    void doCube();

    void doLogo();
private:
    bool m_show_full_screen;
    GLfloat m_z;
    GLfloat m_xrot;
    GLfloat m_yrot;
    GLfloat m_xspeed;
    GLfloat m_yspeed;

    bool m_multitextureSupported;						// 标志是否支持多重渲染
    bool m_useMultitexture;							// 如果支持,是否使用它
    bool m_emboss;
    bool m_bumps;
    GLint m_maxTexelUnits;							// 纹理处理单元的个数
    //下面我们来定义一些变量：
    //filter定义过滤器类型
    //texture[3]保存三个纹理
    //bump[3]保存三个凹凸纹理
    //invbump[3]保存三个反转了的凹凸纹理
    //glLogo保存标志
    //multiLogo保存多重纹理标志
    GLuint m_filter;									// 定义过滤器类型
    GLuint m_texture[3];								// 保存三个纹理
    GLuint m_bump[3];									//保存三个凹凸纹理
    GLuint m_invbump[3];								// 保存三个反转了的凹凸纹理
    GLuint m_glLogo;									// glLogo保存标志
    GLuint m_multiLogo;									// multiLogo保存多重纹理标志
};

#endif // MYGLWIDGET_H
