#include "myglwidget.h"

//图形字体:
//在一课我们将教你绘制3D的图形字体，它们可像一般的3D模型一样被变换。

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false),
    m_freeTypeFont(NULL),  m_rot(0.0f)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
}

void MyGLWidget::resizeGL(int w, int h)
{
    if (h==0)										// Prevent A Divide By Zero By
    {
        h=1;										// Making Height Equal One
    }

    glViewport(0,0,w,h);						// Reset The Current Viewport

    glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
    glLoadIdentity();									// Reset The Projection Matrix

                                                // Calculate The Aspect Ratio Of The Window
    gluPerspective(45.0f,(GLfloat)w/(GLfloat)h,0.1f,100.0f);

    glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
    glLoadIdentity();									// Reset The Modelview Matrix
}

void MyGLWidget::initializeGL()
{
    glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
    glClearDepth(1.0f);									// Depth Buffer Setup
    glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
    glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
    glEnable(GL_LIGHT0);								// Enable Default Light (Quick And Dirty)
    glEnable(GL_LIGHTING);								// Enable Lighting
    glEnable(GL_COLOR_MATERIAL);						// Enable Coloring Of Material

    QString fontPath = QApplication::applicationDirPath() + "/Test.ttf";
    m_freeTypeFont = new FTGLExtrdFont(fontPath.toLatin1().data());
    if(m_freeTypeFont == NULL)
    {
        QMessageBox::warning(this, tr("Warning"), tr("%1 file can't find").arg(fontPath));
    }
    else
    {
        m_freeTypeFont->FaceSize(1);
        m_freeTypeFont->Depth(0.2);
        m_freeTypeFont->CharMap(ft_encoding_unicode);
    }
}

void MyGLWidget::paintGL()
{
    //下面就是画图的代码了。我们从清除屏幕和深度缓存开始。我们调用glLoadIdentity()来重置所有东西。然后我们将坐标系向屏幕里移动十个单位。
    //轮廓字体在透视图模式下表现非常好。你将文字移入屏幕越深，文字开起来就更小。文字离你越近，它看起来就更大。
    //也可以使用glScalef(x,y,z)命令来操作轮廓字体。如果你想把字体放大两倍，可以使用glScalef(1.0f,2.0f,1.0f). 2.0f 作用在y轴，
    //它告诉OpenGL将显示列表的高度绘制为原来的两倍。如果2.0f作用在x轴，那么文本的宽度将变成原来的两倍
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// 清除屏幕及深度缓存
    glLoadIdentity();// 重置当前的模型观察矩阵
    glTranslatef(0.0f,0.0f,-10.0f);// 移入屏幕一个单位
    //在向屏幕里移动以后，我们希望文本能旋转起来。下面3行代码用来在3个轴上旋转屏幕。我将rot乘以不同的数，以便每个方向上的旋转速度不同。
    glRotatef(m_rot,1.0f,0.0f,0.0f);// X轴旋转
    glRotatef(m_rot*1.5f,0.0f,1.0f,0.0f);// Y轴旋转
    glRotatef(m_rot*1.4f,0.0f,0.0f,1.0f);// Z轴旋转
    // Pulsing Colors Based On The Rotation
    //下面是令人兴奋的颜色循环了。照常，我们使用唯一递增的变量（rot）。颜色通过使用COS和SIN来循环变化。我将rot除以不同的数，
    //这样每种颜色会以不同的速度递增。最终的效果非常好
    glColor3f(1.0f*float(cos(m_rot/20.0f)),1.0f*float(sin(m_rot/25.0f)),1.0f-0.5f*float(cos(m_rot/17.0f)));
    QString renderText = QString("NeHe - %1").arg(m_rot/50);
    glTranslatef(-3.0,0.0f,0.0f);                           // Center Our Text On The Screen
    m_freeTypeFont->Render(renderText.toLatin1().data());						// Print GL Text To The Screen
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
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    m_rot+=0.5f;											// Increase The Rotation Variable
    updateGL();
    QGLWidget::timerEvent(event);
}

//在这节课结束的时候，你应该已经学会在你的OpenGL程序中使用轮廓字体了。就像第13课，我曾在网上寻找一篇与这一课相似的教程，
//但是也没有找到。或许我的网站是第一个涉及这个主题同时又把它解释的简单易懂的C代码的网站吧。享用这篇教程，快乐编码！
