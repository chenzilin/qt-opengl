#include "myglwidget.h"

//全屏反走样
//当今显卡的强大功能，你几乎什么都不用做，只需要在创建窗口的时候该一个数据。看看吧，驱动程序为你做完了一切。

//在图形的绘制中，直线的走样是非常影响美观的，我们可以使用反走样解决这个问题。
//在众多的解决方案里，多重采样是一种易于硬件实现的方法，也是一种快速的方法。
//全凭多重采样可以使你的图形看起来更美观，我们可以使用ARB_MULTISAMPLE扩展完成这个功能，但它会降低你的程序的速度。

//如果你想知道更多的关于多重采样的信息，请访问下面的链接：
//GDC2002 -- OpenGL Multisample
//OpenGL Pixel Formats and Multisample Antialiasing
//下面我们来介绍如何使用多重采样，不向其他的扩展，我们在使用多重采样时，必须在窗口创建时告诉它使用多重采样，
//典型的步骤如下：
//1、创建一个窗口
//2、查询是否支持多重采样
//3、如果支持删除当前的窗口，使用支持多重采样的格式创建窗口
//4、如果我们想使用多重采样，仅仅启用它既可。
//了解了上面，我们从头说明如何使用多重采样，并介绍ARB_Multisample的实现方法：

MyGLWidget::MyGLWidget(QGLFormat format, QWidget *parent) :
    QGLWidget(format, parent), m_show_full_screen(false), m_angle(0.0f),
    m_domulti(true), m_doangle(true)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_angle(0.0f),
    m_domulti(true), m_doangle(true)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
}

//下面的代码的作用是重新设置OpenGL场景的大小，而不管窗口的大小是否已经改变(假定您没有使用全屏模式)。
//甚至您无法改变窗口的大小时(例如您在全屏模式下)，它至少仍将运行一次--在程序开始时设置我们的透视图。
//OpenGL场景的尺寸将被设置成它显示时所在窗口的大小。
void MyGLWidget::resizeGL(int w, int h)
{
    glViewport (0, 0, (GLsizei)(w), (GLsizei)(h));				// Reset The Current Viewport
    glMatrixMode (GL_PROJECTION);										// Select The Projection Matrix
    glLoadIdentity ();													// Reset The Projection Matrix
    gluPerspective(50, (float)w/(float)h, 5,  2000);
    glMatrixMode (GL_MODELVIEW);										// Select The Modelview Matrix
    glLoadIdentity ();													// Reset The Modelview Matrix
}

//接下的代码段中，我们将对OpenGL进行所有的设置。我们将设置清除屏幕所用的颜色，打开深度缓存，启用smooth shading(阴影平滑)，等等。
//这个例程直到OpenGL窗口创建之后才会被调用。此过程将有返回值。但我们此处的初始化没那么复杂，现在还用不着担心这个返回值。
void MyGLWidget::initializeGL()
{
    // Start Of User Initialization
    m_angle = 0.0f;                                               // Set Starting Angle To Zero
    glViewport(0 , 0, this->window()->width(), this->window()->height());	// Set Up A Viewport
    glMatrixMode(GL_PROJECTION);								// Select The Projection Matrix
    glLoadIdentity();											// Reset The Projection Matrix
    gluPerspective(50, (float)this->window()->width()/(float)this->window()->height(), 5,  2000); // Set Our Perspective
    glMatrixMode(GL_MODELVIEW);									// Select The Modelview Matrix
    glLoadIdentity();											// Reset The Modelview Matrix
    glEnable(GL_DEPTH_TEST);									// Enable Depth Testing
    glShadeModel(GL_SMOOTH);									// Select Smooth Shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.5);						// Set The Clear Color To Black
}

void MyGLWidget::paintGL()
{
    if(m_domulti)
        glEnable(GL_MULTISAMPLE_ARB);							// Enable Our Multisampling
    // ENDROACH

    glClearColor(0.0f, 0.0f, 0.0f, 0.5);						// Set The Clear Color To Black
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer
    glLoadIdentity();											// Reset The View

    for(float i=-10;i<10;i++)
        for(float j=-10;j<10;j++)
        {
            glPushMatrix();
            glTranslatef(i*2.0f,j*2.0f,-5.0f);
            glRotatef(m_angle,0.f,0.f,1.f);
                glBegin(GL_QUADS);
                glColor3f(1.0f,0.0f,0.0f);glVertex3f(i,j,0.0f);
                glColor3f(0.0f,1.0f,0.0f);glVertex3f(i + 2.0f,j,0.0f);
                glColor3f(0.0f,0.0f,1.0f);glVertex3f(i + 2.0f,j + 2.0f,0.0f);
                glColor3f(1.0f,1.0f,1.0f);glVertex3f(i,j + 2.0f,0.0f);
                glEnd();
            glPopMatrix();
        }

    if(m_doangle)
        m_angle+=0.05f;

    glFlush ();													// Flush The GL Rendering Pipeline

    // ROACH
    if(m_domulti)
        glDisable(GL_MULTISAMPLE_ARB);
    // ENDROACH
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
        case Qt::Key_Space:
        {
            m_domulti=!m_domulti;
            break;
        }
        case Qt::Key_M:
        {
            m_doangle=!m_doangle;
            break;
        }
    }
    QGLWidget::keyPressEvent(event);
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    updateGL();

//    QGLFormat f = this->format();
//    if(m_domulti)
//    {
//        f.setSampleBuffers(false);
//    }
//    else
//    {
//        f.setSampleBuffers(true);
//    }
//    setFormat(f);
    QGLWidget::timerEvent(event);
}

//好了，那就是全部，希望你能喜欢：）
