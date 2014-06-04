#include "myglwidget.h"

//蒙板:
//到目前为止你已经学会如何使用alpha混合，把一个透明物体渲染到屏幕上了，但有的使用它看起来并不是那么的复合你的心意。
//使用蒙板技术，将会按照你蒙板的位置精确的绘制。

//欢迎来到第20课的教程，*.bmp图像被给各种操作系统所支持，因为它简单，所以可以很轻松的作为纹理图片加载它。
//知道现在，我们在把图像加载到屏幕上时没有擦除背景色，因为这样简单高效。但是效果并不总是很好。
//大部分情况下，把纹理混合到屏幕，纹理不是太少就是太多。当使用精灵时，我不希望背景从精灵的缝隙中透出光来；但在显示文字时，
//你希望文字的间隙可以显示背景色。
//由于以上原因，我们需要使用“掩模”。使用“掩模”需要两个步骤，首先我们在场景上放置黑白相间的纹理，白色代表透明部分，黑色代表不透明部分。
//接着我们使用一种特殊的混合方式，只有在黑色部分上的纹理才会显示在场景中。
//我只重写那些改变的地方，如果你做好了学习的准备，我们就上路吧。

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_masking(true), m_scene(false), m_roll(0.0f)
{
    showNormal();
    startTimer(50);
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(5, &m_texture[0]);
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

//现在到了最有趣的绘制部分了，我们从清楚背景色开始，接着把物体移入屏幕2个单位。
void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f,0.0f,-2.0f);						// 物体移入屏幕2个单位
    //下面一行，我们选择'logo'纹理。我们将要通过四边形把纹理映射到屏幕，并按照顶点的顺序设置纹理坐标。
    //Jonathan Roy说OpenGL是一个基于顶点的图形系统，大部分你设置的参数是作为顶点的属性而记录的，纹理坐标就是这样一种属性。
    //你只要简单的设置各个顶点的纹理坐标，OpenGL就自动帮你把多边形内部填充纹理，通过一个插值的过程。
    //向前面几课一样，我们假定四边形面对我们，并把纹理坐标(0,0)绑定到左下角，(1,0)绑定到右下角，(1,1)绑定到右上角。
    //给定这些设置，你应该能猜到四边形中间对应的纹理坐标为(0.5,0.5)，但你自己并没有设置此处的纹理坐标！OpenGL为你做了计算。
    //在这一课里，我们通过设置纹理坐标达到一种滚动纹理的目的。纹理坐标是被归一化的，它的范围从0.0-1.0，值0被映射到纹理的一边，
    //值1被映射到纹理的另一边。超过1的值，纹理可以按照不同的方式被映射，这里我们设置为它将回绕道另一边并重复纹理。
    //例如如果使用这样的映射方式，纹理坐标(0.3,0.5)和(1.3,0.5)被映射到同一个纹理坐标。在这一课里，我们将尝试一种无缝填充的效果。

    //我们使用roll变量去设置纹理坐标，当它为0时，它把纹理的左下角映射到四边形的左下角。
    //当它大于0时，把纹理的左上角映射到四边形的左下角，看起来的效果就是纹理沿四边形向上滚动。
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);				// 选择Logo纹理
    glBegin(GL_QUADS);							// 绘制纹理四边形
        glTexCoord2f(0.0f, -m_roll+0.0f); glVertex3f(-1.1f, -1.1f,  0.0f);
        glTexCoord2f(3.0f, -m_roll+0.0f); glVertex3f( 1.1f, -1.1f,  0.0f);
        glTexCoord2f(3.0f, -m_roll+3.0f); glVertex3f( 1.1f,  1.1f,  0.0f);
        glTexCoord2f(0.0f, -m_roll+3.0f); glVertex3f(-1.1f,  1.1f,  0.0f);
    glEnd();

    //启用混合和禁用深度测试
    glEnable(GL_BLEND);							// 启用混合
    glDisable(GL_DEPTH_TEST);							// 禁用深度测试
    //接下来我们需要根据masking的值设置是否使用“掩模”，如果是，则需要设置相应的混合系数。
    if(m_masking)								// 是否启用“掩模”
    {
        //如果启用了“掩模”，我们将要设置“掩模”的混合系数。一个“掩模”只是一幅绘制到屏幕的纹理图片，但只有黑色和白色。
        //白色的部分代表透明，黑色的部分代表不透明。
        //下面这个混合系数使得，任何对应“掩模”黑色的部分会变为黑色，白色的部分会保持原来的颜色。
        glBlendFunc(GL_DST_COLOR,GL_ZERO);					// 使用黑白“掩模”混合屏幕颜色
    }
    //现在我们检查绘制那一个层，如果为True绘制第二个层，否则绘制第一个层
    if(m_scene)
    {
        //为了不使它看起来显得非常大，我们把它移入屏幕一个单位，并把它按roll变量的值进行旋转（沿Z轴）。
        glTranslatef(0.0f,0.0f,-1.0f);					// 移入屏幕一个单位
        glRotatef(m_roll*360,0.0f,0.0f,1.0f);			// 沿Z轴旋转
        //接下我们检查masking的值来绘制我们的对象
        if(m_masking)							// “掩模”是否打开
        {
            //如果“掩模打开”，我们会把掩模绘制到屏幕。当我们完成这个操作时，将会看到一个镂空的纹理出现在屏幕上。
            glBindTexture(GL_TEXTURE_2D, m_texture[3]);		// 选择第二个“掩模”纹理
            glBegin(GL_QUADS);                              // 开始绘制四边形
                glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.1f, -1.1f,  0.0f);
                glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.1f, -1.1f,  0.0f);
                glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.1f,  1.1f,  0.0f);
                glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.1f,  1.1f,  0.0f);
            glEnd();
        }
        //当我们把“掩模”绘制到屏幕上后，接着我们变换混合系数。这次我们告诉OpenGL把任何黑色部分对应的像素复制到屏幕，
        //这样看起来纹理就像被镂空一样帖子屏幕上。
        //注意，我们在变换了混合模式后在选择的纹理。
        //如果我们没有使用“掩模”，我们的图像将与屏幕颜色混合。
        glBlendFunc(GL_ONE, GL_ONE);				// 把纹理2复制到屏幕
        glBindTexture(GL_TEXTURE_2D, m_texture[4]);			// 选择第二个纹理
        glBegin(GL_QUADS);						// 绘制四边形
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.1f, -1.1f,  0.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.1f, -1.1f,  0.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.1f,  1.1f,  0.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.1f,  1.1f,  0.0f);
        glEnd();
    }
    else
    {
        if(m_masking)							// “掩模”是否打开
        {
            //如果“掩模打开”，我们会把掩模绘制到屏幕。当我们完成这个操作时，将会看到一个镂空的纹理出现在屏幕上。
            glBindTexture(GL_TEXTURE_2D, m_texture[1]);		// 选择第一个“掩模”纹理
            glBegin(GL_QUADS);					// 开始绘制四边形
                glTexCoord2f(m_roll+0.0f, 0.0f); glVertex3f(-1.1f, -1.1f,  0.0f);
                glTexCoord2f(m_roll+4.0f, 0.0f); glVertex3f( 1.1f, -1.1f,  0.0f);
                glTexCoord2f(m_roll+4.0f, 4.0f); glVertex3f( 1.1f,  1.1f,  0.0f);
                glTexCoord2f(m_roll+0.0f, 4.0f); glVertex3f(-1.1f,  1.1f,  0.0f);
            glEnd();
        }
        //当我们把“掩模”绘制到屏幕上后，接着我们变换混合系数。这次我们告诉OpenGL把任何黑色部分对应的像素复制到屏幕，
        //这样看起来纹理就像被镂空一样帖子屏幕上。
        //注意，我们在变换了混合模式后在选择的纹理。
        //如果我们没有使用“掩模”，我们的图像将与屏幕颜色混合。
        glBlendFunc(GL_ONE, GL_ONE);					// 把纹理1复制到屏幕
        glBindTexture(GL_TEXTURE_2D, m_texture[2]);				// 选择第一个纹理
        glBegin(GL_QUADS);							// 开始绘制四边形
            glTexCoord2f(m_roll+0.0f, 0.0f); glVertex3f(-1.1f, -1.1f,  0.0f);
            glTexCoord2f(m_roll+4.0f, 0.0f); glVertex3f( 1.1f, -1.1f,  0.0f);
            glTexCoord2f(m_roll+4.0f, 4.0f); glVertex3f( 1.1f,  1.1f,  0.0f);
            glTexCoord2f(m_roll+0.0f, 4.0f); glVertex3f(-1.1f,  1.1f,  0.0f);
        glEnd();
    }
    //接下来启用深度测试，禁用混合。
    glEnable(GL_DEPTH_TEST);							// 启用深度测试
    glDisable(GL_BLEND);							// 禁用混合

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
        case Qt::Key_Space:
        {
            m_scene = !m_scene;
            break;
        }
        case Qt::Key_M:
        {
            m_masking = !m_masking;
            break;
        }
    }
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    //最后增加roll变量，如果大于1，把它的值减1。
    m_roll+=0.02f;								// 增加纹理滚动变量
    if(m_roll>1.0f)								// 大于1则减1
    {
        m_roll-=1.0f;
    }
    updateGL();
    QGLWidget::timerEvent(event);
}

