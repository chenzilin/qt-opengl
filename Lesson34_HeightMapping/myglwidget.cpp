#include "myglwidget.h"

//从高度图生成地形:
//这一课将教会你如何从一个2D的灰度图创建地形
//欢迎来到新的一课，Ben Humphrey写了这一课的代码，它是基于第一课所写的。
//在这一课里，我们将教会你如何使用地形，你将知道高度图这个概念。

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false),m_bRender(true),
    m_scaleValue(0.15f)
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
    gluPerspective(45.0f,(GLfloat)w/(GLfloat)h, 0.1f, 500.0f);

    glMatrixMode(GL_MODELVIEW);	//选择模型观察矩阵
    glLoadIdentity(); // 重置模型观察矩阵
}

void MyGLWidget::initializeGL()
{
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // 载入1024*1024的高度图道g_HeightMap数组中
    loadRawFile(":/data/Terrain.raw", MAP_SIZE * MAP_SIZE, m_gHeightMap);
}

//DrawGLScene函数基本没变化，只是设置了视点和缩放系数，调用上面的函数绘制出地形。
void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // 设置视点
    gluLookAt(212, 60, 194,  186, 55, 171,  0, 1, 0);

    glScalef(m_scaleValue, m_scaleValue * HEIGHT_RATIO, m_scaleValue);

    // 渲染高度图
    renderHeightMap(m_gHeightMap);
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
            m_scaleValue += 0.001f;
            updateGL();
            break;
        }
        case Qt::Key_Down:
        {
            m_scaleValue -= 0.001f;
            updateGL();
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

void MyGLWidget::mousePressEvent(QMouseEvent *event)
{
    m_bRender = !m_bRender;
    updateGL();
    QGLWidget::mousePressEvent(event);
}

void MyGLWidget::loadRawFile(const QString &fileName, int nSize, unsigned char *pHeightMap)
{
    QFile pFile(fileName);
    // 打开文件
    if(!pFile.open(QIODevice::ReadOnly))
    {
        // 如果文件不能打开
        QMessageBox::critical(this, tr("Error"), tr("Can't find the height map!"));
        return;
    }
    // 读取文件数据到pHeightMap数组中
    pFile.read((char *)pHeightMap, nSize);

    // 读取是否成功
    QFileDevice::FileError error = pFile.error();
    if (error != QFileDevice::NoError)
    {
        // 如果不成功，提示错误退出
        QMessageBox::critical(this, tr("Error"), tr("Failed to get data!"));
    }

    // 关闭文件
    pFile.close();
}

// 下面的函数返回(x,y)点的高度
int MyGLWidget::height(unsigned char *pHeightMap, int x, int y)
{
    int tmp_x = x % MAP_SIZE;// 限制X的值在0-1024之间
    int tmp_y = y % MAP_SIZE;// 限制Y的值在0-1024之间

    // 检测高度图是否存在，不存在则返回0
    if(!pHeightMap)
    {
        return 0;
    }
    return pHeightMap[tmp_x + (tmp_y * MAP_SIZE)];// 返回（x,y)的高度
}

// 按高度设置顶点的颜色，越高的地方越亮
void MyGLWidget::setVertexColor(unsigned char *pHeightMap, int x, int y)
{
    if(!pHeightMap)
    {
        return;
    }
    float fColor = -0.15f + (height(pHeightMap, x, y ) / 256.0f);

    // 设置顶点的颜色
    glColor3f(0, 0, fColor );
}

// 根据高度图渲染输出地形
void MyGLWidget::renderHeightMap(unsigned char *pHeightMap)
{
    // 确认高度图存在
    if(!pHeightMap)
    {
        return;
    }

    // 选择渲染模式
    if(m_bRender)
    {
        // 渲染为四边形
        glBegin( GL_QUADS );
    }
    else
    {
        // 渲染为直线
        glBegin( GL_LINES );
    }

    int x, y, z;
    for ( int i = 0; i < (MAP_SIZE-STEP_SIZE); i += STEP_SIZE )
    {
        for ( int j = 0; j < (MAP_SIZE-STEP_SIZE); j += STEP_SIZE )
        {
            // 绘制(x,y)处的顶点
            // 获得(x,y,z)坐标
            x = i;
            y = height(pHeightMap, i, j );
            z = j;
            // 设置顶点颜色
            setVertexColor(pHeightMap, x, z);
            // 调用OpenGL绘制顶点的命令
            glVertex3i(x, y, z);

            // 绘制(x,y+1)处的顶点
            x = i;
            y = height(pHeightMap, i, j + STEP_SIZE );
            z = j + STEP_SIZE ;
            setVertexColor(pHeightMap, x, z);
            glVertex3i(x, y, z);

            // 绘制(x+1,y+1)处的顶点
            x = i + STEP_SIZE;
            y = height(pHeightMap, i + STEP_SIZE, j + STEP_SIZE );
            z = j + STEP_SIZE ;
            setVertexColor(pHeightMap, x, z);
            glVertex3i(x, y, z);

            // 绘制(x+1,y)处的顶点
            x = i + STEP_SIZE;
            y = height(pHeightMap, i + STEP_SIZE, j );
            z = j;
            setVertexColor(pHeightMap, x, z);
            glVertex3i(x, y, z);
        }
    }

    glEnd();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);// 重置颜色
}

//上面就是所有绘制地形的代码了，简单吧。
//希望你喜欢这个教程！
