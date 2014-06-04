#include "myglwidget.h"

//变形和从文件中加载3D物体:
//在这一课中，你将学会如何从文件加载3D模型，并且平滑的从一个模型变换为另一个模型。

//欢迎来到这激动人心的一课，在这一课里，我们将介绍模型的变形。需要注意的是各个模型必须要有相同的顶点，才能一一对应，并应用变形。
//在这一课里，我们同样要教会你如何从一个文件中读取模型数据。
//文件开始的部分和前面一样，没有任何变化。

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_xrot(0.0f), m_yrot(0.0f), m_zrot(0.0f),
    m_xspeed(0.0f), m_yspeed(0.0f), m_zspeed(0.0f), m_cx(0.0f), m_cy(0.0f), m_cz(-15.0f),
    m_key(1), m_step(0), m_steps(200), m_morph(false)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
    objfree(&m_morph1);								// 释放模型1内存
    objfree(&m_morph2);								// 释放模型2内存
    objfree(&m_morph3);								// 释放模型3内存
    objfree(&m_morph4);								// 释放模型4内存
    objfree(&m_helper);								// 释放模型5内存
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

//下面的函数完成初始化功能，它设置混合模式为半透明
void MyGLWidget::initializeGL()
{
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);// 设置半透明混合模式
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);// 设置清除色为黑色
    glClearDepth(1.0);// 设置深度缓存中值为1
    glDepthFunc(GL_LESS);// 设置深度测试函数
    glEnable(GL_DEPTH_TEST);// 启用深度测试
    glShadeModel(GL_SMOOTH);// 设置着色模式为光滑着色
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    //下面的代码用来加载我们的模型物体
    m_maxver=0;// 初始化最大顶点数为0
    objload(":/data/Sphere.txt",&m_morph1);// 加载球模型
    objload(":/data/Torus.txt",&m_morph2);// 加载圆环模型
    objload(":/data/Tube.txt",&m_morph3);// 加载立方体模型
    //第四个模型不从文件读取，我们在（-7，-7，-7）-（7，7，7）之间随机生成模型点,它和我们载如的模型都一样具有486个顶点。
    objallocate(&m_morph4,486);							// 为第四个模型分配内存资源
    for(int i=0;i<486;i++)							// 随机设置486个顶点
    {
        m_morph4.points[i].x=((float)(qrand()%14000)/1000)-7;
        m_morph4.points[i].y=((float)(qrand()%14000)/1000)-7;
        m_morph4.points[i].z=((float)(qrand()%14000)/1000)-7;
    }
    //初始化中间模型为球体，并把原和目标模型都设置为球
    objload(":/data/Sphere.txt",&m_helper);
    m_sour=m_dest=&m_morph1;
}

