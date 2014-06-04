#include "myglwidget.h"

//贝塞尔曲面:
//这是一课关于数学运算的，没有别的内容了。来，有信心就看看它吧。
//贝塞尔曲面
//作者: David Nikdel ( ogapo@ithink.net )
//这篇教程旨在介绍贝塞尔曲面，希望有比我更懂艺术的人能用她作出一些很COOL的东东并且展示给大家。教程不能用做一个完整的贝塞尔曲面库，
//而是一个展示概念的程序让你熟悉曲面怎样实现的。而且这不是一篇正规的文章，为了方便理解，我也许在有些地方术语不当；
//我希望大家能适应这个。最后，对那些已经熟悉贝塞尔曲面想看我写的如何的，真是丢脸；-）但你要是找到任何纰漏让我或者NeHe知道，
//毕竟人无完人嘛？还有，所有代码没有象我一般写程序那样做优化，这是故意的。我想每个人都能明白写的是什么。好，我想介绍到此为止，
//继续看下文！

//以下是一些简单的向量数学的函数。如果你是C++爱好者你可以用一个顶点类（保证其为3D的）。
// 两个向量相加，p=p+q
POINT_3D pointAdd(POINT_3D p, POINT_3D q)
{
    p.x += q.x;
    p.y += q.y;
    p.z += q.z;
    return p;
}
// 向量和标量相乘p=c*p
POINT_3D pointTimes(double c, POINT_3D p)
{
    p.x *= c;
    p.y *= c;
    p.z *= c;
    return p;
}

// 创建一个3D向量

POINT_3D makePoint(double a, double b, double c)
{
    POINT_3D p;
    p.x = a;
    p.y = b;
    p.z = c;
    return p;
}

// 计算贝塞尔方程的值
// 变量u的范围在0-1之间
POINT_3D Bernstein(float u, POINT_3D *p)
{
    POINT_3D a, b, c, d, r;
    a = pointTimes(pow(u,3), p[0]);
    b = pointTimes(3*pow(u,2)*(1-u), p[1]);
    c = pointTimes(3*u*pow((1-u),2), p[2]);
    d = pointTimes(pow((1-u),3), p[3]);
    r = pointAdd(pointAdd(a, b), pointAdd(c, d));
    return r;
}


MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false),m_showCPoints(true), m_divs(7),
    m_rotz(0.0f)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
    if(m_mybezier.dlBPatch != NULL)// 如果显示列表存在则删除
    {
        glDeleteLists(m_mybezier.dlBPatch, 1);
    }
    glDeleteTextures(1, &m_mybezier.texture);
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

//仅仅加了曲面初始化在这。你每次建一个曲面时都会用这个。再一次，这里是一个用C++的好地方（贝塞尔曲面类？）。
void MyGLWidget::initializeGL()
{
    glEnable(GL_TEXTURE_2D);// 使用2D纹理
    glShadeModel(GL_SMOOTH);// 使用平滑着色
    glClearColor(0.05f, 0.05f, 0.05f, 0.5f);// 设置黑色背景
    glClearDepth(1.0f);// 设置深度缓存
    glEnable(GL_DEPTH_TEST);// 使用深度缓存
    glDepthFunc(GL_LEQUAL);// 设置深度方程
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    initBezier();// 初始化贝塞尔曲面
    loadGLTexture();// 载入纹理
    m_mybezier.dlBPatch = genBezier(m_mybezier, m_divs);		// 创建显示列表
}

//首先调贝塞尔display list。再（如果边线要画）画连接控制点的线。你可以用SPACE键开关这个。
void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f,0.0f,-4.0f);// 移入屏幕4个单位
    glRotatef(-75.0f,1.0f,0.0f,0.0f);
    glRotatef(m_rotz,0.0f,0.0f,1.0f);// 旋转一定的角度
    glCallList(m_mybezier.dlBPatch);// 调用显示列表，绘制贝塞尔曲面
    if (m_showCPoints)
    {
        // 是否绘制控制点
        glDisable(GL_TEXTURE_2D);
        glColor3f(1.0f,0.0f,0.0f);
        for(int i=0;i<4;i++)
        {
            // 绘制水平线
            glBegin(GL_LINE_STRIP);
                for(int j=0;j<4;j++)
                {
                    glVertex3d(m_mybezier.anchors[i][j].x,
                               m_mybezier.anchors[i][j].y,
                               m_mybezier.anchors[i][j].z);
                }
            glEnd();
        }
        for(int i=0;i<4;i++)
        {
            // 绘制垂直线
            glBegin(GL_LINE_STRIP);
                for(int j=0;j<4;j++)
                {
                    glVertex3d(m_mybezier.anchors[j][i].x,
                               m_mybezier.anchors[j][i].y,
                               m_mybezier.anchors[j][i].z);
                }
            glEnd();
        }
        glColor3f(1.0f,1.0f,1.0f);
        glEnable(GL_TEXTURE_2D);
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
        case Qt::Key_Left:
        {
            m_rotz -= 0.8f;
            break;
        }
        case Qt::Key_Right:
        {
            m_rotz += 0.8f;
            break;
        }
        case Qt::Key_Up:
        {
            m_divs++;
            m_mybezier.dlBPatch = genBezier(m_mybezier, m_divs);	// 更新贝塞尔曲面的显示列表
            break;
        }
        case Qt::Key_Down:
        {
            m_divs--;
            m_mybezier.dlBPatch = genBezier(m_mybezier, m_divs);	// 更新贝塞尔曲面的显示列表
            break;
        }
        case Qt::Key_Space:
        {
            m_showCPoints = !m_showCPoints;
            break;
        }
    }
    QGLWidget::keyPressEvent(event);
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    updateGL();
    QGLWidget::timerEvent(event);
}