void MyGLWidget::loadGLTexture()
{
    QVector<QImage> images;
    images.push_back(QImage(":/image/Logo.bmp"));
    images.push_back(QImage(":/image/Mask1.bmp"));
    images.push_back(QImage(":/image/Image1.bmp"));
    images.push_back(QImage(":/image/Mask2.bmp"));
    images.push_back(QImage(":/image/Image2.bmp"));
    for(int i = 0, iend = 5; i < iend; i++)
    {
        images[i] = images[i].convertToFormat(QImage::Format_RGB888);
        images[i] = images[i].mirrored();
    }

    glGenTextures(5, &m_texture[0]);// 创建纹理
    for(int i = 0 ,iend = 5; i < iend; i++)
    {
        glBindTexture(GL_TEXTURE_2D, m_texture[i]);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, images[i].width(), images[i].height(),
                     0, GL_RGB, GL_UNSIGNED_BYTE, images[i].bits());
    }
}

//Eric Desrosiers指出，你也可以在载入的时候测试*.bmp图像中的每一个像素，如果你你想要透明的结果，你可以把颜色的alpha设置为0。
//对于其他的颜色，你可以把alpha设置为1。这个方法也能达到同样的效果，但需要一些额外的代码。
//在这课里，我们给你演示了一个简单的例子，它能高效的绘制一部分纹理而不使用alpha值。
//谢谢Rob Santa的想法和例子程序，我从没想到过这种方法。
//我希望你喜欢这个教程，如果你在理解上有任何问题或找到了任何错误，请我知道，我想做最好的教程，你的反馈是非常重要的。
