#include "myglwidget.h"

//飘动的旗帜:
//这一课从第六课的代码开始，创建一个飘动的旗帜。我相信在这课结束的时候，你可以掌握纹理映射和混合操作。
//大家好！对那些想知道我在这里作了些什么的朋友，您可以先按文章的末尾所列出的链接，下载我那毫无意义的演示（Demo）看看先！
//我是bosco，我将尽我所能教您来实现一个以正弦波方式运动的图象。这一课基于NeHe的教程第六课，当然您至少也应该学会了一至六课的知识。
//您需要下载源码压缩包，并将压缩包内带的data目录连其下的位图一起释放至您的代码目录下。或者使用您自己的位图，
//当然它的尺寸必须适合OpenGL纹理的要求。

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_x_rotate(0.0f), m_y_rotate(0.0f),
    m_z_rotate(0.0f)
{
    showNormal();
    startTimer(100);
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(1, &m_texture[0]);
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

//我只在 InitGL 中增加很少的几行代码。但为了方便您查看增加了哪几行，我这段代码全部重贴一遍。
//loadGLTexture()这行代码调用载入位图并生成纹理。纹理创建好了，我们启用2D纹理映射。
//如果您忘记启用的话，您的对象看起来永远都是纯白色，这一定不是什么好事。
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

    glPolygonMode( GL_BACK, GL_FILL );			// 后表面完全填充
    glPolygonMode( GL_FRONT, GL_LINE );			// 前表面使用线条绘制
    //上面的代码指定使用完全填充模式来填充多边形区域的背面（译者：或者叫做后表面吧）。
    //相反，多边形的正面（译者：前表面）则使用轮廓线填充了。这些方式完全取决于您的个人喜好。并且与多边形的方位或者顶点的方向有关。
    //详情请参考红宝书（Red Book）。这里我顺便推销一本推动我学习OpenGL的好书
    //— Addison-Wesley出版的《Programmer's Guide to OpenGL》。个人以为这是学习OpenGL的无价之宝。

    // X平面循环
    for(int x=0; x<45; x++)
    {
        // Y平面循环
        for(int y=0; y<45; y++)
        {
            // 向表面添加波浪效果
            m_points[x][y][0]=float((x/5.0f)-4.5f);
            m_points[x][y][1]=float((y/5.0f)-4.5f);
            m_points[x][y][2]=float(sin((((x/5.0f)*40.0f)/360.0f)*3.141592654*2.0f));
        }
    }
    //这里感谢Graham Gibbons关于使用整数循环变量消除波浪间的脉冲锯齿的建议。
    //上面的两个循环初始化网格上的点。使用整数循环可以消除由于浮点运算取整造成的脉冲锯齿的出现。我们将x和y变量都除以5，再减去4.5。
    //这样使得我们的波浪可以“居中”（译者：这样计算所得结果将落在区间[-4.5，4.5]之间）。
    //点[x][y][2]最后的值就是一个sine函数计算的结果。Sin()函数需要一个弧度参变量。将float_x乘以40.0f，得到角度值。
    //然后除以360.0f再乘以PI，乘以2，就转换为弧度了。
}

