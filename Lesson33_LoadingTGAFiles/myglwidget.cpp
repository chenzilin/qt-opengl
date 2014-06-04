#include "myglwidget.h"

//加载压缩和未压缩的TGA文件:
//在这一课里，你将学会如何加载压缩和为压缩的TGA文件，由于它使用RLE压缩，所以非常的简单，你能很快地熟悉它的。
//我见过很多人在游戏开发论坛或其它地方询问关于TGA读取的问题。接下来的程序及注释将会向你展示如何读取未压缩的
//TGA文件和RLE压缩的文件。这个详细的教程适合于OpenGL，但是我计划改进它使其在将来更具普遍性。

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_spin(0.0f)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(1, &m_textures[0].texID);
    glDeleteTextures(1, &m_textures[1].texID);
}

//下面的代码的作用是重新设置OpenGL场景的大小，而不管窗口的大小是否已经改变(假定您没有使用全屏模式)。
//甚至您无法改变窗口的大小时(例如您在全屏模式下)，它至少仍将运行一次--在程序开始时设置我们的透视图。
//OpenGL场景的尺寸将被设置成它显示时所在窗口的大小。
void MyGLWidget::resizeGL(int w, int h)
{
    if(h == 0)// 防止被零除
    {
        h = 1;// 将高设为1
    }
    glViewport(0, 0, w, h); //重置当前的视口
    //下面几行为透视图设置屏幕。意味着越远的东西看起来越小。这么做创建了一个现实外观的场景。
    //此处透视按照基于窗口宽度和高度的45度视角来计算。0.1f，100.0f是我们在场景中所能绘制深度的起点和终点。
    //glMatrixMode(GL_PROJECTION)指明接下来的两行代码将影响projection matrix(投影矩阵)。
    //投影矩阵负责为我们的场景增加透视。 glLoadIdentity()近似于重置。它将所选的矩阵状态恢复成其原始状态。
    //调用glLoadIdentity()之后我们为场景设置透视图。
    //glMatrixMode(GL_MODELVIEW)指明任何新的变换将会影响 modelview matrix(模型观察矩阵)。
    //模型观察矩阵中存放了我们的物体讯息。最后我们重置模型观察矩阵。如果您还不能理解这些术语的含义，请别着急。
    //在以后的教程里，我会向大家解释。只要知道如果您想获得一个精彩的透视场景的话，必须这么做。
    glMatrixMode(GL_PROJECTION);// 选择投影矩阵
    glLoadIdentity();// 重置投影矩阵
    //设置视口的大小
    gluPerspective(45.0f,(GLfloat)w/(GLfloat)h,0.1f,100.0f);

    glMatrixMode(GL_MODELVIEW);	//选择模型观察矩阵
    glLoadIdentity(); // 重置模型观察矩阵
}

//接下的代码段中，我们将对OpenGL进行所有的设置。我们将设置清除屏幕所用的颜色，打开深度缓存，启用smooth shading(阴影平滑)，等等。
//这个例程直到OpenGL窗口创建之后才会被调用。此过程将有返回值。但我们此处的初始化没那么复杂，现在还用不着担心这个返回值。
void MyGLWidget::initializeGL()
{
    loadGLTextures();
    glEnable(GL_TEXTURE_2D);// Enable Texture Mapping ( NEW )
    glShadeModel(GL_SMOOTH);// Enable Smooth Shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);// Black Background
    glClearDepth(1.0f);// Depth Buffer Setup
    glEnable(GL_DEPTH_TEST);// Enables Depth Testing
    glDepthFunc(GL_LEQUAL);// The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);// Really Nice Perspective Calculations
}

void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// Clear The Screen And The Depth Buffer
    glLoadIdentity();// Reset The Modelview Matrix
    glTranslatef(0.0f,0.0f,-10.0f);// Translate 20 Units Into The Screen

    for (int loop=0; loop<20; loop++)// Loop Of 20
    {
        glPushMatrix();// Push The Matrix
            glRotatef(m_spin+loop*18.0f,1.0f,0.0f,0.0f);// Rotate On The X-Axis (Up - Down)
            glTranslatef(-2.0f,2.0f,0.0f);// Translate 2 Units Left And 2 Up

            glBindTexture(GL_TEXTURE_2D, m_textures[0].texID);// ( CHANGE )
            glBegin(GL_QUADS);// Draw Our Quad
                glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, 0.0f);
                glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, 0.0f);
                glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, 0.0f);
                glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
            glEnd();// Done Drawing The Quad
        glPopMatrix();// Pop The Matrix

        glPushMatrix();// Push The Matrix
            glTranslatef(2.0f,0.0f,0.0f);// Translate 2 Units To The Right
            glRotatef(m_spin+loop*36.0f,0.0f,1.0f,0.0f);// Rotate On The Y-Axis (Left - Right)
            glTranslatef(1.0f,0.0f,0.0f);// Move One Unit Right

            glBindTexture(GL_TEXTURE_2D, m_textures[1].texID);// ( CHANGE )
            glBegin(GL_QUADS);// Draw Our Quad
                glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f, 0.0f);
                glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f, 0.0f);
                glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1.0f, 0.0f);
                glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
            glEnd();// Done Drawing The Quad
        glPopMatrix();// Pop The Matrix
    }
}

void MyGLWidget::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_F2:
        {
            m_show_full_screen = !m_show_full_screen;
            if(m_show_full_screen)
            {
                showFullScreen();
            }
            else
            {
                showNormal();
            }
            updateGL();
            break;
        }
        case Qt::Key_Escape:
        {
            qApp->exit();
            break;
        }
    }
    QGLWidget::keyPressEvent(event);
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    m_spin+=0.05f;// Increase Spin
    updateGL();
    QGLWidget::timerEvent(event);
}

int MyGLWidget::loadGLTextures()
{
    int Status=false;
    QString uncompressedFile = QApplication::applicationDirPath() + "/image/Uncompressed.tga";
    QString compressedFile = QApplication::applicationDirPath() + "/image/Compressed.tga";
    if (LoadTGA(&m_textures[0], uncompressedFile) &&
        LoadTGA(&m_textures[1], compressedFile))
    {
        Status=true;
        for (int loop=0; loop<2; loop++)
        {
            glGenTextures(1, &m_textures[loop].texID);
            glBindTexture(GL_TEXTURE_2D, m_textures[loop].texID);
            glTexImage2D(GL_TEXTURE_2D, 0, m_textures[loop].bpp / 8, m_textures[loop].width,
                         m_textures[loop].height, 0, m_textures[loop].type, GL_UNSIGNED_BYTE,
                         m_textures[loop].imageData);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

            if (m_textures[loop].imageData)
            {
                free(m_textures[loop].imageData);
            }
        }
    }
    return Status;
}

//现在你已经为glGenTextures和glBindTexture准备好了数据。我建议你查看Nehe的教程6和24以获取这些命令的更多信息。
//那证实了我先前写的教程的正确性，我不确保的代码中没有错误，虽然我努力使之不发生错误。
//特别感谢Jeff“Nehe”Molofee写了这个伟大的教程，以及Trent“ShiningKnight”Polack帮助我修订这个教程。
//如果你发现了错误、有建议或者注释，请自由地给我发Email