// 生成贝塞尔曲面的显示列表
GLuint MyGLWidget::genBezier(BEZIER_PATCH patch, int divs)
{
    float py, px, pyold;
    GLuint drawlist = glGenLists(1);// 创建显示列表
    POINT_3D temp[4];
    POINT_3D *last = (POINT_3D*)malloc(sizeof(POINT_3D)*(divs+1));// 更具每一条曲线的细分数，分配相应的内存
    if(patch.dlBPatch != NULL)// 如果显示列表存在则删除
    {
        glDeleteLists(patch.dlBPatch, 1);
    }
    temp[0] = patch.anchors[0][3];// 获得u方向的四个控制点
    temp[1] = patch.anchors[1][3];
    temp[2] = patch.anchors[2][3];
    temp[3] = patch.anchors[3][3];
    for (int v=0;v<=divs;v++)
    {
        // 根据细分数，创建各个分割点额参数
        px = ((float)v)/((float)divs);
        // 使用Bernstein函数求的分割点的坐标
        last[v] = Bernstein(px, temp);
    }
    glNewList(drawlist, GL_COMPILE);// 创建一个新的显示列表
    glBindTexture(GL_TEXTURE_2D, patch.texture);// 邦定纹理
        for (int u=1;u<=divs;u++)
        {
            py = ((float)u)/((float)divs);// 计算v方向上的细分点的参数
            pyold = ((float)u-1.0f)/((float)divs);// 上一个v方向上的细分点的参数
            temp[0] = Bernstein(py, patch.anchors[0]);		// 计算每个细分点v方向上贝塞尔曲面的控制点
            temp[1] = Bernstein(py, patch.anchors[1]);
            temp[2] = Bernstein(py, patch.anchors[2]);
            temp[3] = Bernstein(py, patch.anchors[3]);
            glBegin(GL_TRIANGLE_STRIP);// 开始绘制三角形带
                for (int v=0;v<=divs;v++)
                {
                    px = ((float)v)/((float)divs);// 沿着u轴方向顺序绘制
                    glTexCoord2f(pyold, px);// 设置纹理坐标
                    glVertex3d(last[v].x, last[v].y, last[v].z);// 绘制一个顶点
                    last[v] = Bernstein(px, temp);// 创建下一个顶点
                    glTexCoord2f(py, px);// 设置纹理
                    glVertex3d(last[v].x, last[v].y, last[v].z);// 绘制新的顶点
                }
            glEnd();// 结束三角形带的绘制
        }
    glEndList();// 显示列表绘制结束
    free(last);// 释放分配的内存
    return drawlist;// 返回创建的显示列表
}

//这里我们调用一个我认为有一些很酷的值的矩阵。
void MyGLWidget::initBezier()
{
    m_mybezier.anchors[0][0] = makePoint(-0.75,	-0.75,	-0.50);	// 设置贝塞尔曲面的控制点
    m_mybezier.anchors[0][1] = makePoint(-0.25,	-0.75,	 0.00);
    m_mybezier.anchors[0][2] = makePoint( 0.25,	-0.75,	 0.00);
    m_mybezier.anchors[0][3] = makePoint( 0.75,	-0.75,	-0.50);
    m_mybezier.anchors[1][0] = makePoint(-0.75,	-0.25,	-0.75);
    m_mybezier.anchors[1][1] = makePoint(-0.25,	-0.25,	 0.50);
    m_mybezier.anchors[1][2] = makePoint( 0.25,	-0.25,	 0.50);
    m_mybezier.anchors[1][3] = makePoint( 0.75,	-0.25,	-0.75);
    m_mybezier.anchors[2][0] = makePoint(-0.75,	 0.25,	 0.00);
    m_mybezier.anchors[2][1] = makePoint(-0.25,	 0.25,	-0.50);
    m_mybezier.anchors[2][2] = makePoint( 0.25,	 0.25,	-0.50);
    m_mybezier.anchors[2][3] = makePoint( 0.75,	 0.25,	 0.00);
    m_mybezier.anchors[3][0] = makePoint(-0.75,	 0.75,	-0.50);
    m_mybezier.anchors[3][1] = makePoint(-0.25,	 0.75,	-1.00);
    m_mybezier.anchors[3][2] = makePoint( 0.25,	 0.75,	-1.00);
    m_mybezier.anchors[3][3] = makePoint( 0.75,	 0.75,	-0.50);
    m_mybezier.dlBPatch = NULL;// 默认的显示列表为0
}

void MyGLWidget::loadGLTexture()
{
    QImage image(":/image/NeHe.bmp");
    image = image.convertToFormat(QImage::Format_RGB888);
    image = image.mirrored();
    glGenTextures(1, &m_mybezier.texture);
    // 邦定纹理
    glBindTexture(GL_TEXTURE_2D, m_mybezier.texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width(), image.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, image.bits());
}

//恩，我希望这个教程让你了然于心而且你现在象我一样喜欢上了贝塞尔曲面。；-）
//如果你喜欢这个教程我会继续写一篇关于NURBS的如果有人喜欢。请EMAIL我让我知道你怎么想这篇教程。