//现在我们绘制贴图『译者注：其实贴图就是纹理映射。将术语换来换去不好，我想少打俩字。^_^』过的立方体。
//这段代码被狂注释了一把，应该很好懂。
//开始两行代码 glClear() 和 glLoadIdentity() 是第一课中就有的代码。
//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) 清除屏幕并设为我们在 InitGL() 中选定的颜色，本例中是黑色。
//深度缓存也被清除。模型观察矩阵也使用glLoadIdentity()重置。
void MyGLWidget::paintGL()
{
    float float_x, float_y, float_xb, float_yb;		// 用来将旗形的波浪分割成很小的四边形
    //我们使用不同的变量来控制循环。下面的代码中大多数变量除了用来控制循环和存储临时变量之外并没有什么别的用处。
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);			// 清除屏幕和深度缓存
    glLoadIdentity();							// 重置当前的模型观察矩阵

    glTranslatef(0.0f,0.0f,-12.0f);				// 移入屏幕12个单位
    glRotatef(m_x_rotate,1.0f,0.0f,0.0f);				// 绕 X 轴旋转
    glRotatef(m_y_rotate,0.0f,1.0f,0.0f);				// 绕 Y 轴旋转
    glRotatef(m_z_rotate,0.0f,0.0f,1.0f);				// 绕 Z 轴旋转
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);		// 选择纹理
    glBegin(GL_QUADS);					// 四边形绘制开始
        for(int x = 0; x < 44; x++ )				// 沿 X 平面 0-44 循环(45点)
        {
            for(int y = 0; y < 44; y++ )			// 沿 Y 平面 0-44 循环(45点)
            {
                //接着开始使用循环进行多边形绘制。这里使用整型可以避免我以前所用的int()强制类型转换。
                float_x = float(x)/44.0f;		// 生成X浮点值
                float_y = float(y)/44.0f;		// 生成Y浮点值
                float_xb = float(x+1)/44.0f;		// X浮点值+0.0227f
                float_yb = float(y+1)/44.0f;		// Y浮点值+0.0227f
                //上面我们使用4个变量来存放纹理坐标。每个多边形（网格之间的四边形）分别映射了纹理的1/44×1/44部分。
                //循环首先确定左下顶点的值，然后我们据此得到其他三点的值。
                glTexCoord2f( float_x, float_y);	// 第一个纹理坐标 (左下角)
                glVertex3f( m_points[x][y][0], m_points[x][y][1], m_points[x][y][2] );
                glTexCoord2f( float_x, float_yb );	// 第二个纹理坐标 (左上角)
                glVertex3f( m_points[x][y+1][0], m_points[x][y+1][1], m_points[x][y+1][2] );
                glTexCoord2f( float_xb, float_yb );	// 第三个纹理坐标 (右上角)
                glVertex3f( m_points[x+1][y+1][0], m_points[x+1][y+1][1], m_points[x+1][y+1][2] );
                glTexCoord2f( float_xb, float_y );	// 第四个纹理坐标 (右下角)
                glVertex3f( m_points[x+1][y][0], m_points[x+1][y][1], m_points[x+1][y][2] );
            }
        }
    glEnd();						// 四边形绘制结束
    //上面几行使用glTexCoord2f()和glVertex3f()载入数据。提醒一点：四边形是逆时针绘制的。
    //这就是说，您开始所见到的表面是背面。后表面完全填充了，前表面由线条组成。
    //如果您按顺时针顺序绘制的话，您初始时见到的可能是前表面。也就是说您将看到网格型的纹理效果而不是完全填充的。
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
    for(int y = 0; y < 45; y++ )			// Y平面循环
    {
        GLfloat hold = m_points[0][y][2];	// 存储当前左侧波浪值
        for(int x = 0; x < 44; x++)		// 沿X平面循环
        {
            // 当前波浪值等于其右侧的波浪值
            m_points[x][y][2] = m_points[x+1][y][2];
        }
        m_points[44][y][2]=hold;			// 刚才的值成为最左侧的波浪值
    }
    //上面所作的事情是先存储每一行的第一个值，然后将波浪左移一下，是图象产生波浪。
    //存储的数值挪到末端以产生一个永无尽头的波浪纹理效果。然后重置计数器wiggle_count以保持动画的进行。
    //上面的代码由NeHe（2000年2月）修改过，以消除波浪间出现的细小锯齿。
    //现在增加 xrot , yrot 和 zrot 的值。
    m_x_rotate+=0.3f;								// X 轴旋转
    m_y_rotate+=0.2f;								// Y 轴旋转
    m_z_rotate+=0.4f;								// Z 轴旋转
    updateGL();
    QGLWidget::timerEvent(event);
}

void MyGLWidget::loadGLTexture()
{
    //现在载入图像，并将其转换为纹理。
    QImage image(":/image/Tim.bmp");
    image = image.convertToFormat(QImage::Format_RGB888);
    image = image.mirrored();
    glGenTextures(1, &m_texture[0]);// 创建纹理
    // 使用来自位图数据生成 的典型纹理
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);
    //下来我们创建真正的纹理。下面一行告诉OpenGL此纹理是一个2D纹理 ( GL_TEXTURE_2D )。
    //参数“0”代表图像的详细程度，通常就由它为零去了。参数三是数据的成分数。
    //因为图像是由红色数据，绿色数据，蓝色数据三种组分组成。 TextureImage[0]->sizeX 是纹理的宽度。
    //如果您知道宽度，您可以在这里填入，但计算机可以很容易的为您指出此值。
    //TextureImage[0]->sizey 是纹理的高度。参数零是边框的值，一般就是“0”。
    //GL_RGB 告诉OpenGL图像数据由红、绿、蓝三色数据组成。
    //GL_UNSIGNED_BYTE 意味着组成图像的数据是无符号字节类型的。
    //最后... TextureImage[0]->data 告诉OpenGL纹理数据的来源。此例中指向存放在 TextureImage[0] 记录中的数据。
    glTexImage2D(GL_TEXTURE_2D, 0, 3,
                 image.width(), image.height(), 0, GL_RGB, GL_UNSIGNED_BYTE,
                 image.bits());
    //下面的两行告诉OpenGL在显示图像时，
    //当它比放大得原始的纹理大 ( GL_TEXTURE_MAG_FILTER )或缩小得比原始得纹理小( GL_TEXTURE_MIN_FILTER )时
    //OpenGL采用的滤波方式。通常这两种情况下我都采用 GL_LINEAR 。
    //这使得纹理从很远处到离屏幕很近时都平滑显示。使用 GL_LINEAR 需要CPU和显卡做更多的运算。
    //如果您的机器很慢，您也许应该采用 GL_NEAREST 。过滤的纹理在放大的时候，看起来斑驳的很『译者注：马赛克啦』。
    //您也可以结合这两种滤波方式。在近处时使用 GL_LINEAR ，远处时 GL_NEAREST 。
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	// 线形滤波
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	// 线形滤波
}

//标准的NeHe旋转增量。现在编译并运行程序，您将看到一个漂亮的位图波浪。除了嘘声一片之外，我不敢确信大家的反应。
//但我希望大家能从这一课中学到点什么。如果您有任何问题或者需要澄清的地方，请随便联络我。感谢大家。
