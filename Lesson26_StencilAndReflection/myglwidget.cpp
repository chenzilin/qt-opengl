#include "myglwidget.h"

//剪裁平面，蒙板缓存和反射:
//在这一课中你将学会如何创建镜面显示效果，它使用剪裁平面，蒙板缓存等OpenGL中一些高级的技巧。
//欢迎来到另一个激动人心的课程，这课的代码是Banu Cosmin所写，当然教程还是我自己写的。
//在这课里，我将教你创建真正的反射，基于物理的。
//由于它将用到蒙板缓存，所以需要耗费一些资源。当然随着显卡和CPU的发展，这些都不是问题了，好了让我们开始吧！

//下面我们设置光源的参数
static GLfloat	LightAmb[] = {0.7f, 0.7f, 0.7f, 1.0f};// 环境光
static GLfloat	LightDif[] = {1.0f, 1.0f, 1.0f, 1.0f};// 漫射光
static GLfloat	LightPos[] = {4.0f, 4.0f, 6.0f, 1.0f};// 灯光的位置

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false),m_q(NULL),
    m_xrot(0.0f), m_yrot(0.0f), m_xrotspeed(0.0f), m_yrotspeed(0.0f),
    m_zoom(-7.0f), m_height(2.0f)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(3, &m_textures[0]);
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

//一个新的函数glClearStencil被加入到初始化代码中，它用来设置清空操作后蒙板缓存中的值。其他的操作保持不变。
void MyGLWidget::initializeGL()
{
    loadGLTexture();
    glShadeModel(GL_SMOOTH);
    glClearColor(0.2f, 0.5f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glClearStencil(0);// 设置蒙板值
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_TEXTURE_2D);// 使用2D纹理
    //下面的代码用来启用光照
    glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDif);
    glLightfv(GL_LIGHT0, GL_POSITION, LightPos);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    //下面的代码使用二次几何体创建一个球体，在前面的教程中都已经详纤，这里不再重复。
    m_q = gluNewQuadric();// 创建一个二次几何体
    gluQuadricNormals(m_q, GL_SMOOTH);// 使用平滑法线
    gluQuadricTexture(m_q, GL_TRUE);// 使用纹理
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);// 设置球纹理映射
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
}

