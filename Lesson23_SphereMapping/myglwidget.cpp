#include "myglwidget.h"

//球面映射:
//这一个将教会你如何把环境纹理包裹在你的3D模型上，让它看起来象反射了周围的场景一样。

//球体环境映射是一个创建快速金属反射效果的方法，但它并不像真实世界里那么精确！我们从18课的代码开始来创建这个教程，教你如何创建这种效果。
//在我们开始之间，看一下红宝书中的介绍。
//它定义球体环境映射为一幅位于无限远的图像，把它映射到球面上。 在Photoshop中创建一幅球体环境映射图。
//首先，你需要一幅球体环境映射图，用来把它映射到球体上。
//在Photoshop中打开一图并选择所有的像素，创建它的一个复制。接着，我们把图像变为2的幂次方大小，一般为128x128或256x256。
//最后使用扭曲(distort)滤镜，并应用球体效果。然后把它保存为*.bmp文件。
//我们并没有添加任何全局变量，只是把纹理组的大小变为6，以保存6幅纹理。

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_light(false),
    m_x_rotate(0.0f), m_y_rotate(0.0f), m_x_speed(0.0f), m_y_speed(0.0f),
    m_z(-5.0f), m_light_ambient({0.5f, 0.5f, 0.5f, 1.0f}),
    m_light_diffuse({1.0f, 1.0f, 1.0f, 1.0f}),
    m_light_position({0.0f, 0.0f, 2.0f, 1.0f}),
    m_filter(0), m_part1(0), m_part2(0), m_p1(0), m_p2(1),
    m_object(0)
{
    showNormal();
    startTimer(50);
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(6, &m_texture[0]);
}

//在resizeGL()之前，我们增加了下面这一段代码。
//这段代码用来加载位图文件。如果文件不存在，返回 NULL 告知程序无法加载位图。在
//我开始解释这段代码之前，关于用作纹理的图像我想有几点十分重要，并且您必须明白。
//此图像的宽和高必须是2的n次方；宽度和高度最小必须是64象素；并且出于兼容性的原因，图像的宽度和高度不应超过256象素。
//如果您的原始素材的宽度和高度不是64,128,256象素的话，使用图像处理软件重新改变图像的大小。
//可以肯定有办法能绕过这些限制，但现在我们只需要用标准的纹理尺寸。
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

