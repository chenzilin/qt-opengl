#include "myglwidget.h"

//加载3D世界，并在其中漫游:
//在这一课中，你将学会如何加载3D世界，并在3D世界中漫游。
//这一课使用第一课的代码，当然在课程说明中我只介绍改变了代码。

//这一课是由Lionel Brits (βtelgeuse)所写的。在本课中我们只对增加的代码做解释。当然只添加课程中所写的代码，程序是不会运行的。
//如果您有兴趣知道下面的每一行代码是如何运行的话，请下载完整的源码，并在浏览这一课的同时，对源码进行跟踪。
//好了现在欢迎来到名不见经传的第十课。到现在为止，您应该有能力创建一个旋转的立方体或一群星星了，对3D编程也应该有些感觉了吧？
//但还是请等一下！不要立马冲动地要开始写个Quake
//IV，好不好...:)。只靠旋转的立方体还很难来创造一个可以决一死战的酷毙了的对手....:)。
//现在这些日子您所需要的是一个大一点的、更复杂些的、动态3D世界，它带有空间的六自由度和花哨的效果如镜像、入口、扭曲等等，
//当然还要有更快的帧显示速度。这一课就要解释一个基本的3D世界"结构"，以及如何在这个世界里游走。
//数据结构
//当您想要使用一系列的数字来完美的表达3D环境时，随着环境复杂度的上升，这个工作的难度也会随之上升。
//出于这个原因，我们必须将数据归类,使其具有更多的可操作性风格。在程序清单头部出现了sector(区段)的定义。
//每个3D世界基本上可以看作是sector(区段)的集合。一个sector(区段)可以是一个房间、一个立方体、或者任意一个闭合的区间。

const float piover180 = 0.0174532925f;

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_yrot(0.0f),
    m_xpos(0.0f), m_zpos(0.0f), m_heading(0.0f), m_walkbias(0.0f),
    m_walkbiasangle(0.0f), m_lookupdown(0.0f), m_filter(0),m_blend(false)
{
    showNormal();
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
    gluPerspective(45.0f,(GLfloat)w/(GLfloat)h,0.1f,100.0f);

    glMatrixMode(GL_MODELVIEW);	//选择模型观察矩阵
    glLoadIdentity(); // 重置模型观察矩阵
}

//接下的代码段中，我们将对OpenGL进行所有的设置。我们将设置清除屏幕所用的颜色，打开深度缓存，启用smooth shading(阴影平滑)，等等。
//这个例程直到OpenGL窗口创建之后才会被调用。此过程将有返回值。但我们此处的初始化没那么复杂，现在还用不着担心这个返回值。
void MyGLWidget::initializeGL()
{
    loadTexture();
    glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);					// Set The Blending Function For Translucency
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);				// This Will Clear The Background Color To Black
    glClearDepth(1.0);									// Enables Clearing Of The Depth Buffer
    glDepthFunc(GL_LESS);								// The Type Of Depth Test To Do
    glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
    glShadeModel(GL_SMOOTH);							// Enables Smooth Color Shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

    setupWorld();

    m_blend=!m_blend;
    if (!m_blend)
    {
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
    }
}