//现在到了我们绘制函数的地方，我们将把所有的模型结合起来创建一个反射的场景。
//向往常一样先把各个缓存清空，接着定义我们的剪切平面，它用来剪切我们的图像。这个平面的方程为equ[]={0,-1,0,0},向你所看到的它的法线是指向-y轴的，
//这告诉我们你只能看到y轴坐标小于0的像素，如果你启用剪切功能的话。
//关于剪切平面，我们在后面会做更多的讨论。继续吧：）
void MyGLWidget::paintGL()
{
    // 清除缓存
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    // 设置剪切平面
    double eqr[] = {0.0f,-1.0f, 0.0f, 0.0f};
    //下面我们把地面向下平移0.6个单位，因为我们的眼睛在y=0的平面，如果不平移的话，那么看上去平面就会变为一条线，为了看起来更真实，我们平移了它。
    glLoadIdentity();
    glTranslatef(0.0f, -0.6f, m_zoom);// 平移和缩放地面
    //下面我们设置了颜色掩码，在默认情况下所有的颜色都可以写入，即在函数glColorMask中，所有的参数都被设为GL_TRUE，如果设为零表示这部分颜色不可写入。
    //现在我们不希望在屏幕上绘制任何东西，所以把参数设为0。
    glColorMask(0,0,0,0);
    //下面来设置蒙板缓存和蒙板测试。
    //首先我们启用蒙板测试，这样就可以修改蒙板缓存中的值。
    //下面我们来解释蒙板测试函数的含义：
    //当你使用glEnable(GL_STENCIL_TEST)启用蒙板测试之后，蒙板函数用于确定一个颜色片段是应该丢弃还是保留（被绘制）。蒙板缓存区中的值与参考值ref进行比较，
    //比较标准是func所指定的比较函数。参考值和蒙板缓存区的值都可以与掩码进行为AND操作。蒙板测试的结果还导致蒙板缓存区根据glStencilOp函数所指定的行为进行修改。
    //func的参数值如下：
    //常量	含义
    //GL_NEVER	从不通过蒙板测试
    //GL_ALWAYS	总是通过蒙板测试
    //GL_LESS	只有参考值<(蒙板缓存区的值&mask)时才通过
    //GL_LEQUAL	只有参考值<=(蒙板缓存区的值&mask)时才通过
    //GL_EQUAL	只有参考值=(蒙板缓存区的值&mask)时才通过
    //GL_GEQUAL	只有参考值>=(蒙板缓存区的值&mask)时才通过
    //GL_GREATER	只有参考值>(蒙板缓存区的值&mask)时才通过
    //GL_NOTEQUAL	只有参考值!=(蒙板缓存区的值&mask)时才通过
    //接下来我们解释glStencilOp函数，它用来根据比较结果修改蒙板缓存区中的值，它的函数原形为：
    //void glStencilOp(GLenum sfail, GLenum zfail, GLenum zpass)，各个参数的含义如下：
    //sfail
    //当蒙板测试失败时所执行的操作
    //zfail
    //当蒙板测试通过，深度测试失败时所执行的操作
    //zpass
    //当蒙板测试通过，深度测试通过时所执行的操作
    //具体的操作包括以下几种
    //常量	描述
    //GL_KEEP	保持当前的蒙板缓存区值
    //GL_ZERO	把当前的蒙板缓存区值设为0
    //GL_REPLACE	用glStencilFunc函数所指定的参考值替换蒙板参数值
    //GL_INCR	增加当前的蒙板缓存区值，但限制在允许的范围内
    //GL_DECR	减少当前的蒙板缓存区值，但限制在允许的范围内
    //GL_INVERT	将当前的蒙板缓存区值进行逐位的翻转
    //当完成了以上操作后我们绘制一个地面，当然现在你什么也看不到，它只是把覆盖地面的蒙板缓存区中的相应位置设为1。
    glEnable(GL_STENCIL_TEST);// 启用蒙板缓存
    glStencilFunc(GL_ALWAYS, 1, 1);// 设置蒙板测试总是通过，参考值设为1，掩码值也设为1
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);// 设置当深度测试不通过时，保留蒙板中的值不变。如果通过则使用参考值替换蒙板值
    glDisable(GL_DEPTH_TEST);// 禁用深度测试
    drawFloor();// 绘制地面
    //我们现在已经在蒙板缓存区中建立了地面的蒙板了，这是绘制影子的关键，如果想知道为什么，接着向后看吧:)
    //下面我们启用深度测试和绘制颜色，并相应设置蒙板测试和函数的值，这种设置可以使我们在屏幕上绘制而不改变蒙板缓存区的值。
    glEnable(GL_DEPTH_TEST);//启用深度测试
    glColorMask(1,1,1,1);// 可以绘制颜色
    glStencilFunc(GL_EQUAL, 1, 1);//下面的设置指定当我们绘制时，不改变蒙板缓存区的值
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    //下面的代码设置并启用剪切平面，使得只能在地面的下方绘制
    glEnable(GL_CLIP_PLANE0);// 使用剪切平面
    glClipPlane(GL_CLIP_PLANE0, eqr);// 设置剪切平面为地面，并设置它的法线为向下
    glPushMatrix();// 保存当前的矩阵
        glScalef(1.0f, -1.0f, 1.0f);// 沿Y轴反转
        //由于上面已经启用了蒙板缓存，则你只能在蒙板中值为1的地方绘制，反射的实质就是在反射屏幕的对应位置在绘制一个物体，并把它放置在反射平面中。
        //下面的代码完成这个功能
        glLightfv(GL_LIGHT0, GL_POSITION, LightPos);// 设置灯光0
        glTranslatef(0.0f, m_height, 0.0f);
        glRotatef(m_xrot, 1.0f, 0.0f, 0.0f);
        glRotatef(m_yrot, 0.0f, 1.0f, 0.0f);
        drawObject();// 绘制反射的球
    glPopMatrix();// 弹出保存的矩阵
    glDisable(GL_CLIP_PLANE0);// 禁用剪切平面
    glDisable(GL_STENCIL_TEST);// 关闭蒙板
    //下面的代码绘制地面，并把地面颜色和反射的球颜色混合，使其看起来像反射的效果。
    glLightfv(GL_LIGHT0, GL_POSITION, LightPos);
    glEnable(GL_BLEND);// 启用混合
    glDisable(GL_LIGHTING);// 关闭光照
    glColor4f(1.0f, 1.0f, 1.0f, 0.8f);// 设置颜色为白色
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);// 设置混合系数
    drawFloor();// 绘制地面
    //下面的代码在距地面高为height的地方绘制一个真正的球
    glEnable(GL_LIGHTING);// 使用光照
    glDisable(GL_BLEND);// 禁用混合
    glTranslatef(0.0f, m_height, 0.0f);// 移动高位height的位置
    glRotatef(m_xrot, 1.0f, 0.0f, 0.0f);// 设置球旋转的角度
    glRotatef(m_yrot, 0.0f, 1.0f, 0.0f);
    drawObject();// 绘制球
    glFlush();// 强制OpenGL执行所有命令
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
        case Qt::Key_Right:
        {
            m_yrotspeed += 0.08f;
            break;
        }
        case Qt::Key_Left:
        {
            m_yrotspeed -= 0.08f;
            break;
        }
        case Qt::Key_Down:
        {
            m_xrotspeed += 0.08f;
            break;
        }
        case Qt::Key_Up:
        {
            m_xrotspeed -= 0.08f;
            break;
        }
        case Qt::Key_A:
        {
            m_zoom +=0.05f;
            break;
        }
        case Qt::Key_Z:
        {
            m_zoom -=0.05f;
            break;
        }
        case Qt::Key_PageUp:
        {
            m_height +=0.03f;
            break;
        }
        case Qt::Key_PageDown:
        {
            m_height -=0.03f;
            break;
        }
    }
    QGLWidget::keyPressEvent(event);
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    m_xrot += m_xrotspeed;// 更新X轴旋转速度
    m_yrot += m_yrotspeed;// 更新Y轴旋转速度
    updateGL();
    QGLWidget::timerEvent(event);
}

