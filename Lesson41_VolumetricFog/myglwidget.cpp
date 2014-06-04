#include "myglwidget.h"

//体积雾气
//把雾坐标绑定到顶点，你可以在雾中漫游，体验一下吧。
//这一课我们将介绍体积雾，为了运行这个程序，你的显卡必须支持扩展"GL_EXT_fot_coord"。

//下面的代码设置雾的颜色
GLfloat	fogColor[4] = {0.6f, 0.3f, 0.0f, 1.0f};					// 雾的颜色

//下面变量GL_FOG_COORDINATE_SOURCE_EXT和GL_FOG_COORDINATE_EXT具有初值，他们在glext.h文件中被定义，这里我们必须感谢Lev Povalahev，它创建了这个文件。
//如果你想编译你的代码，你必须设置这个值。
//为了使用glFogCoordfExt，我们需要定义这个函数的指针，并在程序运行时把它指向显卡中的函数。
#define GL_FOG_COORDINATE_SOURCE_EXT	0x8450					// 从GLEXT.H得到的值
#define GL_FOG_COORDINATE_EXT           0x8451

typedef void (APIENTRY * PFNGLFOGCOORDFEXTPROC) (GLfloat coord);		// 声明雾坐标函数的原形
PFNGLFOGCOORDFEXTPROC glFogCoordfEXT = NULL;					// 设置雾坐标函数指针为NULL

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false)
{
    showNormal();
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(1, &m_texture[0]);
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

//下面的代码初始化OpenGL，并设置雾气的参数。
void MyGLWidget::initializeGL()
{
    // 初始化扩展
    extension_Init();
    loadGLTexture();
    glEnable(GL_TEXTURE_2D);
    glClearColor (0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepth (1.0f);
    glDepthFunc (GL_LEQUAL);
    glEnable (GL_DEPTH_TEST);
    glShadeModel (GL_SMOOTH);
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    //下面的代码设置雾气的属性
    //最后一个设置为雾气基于顶点的坐标，这运行我们把雾气放置在场景中的任意地方。
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_START,  1.0f);
    glFogf(GL_FOG_END,    0.0f);
    glHint(GL_FOG_HINT, GL_NICEST);
    glFogi(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);//设置雾气是基于顶点的坐标的
    m_camz = -19.0f;
}

//下面的代码绘制具体的场景
void MyGLWidget::paintGL()
{
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity ();
    glTranslatef(0.0f, 0.0f, m_camz);
    //下面的代码绘制四边形组成的墙，并设置每个顶点的纹理坐标和雾坐标
    glBegin(GL_QUADS);							//后墙
        glFogCoordfEXT(0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.5f,-2.5f,-15.0f);
        glFogCoordfEXT(0.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f( 2.5f,-2.5f,-15.0f);
        glFogCoordfEXT(0.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f( 2.5f, 2.5f,-15.0f);
        glFogCoordfEXT(0.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-2.5f, 2.5f,-15.0f);
    glEnd();
    glBegin(GL_QUADS);							// 地面
        glFogCoordfEXT(0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.5f,-2.5f,-15.0f);
        glFogCoordfEXT(0.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f( 2.5f,-2.5f,-15.0f);
        glFogCoordfEXT(1.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f( 2.5f,-2.5f, 15.0f);
        glFogCoordfEXT(1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-2.5f,-2.5f, 15.0f);
    glEnd();
    glBegin(GL_QUADS);							// 天花板
        glFogCoordfEXT(0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.5f, 2.5f,-15.0f);
        glFogCoordfEXT(0.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f( 2.5f, 2.5f,-15.0f);
        glFogCoordfEXT(1.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f( 2.5f, 2.5f, 15.0f);
        glFogCoordfEXT(1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-2.5f, 2.5f, 15.0f);
    glEnd();
    glBegin(GL_QUADS);							// 右墙
        glFogCoordfEXT(1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f( 2.5f,-2.5f, 15.0f);
        glFogCoordfEXT(1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f( 2.5f, 2.5f, 15.0f);
        glFogCoordfEXT(0.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f( 2.5f, 2.5f,-15.0f);
        glFogCoordfEXT(0.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f( 2.5f,-2.5f,-15.0f);
    glEnd();
    glBegin(GL_QUADS);							// 左墙
        glFogCoordfEXT(1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.5f,-2.5f, 15.0f);
        glFogCoordfEXT(1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-2.5f, 2.5f, 15.0f);
        glFogCoordfEXT(0.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f(-2.5f, 2.5f,-15.0f);
        glFogCoordfEXT(0.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(-2.5f,-2.5f,-15.0f);
    glEnd();
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
        case Qt::Key_Up:
        {
            if(m_camz < 14.0f)
            {
                m_camz+=0.1f;
                updateGL();
            }
            break;
        }
        case Qt::Key_Down:
        {
            if(m_camz > -19.0f)
            {
                m_camz-=0.1f;
                updateGL();
            }
            break;
        }
    }
}

//Nehe的原文介绍了Ipicture的接口，它不是我们这一课的重点，故我还是使用以前的方法加载纹理。
//下面的代码用来检测用户的显卡是否支持EXT_fog_coord扩展，这段代码只有在你获得OpenGL渲染描述表后才能调用，否则你将获得一个错误。
//首先，我们创建一个字符串，来描述我们需要的扩展。接着我们分配一块内存，用来保存显卡支持的扩展，它可以通过glGetString函数获得。
//接着我们检测是否含有需要的扩展，如果不存在，则返回false，如存在我们把函数的指针指向这个扩展。
int MyGLWidget::extension_Init()
{
    char Extension_Name[] = "EXT_fog_coord";
    // 返回扩展字符串
    char* glextstring=(char *)malloc(strlen((char *)glGetString(GL_EXTENSIONS))+1);
    strcpy(glextstring,(char *)glGetString(GL_EXTENSIONS));
    if (!strstr(glextstring,Extension_Name))// 查找是否有我们想要的扩展
        return FALSE;
    free(glextstring);							// 释放分配的内存
    // 获得函数的指针
    glFogCoordfEXT = (PFNGLFOGCOORDFEXTPROC) wglGetProcAddress("glFogCoordfEXT");
    return TRUE;
}

void MyGLWidget::loadGLTexture()
{
    QImage image(":/image/Wall.bmp");
    image = image.convertToFormat(QImage::Format_RGB888);
    image = image.mirrored();
    glGenTextures(1, &m_texture[0]);
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width(), image.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, image.bits());
}

//在这一课中，我已试着尽量详细解释一切。每一步都与设置有关，并创建了一个全屏OpenGL程序。
//当您按下ESC键程序就会退出，并监视窗口是否激活。
//如果您有什么意见或建议请给我EMAIL。如果您认为有什么不对或可以改进，请告诉我。
//我想做最好的OpenGL教程并对您的反馈感兴趣。