//下一段包括了所有的绘图代码。任何您所想在屏幕上显示的东东都将在此段代码中出现。
//以后的每个教程中我都会在例程的此处增加新的代码。如果您对OpenGL已经有所了解的话，您可以在glLoadIdentity()调用之后，
//试着添加一些OpenGL代码来创建基本的形。
//如果您是OpenGL新手，等着我的下个教程。目前我们所作的全部就是将屏幕清除成我们前面所决定的颜色，清除深度缓存并且重置场景。
//我们仍没有绘制任何东东。
void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// 清除 场景 和 深度缓冲
    glLoadIdentity();						// 重置当前矩阵

    GLfloat x_m, y_m, z_m, u_m, v_m;				// 顶点的临时 X, Y, Z, U 和 V 的数值
    GLfloat xtrans = -m_xpos;						// 用于游戏者沿X轴平移时的大小
    GLfloat ztrans = -m_zpos;						// 用于游戏者沿Z轴平移时的大小
    GLfloat ytrans = -m_walkbias-0.25f;				// 用于头部的上下摆动
    GLfloat sceneroty = 360.0f - m_yrot;				// 位于游戏者方向的360度角
    int numtriangles;						// 保有三角形数量的整数
    glRotatef(m_lookupdown, 1.0f, 0,0);					// 上下旋转
    glRotatef(sceneroty, 0, 1.0f, 0);					// 根据游戏者正面所对方向所作的旋转
    glTranslatef(xtrans, ytrans, ztrans);				// 以游戏者为中心的平移场景
    glBindTexture(GL_TEXTURE_2D, m_texture[m_filter]);			// 根据 filter 选择的纹理
    numtriangles = m_sector1.numtriangles;				// 取得Sector1的三角形数量
    for (int loop_m = 0; loop_m < numtriangles; loop_m++)		// 遍历所有的三角形
    {
        glBegin(GL_TRIANGLES);					// 开始绘制三角形
            glNormal3f( 0.0f, 0.0f, 1.0f);			// 指向前面的法线
            x_m = m_sector1.triangle[loop_m].vertex[0].x;	// 第一点的 X 分量
            y_m = m_sector1.triangle[loop_m].vertex[0].y;	// 第一点的 Y 分量
            z_m = m_sector1.triangle[loop_m].vertex[0].z;	// 第一点的 Z 分量
            u_m = m_sector1.triangle[loop_m].vertex[0].u;	// 第一点的 U  纹理坐标
            v_m = m_sector1.triangle[loop_m].vertex[0].v;	// 第一点的 V  纹理坐标

            glTexCoord2f(u_m,v_m); glVertex3f(x_m,y_m,z_m);	// 设置纹理坐标和顶点
            x_m = m_sector1.triangle[loop_m].vertex[1].x;	// 第二点的 X 分量
            y_m = m_sector1.triangle[loop_m].vertex[1].y;	// 第二点的 Y 分量
            z_m = m_sector1.triangle[loop_m].vertex[1].z;	// 第二点的 Z 分量
            u_m = m_sector1.triangle[loop_m].vertex[1].u;	// 第二点的 U  纹理坐标
            v_m = m_sector1.triangle[loop_m].vertex[1].v;	// 第二点的 V  纹理坐标

            glTexCoord2f(u_m,v_m); glVertex3f(x_m,y_m,z_m);	// 设置纹理坐标和顶点
            x_m = m_sector1.triangle[loop_m].vertex[2].x;	// 第三点的 X 分量
            y_m = m_sector1.triangle[loop_m].vertex[2].y;	// 第三点的 Y 分量
            z_m = m_sector1.triangle[loop_m].vertex[2].z;	// 第三点的 Z 分量
            u_m = m_sector1.triangle[loop_m].vertex[2].u;	// 第二点的 U  纹理坐标
            v_m = m_sector1.triangle[loop_m].vertex[2].v;	// 第二点的 V  纹理坐标
            glTexCoord2f(u_m,v_m); glVertex3f(x_m,y_m,z_m);	// 设置纹理坐标和顶点
        glEnd();						// 三角形绘制结束
    }
}