//下面是具体的绘制代码，向往常一样我们先设置模型变化，以便我们更好的观察。
void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// 清空缓存
    glLoadIdentity();// 重置模型变换矩阵
    glTranslatef(m_cx,m_cy,m_cz);// 平移和旋转
    glRotatef(m_xrot,1,0,0);
    glRotatef(m_yrot,0,1,0);
    glRotatef(m_zrot,0,0,1);
    m_xrot+=m_xspeed;
    m_yrot+=m_yspeed;
    m_zrot+=m_zspeed;// 根据旋转速度，增加旋转角度
    GLfloat tx,ty,tz;// 顶点临时变量
    VERTEX q;// 保存中间计算的临时顶点
    //接下来我们来绘制模型中的点，如果启用了变形，则计算变形的中间过程点。
    glBegin(GL_POINTS);	// 点绘制开始
        for(int i=0;i<m_morph1.verts;i++)// 循环绘制模型1中的每一个顶点
        {
            if(m_morph)// 如果启用变形，则计算中间模型
            {
                q=calculate(i);
            }
            else
            {
                q.x=q.y=q.z=0;
            }
            m_helper.points[i].x-=q.x;
            m_helper.points[i].y-=q.y;
            m_helper.points[i].z-=q.z;
            tx=m_helper.points[i].x;// 保存计算结果到x,y,z变量中
            ty=m_helper.points[i].y;
            tz=m_helper.points[i].z;
            //为了让动画开起来流畅，我们一共绘制了三个中间状态的点。让变形过程从蓝绿色向蓝色下一个状态变化。
            glColor3f(0,1,1);// 设置颜色
            glVertex3f(tx,ty,tz);// 绘制顶点
            glColor3f(0,0.5f,1);// 把颜色变蓝一些
            tx-=2*q.x;
            ty-=2*q.y;
            ty-=2*q.y;// 如果启用变形，则绘制2步后的顶点
            glVertex3f(tx,ty,tz);
            glColor3f(0,0,1);// 把颜色变蓝一些
            tx-=2*q.x;
            ty-=2*q.y;
            ty-=2*q.y;// 如果启用变形，则绘制2步后的顶点
            glVertex3f(tx,ty,tz);
        }
    glEnd();
    //最后如果启用了变形，则增加递增的步骤参数，然后绘制下一个点。
    // 如果启用变形则把变形步数增加
    if(m_morph && m_step<=m_steps)
    {
        m_step++;
    }
    else
    {
        m_morph = false;
        m_sour = m_dest;
        m_step=0;
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
        case Qt::Key_PageUp:
        {
            m_zspeed+=0.1f;// 按下增加绕z轴旋转的速度
            break;
        }
        case Qt::Key_PageDown:
        {
            m_zspeed-=0.1f;// 按下减少绕z轴旋转的速度
            break;
        }
        case Qt::Key_Up:
        {
            m_xspeed-=0.1f;// 按下减少绕x轴旋转的速度
            break;
        }
        case Qt::Key_Down:
        {
            m_xspeed+=0.1f;// 按下增加绕x轴旋转的速度
            break;
        }
        case Qt::Key_Left:
        {
            m_yspeed-=0.1f;// 按下减少沿y轴旋转的速度
            break;
        }
        case Qt::Key_Right:
        {
            m_yspeed+=0.1f;// 按下增加沿y轴旋转的速度
            break;
        }
        case Qt::Key_Q:
        {
            m_cz-=0.1f;// 是则向屏幕里移动
            break;
        }
        case Qt::Key_Z:
        {
            m_cz+=0.1f;// 是则向屏幕外移动
            break;
        }
        case Qt::Key_W:
        {
            m_cy+=0.1f;// 是则向上移动
            break;
        }
        case Qt::Key_S:
        {
            m_cy-=0.1f;// 是则向下移动
            break;
        }
        case Qt::Key_D:
        {
            m_cx+=0.1f;// 是则向右移动
            break;
        }
        case Qt::Key_A:
        {
            m_cx-=0.1f;// 是则向左移动
            break;
        }
        case Qt::Key_1:
        {
            if(m_key != 1 && !m_morph)
            {
                // 如果1被按下，则变形到模型1
                m_key=1;
                m_morph=true;
                m_dest=&m_morph1;
            }
            break;
        }
        case Qt::Key_2:
        {
            if(m_key != 2 && !m_morph)
            {
                // 如果2被按下，则变形到模型2
                m_key=2;
                m_morph=true;
                m_dest=&m_morph2;
            }
            break;
        }
        case Qt::Key_3:
        {
            if(m_key != 3 && !m_morph)
            {
                // 如果3被按下，则变形到模型3
                m_key=3;
                m_morph=true;
                m_dest=&m_morph3;
            }
            break;
        }
        case Qt::Key_4:
        {
            if(m_key != 4 && !m_morph)
            {
                // 如果4被按下，则变形到模型4
                m_key=4;
                m_morph=true;
                m_dest=&m_morph4;
            }
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

//下面的函数用来为模型分配保存顶点数据的内存空间
void MyGLWidget::objallocate(OBJECT *k,int n)
{
    k->points=(VERTEX*)malloc(sizeof(VERTEX)*n);// 分配n个顶点的内存空间
}

//下面的函数用来释放为模型分配的内存空间
void MyGLWidget::objfree(OBJECT *k)
{
    free(k->points);
}

//下面的代码用来读取文件中的一行。
//我们用一个循环来读取字符，最多读取255个字符，当遇到'\n'回车时，停止读取并立即返回。
void MyGLWidget::readstr(QTextStream *stream, QString &string)// 读取一行字符
{
    do
    {
        string = stream->readLine(255);// 最多读取255个字符
    } while ((string[0] == '/') || (string[0] == '\n'));// 遇到回车则停止读取
}

//下面的代码用来加载一个模型文件，并为模型分配内存，把数据存储进去。
void MyGLWidget::objload(QString name, OBJECT *k)
{
    int	ver;// 保存顶点个数
    float rx,ry,rz;// 保存模型位置
    QFile file(name);// 打开的文件
    QString oneline;// 保存255个字符
    file.open(QIODevice::ReadOnly);// 打开文本文件，供读取
    QTextStream stream(&file);
    readstr(&stream, oneline);// 读入一行文本
    sscanf(oneline.toLatin1().data(), "Vertices: %d\n", &ver);// 搜索字符串"Vertices: "，并把其后的顶点数保存在ver变量中
    k->verts=ver;// 设置模型的顶点个数
    objallocate(k,ver);// 为模型数据分配内存
    //下面的循环，读取每一行（即每个顶点）的数据，并把它保存到内存中?
    for (int i=0;i<ver;i++)	// 循环所有的顶点
    {
        readstr(&stream, oneline);// 读取一行数据
        sscanf(oneline.toLatin1().data(), "%f %f %f", &rx, &ry, &rz);// 把顶点数据保存在rx,ry,rz中
        k->points[i].x = rx;// 保存当前顶点的x坐标
        k->points[i].y = ry;// 保存当前顶点的y坐标
        k->points[i].z = rz;// 保存当前顶点的z坐标
    }
    file.close();// 关闭文件
    if(ver>m_maxver)
    {
        m_maxver = ver;// 记录最大的顶点数
    }
}

//下面的函数根据设定的间隔，计算第i个顶点每次变换的位移
MyGLWidget::VERTEX MyGLWidget::calculate(int i)	// 计算第i个顶点每次变换的位移
{
    VERTEX a;
    a.x=(m_sour->points[i].x-m_dest->points[i].x)/m_steps;
    a.y=(m_sour->points[i].y-m_dest->points[i].y)/m_steps;
    a.z=(m_sour->points[i].z-m_dest->points[i].z)/m_steps;
    return a;
}

//我希望你能喜欢这个教程，相信你已经学会了变形动画。
//Piotr Cieslak 的代码非常的新颖，希望通过这个教程你能知道如何从文件中加载三维模型。
//这份教程化了我三天的时间，如果有什么错误请告诉我。
