#include "myglwidget.h"

//纹理映射:
//在这一课里，我将教会你如何把纹理映射到立方体的六个面。

//学习 texture map 纹理映射(贴图)有很多好处。比方说您想让一颗导弹飞过屏幕。
//根据前几课的知识，我们最可行的办法可能是很多个多边形来构建导弹的轮廓并加上有趣的颜色。
//使用纹理映射，您可以使用真实的导弹图像并让它飞过屏幕。您觉得哪个更好看？照片还是一大堆三角形和四边形？
//使用纹理映射的好处还不止是更好看，而且您的程序运行会更快。导弹贴图可能只是一个飞过窗口的四边形。
//一个由多边形构建而来的导弹却很可能包括成百上千的多边形。很显然，贴图极大的节省了CPU时间。
//我们增加了三个新的浮点变量... xrot , yrot 和 zrot 。这些变量用来使立方体绕X、Y、Z轴旋转。
//GLuint texture[1] 为一个纹理分配存储空间。如果您需要不止一个的纹理，应该将参数1改成您所需要的参数。

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_x_rotate(0.0f), m_y_rotate(0.0f),
    m_z_rotate(0.0f)
{
    showNormal();
    startTimer(50);
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
}

//现在我们绘制贴图『译者注：其实贴图就是纹理映射。将术语换来换去不好，我想少打俩字。^_^』过的立方体。
//这段代码被狂注释了一把，应该很好懂。
//开始两行代码 glClear() 和 glLoadIdentity() 是第一课中就有的代码。
//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) 清除屏幕并设为我们在 InitGL() 中选定的颜色，本例中是黑色。
//深度缓存也被清除。模型观察矩阵也使用glLoadIdentity()重置。
void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);			// 清除屏幕和深度缓存
    glLoadIdentity();							// 重置当前的模型观察矩阵
    glTranslatef(0.0f,0.0f,-5.0f);						// 移入屏幕 5 个单位
    //下面三行使立方体绕X、Y、Z轴旋转。旋转多少依赖于变量 xrot ， yrot 和 zrot 的值。
    glRotatef(m_x_rotate,1.0f,0.0f,0.0f);						// X轴旋转
    glRotatef(m_y_rotate,0.0f,1.0f,0.0f);						// Y轴旋转
    glRotatef(m_z_rotate,0.0f,0.0f,1.0f);						// Z轴旋转

    //下一行代码选择我们使用的纹理。
    //如果您在您的场景中使用多个纹理，您应该使用来 glBindTexture(GL_TEXTURE_2D, texture[ 所使用纹理对应的数字 ]) 选择要绑定的
    //纹理。当您想改变纹理时，应该绑定新的纹理。有一点值得指出的是，您不能在 glBegin() 和 glEnd() 之间绑定纹理，必须在 glBegin()
    //之前或 glEnd() 之后绑定。注意我们在后面是如何使用 glBindTexture 来指定和绑定纹理的。
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);				// 选择纹理
    //为了将纹理正确的映射到四边形上，您必须将纹理的右上角映射到四边形的右上角，纹理的左上角映射到四边形的左上角，
    //纹理的右下角映射到四边形的右下角，纹理的左下角映射到四边形的左下角。
    //如果映射错误的话，图像显示时可能上下颠倒，侧向一边或者什么都不是。
    //glTexCoord2f 的第一个参数是X坐标。 0.0f 是纹理的左侧。 0.5f 是纹理的中点， 1.0f 是纹理的右侧。
    //glTexCoord2f 的第二个参数是Y坐标。 0.0f 是纹理的底部。 0.5f 是纹理的中点， 1.0f 是纹理的顶部。
    //所以纹理的左上坐标是 X：0.0f，Y：1.0f ，四边形的左上顶点是 X： -1.0f，Y：1.0f 。其余三点依此类推。
    //试着玩玩 glTexCoord2f X, Y坐标参数。把 1.0f 改为 0.5f 将只显示纹理的左半部分，把 0.0f 改为 0.5f 将只显示纹理的右半部分。
    glBegin(GL_QUADS);
            // 前面
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);	// 纹理和四边形的左下
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);	// 纹理和四边形的右下
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);	// 纹理和四边形的右上
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);	// 纹理和四边形的左上
            // 后面
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);	// 纹理和四边形的右下
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);	// 纹理和四边形的右上
            glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);	// 纹理和四边形的左上
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);	// 纹理和四边形的左下
            // 顶面
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);	// 纹理和四边形的左上
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);	// 纹理和四边形的左下
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);	// 纹理和四边形的右下
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);	// 纹理和四边形的右上
            // 底面
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);	// 纹理和四边形的右上
            glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);	// 纹理和四边形的左上
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);	// 纹理和四边形的左下
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);	// 纹理和四边形的右下

            // 右面
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);	// 纹理和四边形的右下
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);	// 纹理和四边形的右上
            glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);	// 纹理和四边形的左上
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);	// 纹理和四边形的左下

            // 左面
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);	// 纹理和四边形的左下
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);	// 纹理和四边形的右下
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);	// 纹理和四边形的右上
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);	// 纹理和四边形的左上
    glEnd();

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
    //现在增加 xrot , yrot 和 zrot 的值。尝试变化每次各变量的改变值来调节立方体的旋转速度，或改变+/-号来调节立方体的旋转方向。
    m_x_rotate+=0.3f;								// X 轴旋转
    m_y_rotate+=0.2f;								// Y 轴旋转
    m_z_rotate+=0.4f;								// Z 轴旋转
    updateGL();
    QGLWidget::timerEvent(event);
}

void MyGLWidget::loadGLTexture()
{
    //现在载入图像，并将其转换为纹理。
    QImage image(":/image/NeHe.bmp");
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
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

//现在您应该比较好的理解纹理映射(贴图)了。您应该掌握了给任意四边形表面贴上您所喜爱的图像的技术。
//一旦您对2D纹理映射的理解感到自信的时候，试试给立方体的六个面贴上不同的纹理。
//当您理解纹理坐标的概念后，纹理映射并不难理解。！如果您有什么意见或建议请给我EMAIL。
//如果您认为有什么不对或可以改进，请告诉我。