//好了我们现在开始搞InitGL()函数。我们打算增加3行代码用来初始化我们的二次曲面。
//这3行代码将在你使1号光源有效后增加，但是要在返回之前。
//第一行代码将初始化二次曲面并且创建一个指向改二次曲面的指针，如果改二次曲面不能被创建的话，那么该指针就是NULL。
//第二行代码将在二次曲面的表面创建平滑的法向量，这样当灯光照上去的时候将会好看些。
//另外一些可能的取值是：GLU_NONE和GLU_FLAT。最后我们使在二次曲面表面的纹理映射有效。
//在初始化OpenGL中，我们添加一些新的函数来使用球体纹理映射。
//下面的代码让OpenGL自动为我们计算使用球体映射时，顶点的纹理坐标。
void MyGLWidget::initializeGL()
{
    loadGLTexture();

    glEnable(GL_TEXTURE_2D);// 启用纹理映射
    //下一行启用smooth shading(阴影平滑)。阴影平滑通过多边形精细的混合色彩，并对外部光进行平滑。
    //我将在另一个教程中更详细的解释阴影平滑。
    glShadeModel(GL_SMOOTH); // 启用阴影平滑

    //下一行设置清除屏幕时所用的颜色。如果您对色彩的工作原理不清楚的话，我快速解释一下。
    //色彩值的范围从0.0f到1.0f。0.0f代表最黑的情况，1.0f就是最亮的情况。
    //glClearColor 后的第一个参数是Red Intensity(红色分量),第二个是绿色，第三个是蓝色。
    //最大值也是1.0f，代表特定颜色分量的最亮情况。最后一个参数是Alpha值。当它用来清除屏幕的时候，我们不用关心第四个数字。
    //现在让它为0.0f。我会用另一个教程来解释这个参数。
    //通过混合三种原色(红、绿、蓝)，您可以得到不同的色彩。希望您在学校里学过这些。
    //因此，当您使用glClearColor(0.0f,0.0f,1.0f,0.0f)，您将用亮蓝色来清除屏幕。
    //如果您用 glClearColor(0.5f,0.0f,0.0f,0.0f)的话，您将使用中红色来清除屏幕。不是最亮(1.0f)，也不是最暗 (0.0f)。
    //要得到白色背景，您应该将所有的颜色设成最亮(1.0f)。要黑色背景的话，您该将所有的颜色设为最暗(0.0f)。
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // 黑色背景

    //接下来的三行必须做的是关于depth buffer(深度缓存)的。将深度缓存设想为屏幕后面的层。
    //深度缓存不断的对物体进入屏幕内部有多深进行跟踪。
    //我们本节的程序其实没有真正使用深度缓存，但几乎所有在屏幕上显示3D场景OpenGL程序都使用深度缓存。
    //它的排序决定那个物体先画。这样您就不会将一个圆形后面的正方形画到圆形上来。深度缓存是OpenGL十分重要的部分。
    glClearDepth(1.0f);	// 设置深度缓存
    glEnable(GL_DEPTH_TEST); // 启用深度测试
    glDepthFunc(GL_LEQUAL);	// 所作深度测试的类型

    //接着告诉OpenGL我们希望进行最好的透视修正。这会十分轻微的影响性能。但使得透视图看起来好一点。
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// 告诉系统对透视进行修正

    //现在开始设置光源。下面下面一行设置环境光的发光量，光源light1开始发光。
    //这一课的开始处我们我们将环境光的发光量存放在LightAmbient数组中。
    //现在我们就使用此数组(半亮度环境光)。在int InitGL(GLvoid)函数中添加下面的代码。
    glLightfv(GL_LIGHT1, GL_AMBIENT, m_light_ambient);				// 设置环境光
    //接下来我们设置漫射光的发光量。它存放在LightDiffuse数组中(全亮度白光)。
    glLightfv(GL_LIGHT1, GL_DIFFUSE, m_light_diffuse);				// 设置漫射光
    //然后设置光源的位置。位置存放在 LightPosition 数组中(正好位于木箱前面的中心，X－0.0f，Y－0.0f，Z方向移向观察者2个单位<位于屏幕外面>)。
    glLightfv(GL_LIGHT1, GL_POSITION, m_light_position);			// 设置光源位置
    //最后，我们启用一号光源。我们还没有启用GL_LIGHTING，所以您看不见任何光线。
    //记住：只对光源进行设置、定位、甚至启用，光源都不会工作。除非我们启用GL_LIGHTING。
    glEnable(GL_LIGHT1);							// 启用一号光源
    if(!m_light)
    {
        glDisable(GL_LIGHTING);		// 禁用光源
    }
    else
    {
        glEnable(GL_LIGHTING);		// 启用光源
    }
    m_quadratic = gluNewQuadric();				// 创建二次几何体
    gluQuadricNormals(m_quadratic, GLU_SMOOTH);	// 使用平滑法线
    gluQuadricTexture(m_quadratic, GL_TRUE);	// 使用纹理

    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // 设置s方向的纹理自动生成
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // 设置t方向的纹理自动生成
}