//显示世界
//现在区段已经载入内存，我们下一步要在屏幕上显示它。到目前为止，我们所作过的都是些简单的旋转和平移。
//但我们的镜头始终位于原点(0，0，0)处。任何一个不错的3D引擎都会允许用户在这个世界中游走和遍历，我们的这个也一样。
//实现这个功能的一种途径是直接移动镜头并绘制以镜头为中心的3D环境。这样做会很慢并且不易用代码实现。我们的解决方法如下：
//根据用户的指令旋转并变换镜头位置。
//围绕原点，以与镜头相反的旋转方向来旋转世界。(让人产生镜头旋转的错觉)
//以与镜头平移方式相反的方式来平移世界(让人产生镜头移动的错觉)。

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
            m_lookupdown-=1.0f;
            updateGL();
            break;
        }
        case Qt::Key_PageDown:
        {
            m_lookupdown+=1.0f;
            updateGL();
            break;
        }
        case Qt::Key_Right:
        {
            m_heading -=1.0f;
            m_yrot = m_heading;							// 向左旋转场景
            updateGL();
            break;
        }
        case Qt::Key_Left:
        {
            m_heading += 1.0f;
            m_yrot = m_heading;							// 向右侧旋转场景
            updateGL();
            break;
        }
        case Qt::Key_Up:
        {
            m_xpos -= (float)sin(m_heading*piover180) * 0.05f;			// 沿游戏者所在的X平面移动
            m_zpos -= (float)cos(m_heading*piover180) * 0.05f;			// 沿游戏者所在的Z平面移动
            if (m_walkbiasangle >= 359.0f)					// 如果walkbiasangle大于359度
            {
                m_walkbiasangle = 0.0f;					// 将 walkbiasangle 设为0
            }
            else								// 否则
            {
                 m_walkbiasangle+= 10;					// 如果 walkbiasangle < 359 ，则增加 10
            }
            m_walkbias = (float)sin(m_walkbiasangle * piover180)/20.0f;		// 使游戏者产生跳跃感
            updateGL();
            break;
        }
        case Qt::Key_Down:
        {
            m_xpos += (float)sin(m_heading*piover180) * 0.05f;			// 沿游戏者所在的X平面移动
            m_zpos += (float)cos(m_heading*piover180) * 0.05f;			// 沿游戏者所在的Z平面移动
            if (m_walkbiasangle <= 1.0f)					// 如果walkbiasangle小于1度
            {
                m_walkbiasangle = 359.0f;					// 使 walkbiasangle 等于 359
            }
            else								// 否则
            {
                m_walkbiasangle-= 10;					// 如果 walkbiasangle > 1 减去 10
            }
            m_walkbias = (float)sin(m_walkbiasangle * piover180)/20.0f;		// 使游戏者产生跳跃感
            updateGL();
            break;
        }
        //这个实现很简单。当左右方向键按下后，旋转变量yrot
        //相应增加或减少。当前后方向键按下后，我们使用sine和cosine函数重新生成镜头位置(您需要些许三角函数学的知识:-)。Piover180
        //是一个很简单的折算因子用来折算度和弧度。
        //接着您可能会问：walkbias是什么意思？这是NeHe的发明的单词:-)。基本上就是当人行走时头部产生上下摆动的幅度。
        //我们使用简单的sine正弦波来调节镜头的Y轴位置。如果不添加这个而只是前后移动的话，程序看起来就没这么棒了。
        case Qt::Key_B:
        {
            m_blend=!m_blend;
            if (!m_blend)
            {
                glDisable(GL_BLEND);
                glEnable(GL_DEPTH_TEST);
            }
            else
            {
                glEnable(GL_BLEND);
                glDisable(GL_DEPTH_TEST);
            }
            updateGL();
            break;
        }
        case Qt::Key_F:
        {
            m_filter+=1;
            if(m_filter > 2)
            {
                m_filter = 0;
            }
            updateGL();
        }
    }
}