void MyGLWidget::loadGLTexture()
{
    QVector<QImage> images;
    images.push_back(QImage(":/image/Envwall.bmp"));
    images.push_back(QImage(":/image/Ball.bmp"));
    images.push_back(QImage(":/image/Envroll.bmp"));
    for(int i = 0, iend = images.count(); i < iend; i++)
    {
        images[i] = images[i].convertToFormat(QImage::Format_RGB888);
        images[i] = images[i].mirrored();
    }
    glGenTextures(3, &m_textures[0]);
    for(int i = 0, iend = images.count(); i < iend; i++)
    {
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, images[i].width(), images[i].height(),
                     0, GL_RGB, GL_UNSIGNED_BYTE, images[i].bits());
    }
}

//下面的代码绘制我们的球
void MyGLWidget::drawObject()// 绘制我们的球
{
    glColor3f(1.0f, 1.0f, 1.0f);// 设置为白色
    glBindTexture(GL_TEXTURE_2D, m_textures[1]);// 设置为球的纹理
    gluSphere(m_q, 0.35f, 32, 16);// 绘制球
    //绘制完一个白色的球后，我们使用环境贴图来绘制另一个球，把这两个球按alpha混合起来。
    glBindTexture(GL_TEXTURE_2D, m_textures[2]);// 设置为环境纹理
    glColor4f(1.0f, 1.0f, 1.0f, 0.4f);// 使用alpha为40%的白色
    glEnable(GL_BLEND);// 启用混合
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);// 把原颜色的40%与目标颜色混合
    glEnable(GL_TEXTURE_GEN_S);// 使用球映射
    glEnable(GL_TEXTURE_GEN_T);
    gluSphere(m_q, 0.35f, 32, 16);// 绘制球体，并混合
    glDisable(GL_TEXTURE_GEN_S);// 让OpenGL回到默认的属性
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_BLEND);
}

//绘制地板
void MyGLWidget::drawFloor()
{
    glBindTexture(GL_TEXTURE_2D, m_textures[0]);				// 选择地板纹理，地板由一个长方形组成
    glBegin(GL_QUADS);
        glNormal3f(0.0, 1.0, 0.0);
        glTexCoord2f(0.0f, 1.0f);// 左下
        glVertex3f(-2.0, 0.0, 2.0);
        glTexCoord2f(0.0f, 0.0f);// 左上
        glVertex3f(-2.0, 0.0,-2.0);
        glTexCoord2f(1.0f, 0.0f);// 右上
        glVertex3f( 2.0, 0.0,-2.0);
        glTexCoord2f(1.0f, 1.0f);// 右下
        glVertex3f( 2.0, 0.0, 2.0);
    glEnd();
}

//我真的希望你能喜欢这个教程，我清楚地知道我想做的每一件事，以及如何一步一步实现我心中想创建的效果。但把它表达出来又是另一回事，
//当你坐下来并实际的去向那些从来没听到过蒙板缓存的人解释这一切时，你就会清楚了。
//好了，如果你有什么不清楚的，或者有更好的建议，请让我知道，我想些最好的教程，你的反馈很重要！