//接下来就是场景绘制函数了，在这里我只写一个简单的例子。并且当我绘制一个部分的盘子的时候，我将使用一个静态变量
//（一个局部的变量，该变量可以保留他的值不论你任何时候调用他）来表达一个非常酷的效果。为了清晰起见我将要重写DrawGLScene函数。
//你们将会注意到当我讨论这些正在使用的参数时我忽略了当前函数的第一个参数（quadratic）。
//这个参数将被除立方体外的所有对象使用。所以当我讨论这些参数的时候我忽略了它。
//我们几乎完成了所有的工作！接下来要做的就是就是绘制渲染，我删除了一些二次几何体，因为它们的视觉效果并不好。
//当然我们需要OpenGL为这些几何体自动生成坐标，接着选择球体映射纹理并绘制几何体。最后把OpenGL状态设置正常模式。
void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();									// 重置视口

    glTranslatef(0.0f,0.0f,m_z);

    glEnable(GL_TEXTURE_GEN_S);							// 自动生成s方向纹理坐标
    glEnable(GL_TEXTURE_GEN_T);							// 自动生成t方向纹理坐标

    glBindTexture(GL_TEXTURE_2D, m_texture[m_filter+(m_filter+1)]); // 绑定纹理
    glPushMatrix();
    glRotatef(m_x_rotate,1.0f,0.0f,0.0f);
    glRotatef(m_y_rotate,0.0f,1.0f,0.0f);
    switch(m_object)
    {
    case 0:
        glDrawCube();
        break;
    case 1:
        glTranslatef(0.0f,0.0f,-1.5f);					// 创建圆柱
        gluCylinder(m_quadratic,1.0f,1.0f,3.0f,32,32);
        break;
    case 2:
        gluSphere(m_quadratic,1.3f,32,32);				// 创建球
        break;
    case 3:
        glTranslatef(0.0f,0.0f,-1.5f);					// 创建圆锥
        gluCylinder(m_quadratic,1.0f,0.0f,3.0f,32,32);
        break;
    };

    glPopMatrix();
    glDisable(GL_TEXTURE_GEN_S);                      //禁止自动生成纹理坐标
    glDisable(GL_TEXTURE_GEN_T);

    glBindTexture(GL_TEXTURE_2D, m_texture[m_filter*2]);	//绑定纹理
    glPushMatrix();
        glTranslatef(0.0f, 0.0f, -24.0f);
        glBegin(GL_QUADS);
            glNormal3f( 0.0f, 0.0f, 1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-13.3f, -10.0f,  10.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 13.3f, -10.0f,  10.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 13.3f,  10.0f,  10.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-13.3f,  10.0f,  10.0f);
        glEnd();
    glPopMatrix();
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
        case Qt::Key_L:
        {
            m_light = !m_light;
            if(!m_light)
            {
                glDisable(GL_LIGHTING);		// 禁用光源
            }
            else
            {
                glEnable(GL_LIGHTING);		// 启用光源
            }
            break;
        }
        case Qt::Key_F:
        {
            m_filter+=1;
            if(m_filter > 2)
            {
                m_filter = 0;
            }
            qDebug() << "m_filter = " << m_filter;
            break;
        }
        case Qt::Key_PageUp:
        {
            m_z-=0.2f;
            break;
        }
        case Qt::Key_PageDown:
        {
            m_z+=0.2f;
            break;
        }
        case Qt::Key_Up:
        {
            m_x_speed-=0.01f;
            break;
        }
        case Qt::Key_Down:
        {
            m_y_speed+=0.01f;
            break;
        }
        case Qt::Key_Right:
        {
            m_y_speed+=0.01f;
            break;
        }
        case Qt::Key_Left:
        {
            m_y_speed-=0.01f;
            break;
        }
        case Qt::Key_Escape:
        {
            qApp->exit();
            break;
        }
        case Qt::Key_Space:
        {
            m_object++;
            if(m_object > 5)
            {
                m_object = 0;
            }
            updateGL();
            break;
        }
    }
    QGLWidget::keyPressEvent(event);
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    //现在增加 xrot , yrot 和 zrot 的值。尝试变化每次各变量的改变值来调节立方体的旋转速度，或改变+/-号来调节立方体的旋转方向。
    m_x_rotate+=m_x_speed;								// X 轴旋转
    m_y_rotate+=m_y_speed;								// Y 轴旋转
    if(m_object == 5)
    {
        m_part1+=m_p1;
        m_part2+=m_p2;
        if(m_part1>359)
        {
            m_p1=0;
            m_part1=0;
            m_p2=1;
            m_part2=0;

        }
        if(m_part2>359)
        {
            m_p1=1;
            m_p2=0;
        }
    }
    updateGL();
    QGLWidget::timerEvent(event);
}

void MyGLWidget::loadGLTexture()
{
    QVector<QImage> images;
    images.push_back(QImage(":/image/BG.bmp")); // 背景图
    images.push_back(QImage(":/image/Reflect.bmp")); // 反射图（球形纹理图）
    for(int i = 0, iend = images.count(); i < iend; i++)
    {
        images[i] = images[i].convertToFormat(QImage::Format_RGB888);
        images[i] = images[i].mirrored();
    }
    glGenTextures(6, &m_texture[0]);
    for(int i = 0, iend = 1; i <= iend; i++)
    {
        glBindTexture(GL_TEXTURE_2D, m_texture[i]); // 创建纹理0和1
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, images[i].width(),
                     images[i].height(), 0, GL_RGB, GL_UNSIGNED_BYTE, images[i].bits());

        glBindTexture(GL_TEXTURE_2D, m_texture[i+2]); // 创建纹理2，3
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, images[i].width(),
                     images[i].height(), 0, GL_RGB, GL_UNSIGNED_BYTE, images[i].bits());

        glBindTexture(GL_TEXTURE_2D, m_texture[i+4]); // 创建纹理4，5
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, images[i].width(), images[i].height(),
                          GL_RGB, GL_UNSIGNED_BYTE, images[i].bits());
    }
}

//我们对立方体的绘制代码做了一些小的改动，把法线的范围从[-1,1]缩放到[-0.5,0.5]。
//如果法向量太大的话，会产生一些块状效果，影响视觉效果。
void MyGLWidget::glDrawCube()
{
    glBegin(GL_QUADS);
        // 前面
        glNormal3f( 0.0f, 0.0f, 0.5f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
        // 后面
        glNormal3f( 0.0f, 0.0f,-0.5f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
        // 上面
        glNormal3f( 0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
        // 下面
        glNormal3f( 0.0f,-1.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
        // 右面
        glNormal3f( 1.0f, 0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
        // 左面
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
    glEnd();
}

//我们成功了！现在你可以使用环境映射纹理做一些非常棒的特效了。我想做一个立方体环境映射的例子，
//但我现在的显卡不支持这种特效，所以只有等到以后了。