//载入文件
//在程序内部直接存储数据会让程序显得太过死板和无趣。从磁盘上载入世界资料，会给我们带来更多的弹性，可以让我们体验不同的世界，
//而不用被迫重新编译程序。另一个好处就是用户可以切换世界资料并修改它们而无需知道程序如何读入输出这些资料的。
//数据文件的类型我们准备使用文本格式。这样编辑起来更容易，写的代码也更少。等将来我们也许会使用二进制文件。
//问题是，怎样才能从文件中取得数据资料呢？首先，创建一个叫做SetupWorld()的新函数。把这个文件定义为filein，并且使用只读方式打开文件。
//我们必须在使用完毕之后关闭文件。大家一起来看看现在的代码：
void MyGLWidget::setupWorld()
{
    QFile file(":/world/World.txt");
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, tr("Warning"), tr("Can't open world file."));
        return;
    }

    QTextStream stream(&file);
    //我们对区段进行初始化，并读入部分数据
    QString oneline;							// 存储数据的字符串
    int numtriangles;							// 区段的三角形数量
    float x, y, z, u, v;							// 3D 和 纹理坐标

    readStr(&stream, oneline); // 读入一行数据
    sscanf(oneline.toLatin1().data(), "NUMPOLLIES %d\n", &numtriangles); // 读入三角形数量

    m_sector1.triangle = new TRIANGLE[numtriangles];				// 为numtriangles个三角形分配内存并设定指针
    m_sector1.numtriangles = numtriangles;					// 定义区段1中的三角形数量
    // 遍历区段中的每个三角形
    for (int triloop = 0; triloop < numtriangles; triloop++)		// 遍历所有的三角形
    {
        // 遍历三角形的每个顶点
        for (int vertloop = 0; vertloop < 3; vertloop++)		// 遍历所有的顶点
        {
            readStr(&stream, oneline);				// 读入一行数据
            // 读入各自的顶点数据
            sscanf(oneline.toLatin1().data(), "%f %f %f %f %f", &x, &y, &z, &u, &v);
            // 将顶点数据存入各自的顶点
            m_sector1.triangle[triloop].vertex[vertloop].x = x;	// 区段 1,  第 triloop 个三角形, 第  vertloop 个顶点, 值 x=x
            m_sector1.triangle[triloop].vertex[vertloop].y = y;	// 区段 1,  第 triloop 个三角形, 第  vertloop 个顶点, 值 y=y
            m_sector1.triangle[triloop].vertex[vertloop].z = z;	// 区段 1,  第 triloop 个三角形, 第  vertloop 个顶点, 值 z=z
            m_sector1.triangle[triloop].vertex[vertloop].u = u;	// 区段 1,  第 triloop 个三角形, 第  vertloop 个顶点, 值 u=u
            m_sector1.triangle[triloop].vertex[vertloop].v = v;	// 区段 1,  第 triloop 个三角形, 第  vertloop 个顶点, 值 v=v
        }
    }
    //数据文件中每个三角形都以如下形式声明:
    //X1 Y1 Z1 U1 V1
    //X2 Y2 Z2 U2 V2
    //X3 Y3 Z3 U3 V3
    file.close();
}

//将每个单独的文本行读入变量。这有很多办法可以做到。一个问题是文件中并不是所有的行都包含有意义的信息。
//空行和注释不应该被读入。我们创建了一个叫做readstr()的函数。这个函数会从数据文件中读入一个有意义的行至一个已经初始化过的字符串。
//下面就是代码：
void MyGLWidget::readStr(QTextStream *stream, QString &string)
{
    do								// 循环开始
    {
        string = stream->readLine();
    } while (string[0] == '/' || string[0] == '\n' || string.isEmpty());		// 考察是否有必要进行处理
}

void MyGLWidget::loadTexture()
{
    QImage image(":/image/Crate.bmp");
    image = image.convertToFormat(QImage::Format_RGB888);
    image = image.mirrored();
    glGenTextures(1, &m_texture[0]);// 创建纹理
    // Create Nearest Filtered Texture
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image.width(), image.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, image.bits());

    // Create Linear Filtered Texture
    glBindTexture(GL_TEXTURE_2D, m_texture[1]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image.width(), image.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, image.bits());

    // Create MipMapped Texture
    glBindTexture(GL_TEXTURE_2D, m_texture[2]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image.width(), image.height(),
                      GL_RGB, GL_UNSIGNED_BYTE, image.bits());
}

//搞定！我们已经完成了自己的第一帧画面。这绝对算不上什么Quake，但咳...,我们绝对也不是Carmack或者Abrash。
//运行程序时，您可以按下F、B、
//PgUp 和 PgDown 键来看看效果。PgUp /
//PgDown简单的上下倾斜镜头。如果NeHe决定保留的话，程序中使用的纹理取自于我的学校ID证件上的照片，并且做了浮雕效果....:)。
//现在您也许在考虑下一步该做什么。但还是不要考虑使用这些代码来实现完整的3D引擎，写这个程序的目的也并非如此。
//您也许希望您的游戏中不止存在一个Sector，尤其是实现类似入口这样的部分，您还可能需要使用多边形(超过3个顶点)。
//程序现在的代码实现允许载入多个Sector并剔除了背面(背向镜头不用绘制的多边形)。将来我会写个这样的教程，但这需要更多的数学知识基础。
