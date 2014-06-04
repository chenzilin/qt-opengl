#include "myglwidget.h"

//图形字体的纹理映射:
//这一课，我们将在上一课的基础上创建带有纹理的字体，它真的很简单。

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false),
    m_freeTypeFont(NULL),  m_rot(0.0f)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(1, &m_texture[0]);
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
    //glEnable(GL_COLOR_MATERIAL) 这行被删掉了，如果你想使用glColor3f(r,g,b)来改变纹理的颜色，
    //那么就把glEnable(GL_COLOR_MATERIAL)这行重新加到这部分代码中。
    loadGLTexture();
    glShadeModel(GL_SMOOTH);// 启用阴影平滑
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);// 黑色背景
    glClearDepth(1.0f);// 设置深度缓存
    glEnable(GL_DEPTH_TEST);// 启用深度测试
    glDepthFunc(GL_LEQUAL);// 所作深度测试的类型
    glEnable(GL_LIGHT0);// 使用第0号灯
    glEnable(GL_LIGHTING);// 使用光照
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);// 告诉系统对透视进行修正
    glEnable(GL_TEXTURE_2D);// 使用二维纹理
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);	// 选择使用的纹理

    QString fontPath = QApplication::applicationDirPath() + "/Wingdings.ttf";
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
    glLoadIdentity();									// Reset The View
    //这里是第一处变动。我们打算使用COS和SIN让物体绕着屏幕旋转而不是把它固定在屏幕中间。
    //我们将把物体向屏幕里移动3个单位。在x轴，我们将移动范围限制在-1.1到+1.1之间。我们使用rot变量来控制左右移动。
    //我们把上下移动的范围限制在+0.8到-0.8之间。同样使用rot变量来控制上下移动（最好充分利用你的变量）。
    glTranslatef(1.1f*float(cos(m_rot/16.0f)),0.8f*float(sin(m_rot/20.0f)),-3.0f);// 设置字体的位置
    //这会使符号在X，Y和Z轴旋转。
    glRotatef(m_rot,1.0f,0.0f,0.0f);						// Rotate On The X Axis
    glRotatef(m_rot*1.2f,0.0f,1.0f,0.0f);					// Rotate On The Y Axis
    glRotatef(m_rot*1.4f,0.0f,0.0f,1.0f);					// Rotate On The Z Axis
    //将物体相对观察点向左向下移动一点，以便于把符号定位于每个轴的中心。
    //否则，当我们旋转它的时候，看起来就不像是在围绕它自己的中心在旋转。-0.35只是一个能让符号正确显示的数。
    //我也试过一些其它数，因为我不知道这种字体的宽度是多少，可以适情况作出调整。我不知道为什么这种字体没有一个中心。
    glTranslatef(-0.35f,-0.35f,0.1f);					// Center On X, Y, Z Axis
    m_freeTypeFont->Render("N");						// Draw A Skull And Crossbones Symbol
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
    m_rot+=0.1f;											// Increase The Rotation Variable
    updateGL();
    QGLWidget::timerEvent(event);
}

void MyGLWidget::loadGLTexture()
{
    QImage image(":/image/Lights.bmp");
    image = image.convertToFormat(QImage::Format_RGB888);
    image = image.mirrored();
    glGenTextures(1, &m_texture[0]);
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image.width(), image.height(),
            GL_RGB, GL_UNSIGNED_BYTE, image.bits());
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
    //下面四行代码将为我们绘制在屏幕上的任何物体自动生成纹理坐标。函数glTexGen非常强大，而且复杂，
    //如果要完全讲清楚它的数学原理需要再写一篇教程。不过，你只要知道GL_S和GL_T是纹理坐标就可以了。默认状态下，
    //它被设置为提取物体此刻在屏幕上的x坐标和y坐标，并把它们转换为顶点坐标。你会发现到物体在z平面没有纹理，只显示一些斑纹。
    //正面和反面都被赋予了纹理，这些都是由glTexGen函数产生的。(X(GL_S)用于从左到右映射纹理，Y(GL_T)用于从上到下映射纹理。
    //GL_TEXTURE_GEN_MODE允许我们选择我们想在S和T纹理坐标上使用的纹理映射模式。你有3种选择：
    //GL_EYE_LINEAR - 纹理会固定在屏幕上。它永远不会移动。物体将被赋予处于它通过的地区的那一块纹理。
    //GL_OBJECT_LINEAR - 这种就是我们使用的模式。纹理被固定于在屏幕上运动的物体上。
    //GL_SPHERE_MAP - 每个人都喜欢。创建一种有金属质感的物体。
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
}

//尽管我没有讲的细致入微，但我想你应该很好的理解了如何让OpenGL为你生成纹理坐标。在给你的字体或者是同类物体赋予纹理映射时，
//应该没有问题了，而且只需要改变两行代码，你就可以启用球体映射了，它的效果简直酷毙了！
