#include "myglwidget.h"

//显示列表:
//想知道如何加速你的OpenGL程序么？这一课将告诉你如何使用OpenGL的显示列表，它通过预编译OpenGL命令来加速你的程序，
//并可以为你省去很多重复的代码。

//这次我将教你如何使用显示列表，显示列表将加快程序的速度，而且可以减少代码的长度。
//当你在制作游戏里的小行星场景时，每一层上至少需要两个行星，你可以用OpenGL中的多边形来构造每一个行星。
//聪明点的做法是做一个循环，每个循环画出行星的一个面，最终你用几十条语句画出了一个行星。
//每次把行星画到屏幕上都是很困难的。当你面临更复杂的物体时你就会明白了。
//那么，解决的办法是什么呢？用现实列表，你只需要一次性建立物体，你可以贴图，用颜色，想怎么弄就怎么弄。
//给现实列表一个名字，比如给小行星的显示列表命名为“asteroid”。现在，任何时候我想在屏幕上画出行星，
//我只需要调用glCallList(asteroid)。之前做好的小行星就会立刻显示在屏幕上了。
//因为小行星已经在显示列表里建造好了，OpenGL不会再计算如何构造它。它已经在内存中建造好了。这将大大降低CPU的使用，让你的程序跑的更快。
//那么，开始学习咯。我称这个DEMO为Q-Bert显示列表。最终这个DEMO将在屏幕上画出15个立方体。
//每个立方体都由一个盒子和一个顶部构成，顶部是一个单独的显示列表，盒子没有顶。
//这一课是建立在第六课的基础上的，我将重写大部分的代码，这样容易看懂。下面的这些代码在所有的课程中差不多都用到了。

//接下来建立两个颜色数组
static GLfloat boxcol[5][3]=				// 盒子的颜色数组

{
    // 亮:红，橙，黄，绿，蓝
    {1.0f,0.0f,0.0f},{1.0f,0.5f,0.0f},{1.0f,1.0f,0.0f},{0.0f,1.0f,0.0f},{0.0f,1.0f,1.0f}

};

static GLfloat topcol[5][3]=				// 顶部的颜色数组
{
    // 暗:红，橙，黄，绿，蓝
    {0.5f,0.0f,0.0f},{0.5f,0.25f,0.0f},{0.5f,0.5f,0.0f},{0.0f,0.5f,0.0f},{0.0f,0.5f,0.5f}

};

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false),m_yrot(0.0f),m_xrot(0.0f)
{
    showNormal();
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(1, &m_texture[0]);
    glDeleteLists(m_box, 2);
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

//初始化的代码只有一点改变，加入了一行BuildList()。请注意代码的顺序，先读入纹理，然后建立显示列表，
//这样当我们建立显示列表的时候就可以将纹理贴到立方体上了。
void MyGLWidget::initializeGL()
{
    loadGLTexture();

    buildLists();

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

    //接下来的三行使灯光有效。Light0一般来说是在显卡中预先定义过的，如果Light0不工作，把下面那行注释掉好了。
    //最后一行的GL_COLOR_MATERIAL使我们可以用颜色来贴纹理。如果没有这行代码，纹理将始终保持原来的颜色，glColor3f(r,g,b)就没有用了。
    //总之这行代码是很有用的。
    glEnable(GL_LIGHT0);					// 使用默认的0号灯
    glEnable(GL_LIGHTING);					// 使用灯光
    glEnable(GL_COLOR_MATERIAL);				// 使用颜色材质
}

//现在到了绘制代码的地方了，我们还是和以前一样，以清除背景颜色为开始。
//接着把纹理绑定到立方体，我可以把这些代码加入到显示列表中，但我还是把它留在了显示列表外边，这样我可以随便设置纹理。
void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//清除屏幕和深度缓存
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);		// 选择纹理
    //现在到了真正有趣的地方了。用一个循环，循环变量用于改变Y轴位置，在Y轴上画5个立方体，所以用从1到5的循环。
    for (int yloop=1;yloop<6;yloop++)				// 沿Y轴循环
    {
        //另外用一个循环，循环变量用于改变X轴位置。每行上的立方体数目取决于行数，所以循环方式如下。
        for (int xloop=0;xloop<yloop;xloop++)		// 沿X轴循环
        {
            //重置模型变化矩阵
            glLoadIdentity();			// 重置模型变化矩阵
            //下边的代码是移动和旋转当前坐标系到需要画出立方体的位置。
            //（原文有很罗嗦的一大段，相信大家的数学功底都不错，就不翻译了）
            // 设置盒子的位置
            glTranslatef(1.4f+(float(xloop)*2.8f)-(float(yloop)*1.4f),((6.0f-float(yloop))*2.4f)-7.0f,-20.0f);
            glRotatef(45.0f-(2.0f*yloop)+m_xrot,1.0f,0.0f,0.0f);
            glRotatef(45.0f+m_yrot,0.0f,1.0f,0.0f);
            //然后在正式画盒子之前设置颜色。每个盒子用不同的颜色。
            glColor3fv(boxcol[yloop-1]);
            //好了，颜色设置好了。现在需要做的就是画出盒子。不用写出画多边形的代码，只需要用glCallList(box)命令调用显示列表。
            //盒子将会用glColor3fv()所设置的颜色画出来。
            glCallList(m_box);			// 绘制盒子
            //然后用另外的颜色画顶部。搞定。
            glColor3fv(topcol[yloop-1]);		// 选择顶部颜色
            glCallList(m_top);			// 绘制顶部
        }
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
        case Qt::Key_Up:
        {
            m_xrot-=0.2f;
            updateGL();
            break;
        }
        case Qt::Key_Down:
        {
            m_xrot+=0.2f;
            updateGL();
            break;
        }
        case Qt::Key_Left:
        {
            m_yrot-=0.2f;
            updateGL();
            break;
        }
        case Qt::Key_Right:
        {
            m_yrot+=0.2f;
            updateGL();
            break;
        }
    }
}

//现在正式开始建立显示列表。你可能注意到了，所有创造盒子的代码都在第一个显示列表里，所有创造顶部的代码都在另一个列表里。
//我会努力解释这些细节。
void MyGLWidget::buildLists()					// 创建盒子的显示列表
{
    //开始的时候我们告诉OpenGL我们要建立两个显示列表。
    //glGenLists(2)建立了两个显示列表的空间，并返回第一个显示列表的指针。
    //“box”指向第一个显示列表，任何时候调用“box”第一个显示列表就会显示出来。
    m_box = glGenLists(2);				// 创建两个显示列表的名称

    //现在开始构造第一个显示列表。我们已经申请了两个显示列表的空间了，并且有box指针指向第一个显示列表。
    //所以现在我们应该告诉OpenGL要建立什么类型的显示列表。
    //我们用glNewList()命令来做这个事情。你一定注意到了box是第一个参数，这表示OpenGL将把列表存储到box所指向的内存空间。
    //第二个参数GL_COMPILE告诉OpenGL我们想预先在内存中构造这个列表，这样每次画的时候就不必重新计算怎么构造物体了。
    //GL_COMPILE类似于编程。在你写程序的时候，把它装载到编译器里，你每次运行程序都需要重新编译。
    //而如果他已经编译成了.exe文件，那么每次你只需要点击那个.exe文件就可以运行它了，不需要编译。
    //当OpenGL编译过显示列表后，就不需要再每次显示的时候重新编译它了。这就是为什么用显示列表可以加快速度。
    glNewList(m_box, GL_COMPILE);			// 创建第一个显示列表

    //下面这部分的代码画出一个没有顶部的盒子，它不会出现在屏幕上，只会存储在显示列表里。
    //你可以在glNewList()和glEngList()中间加上任何你想加上的代码。可以设置颜色，贴图等等。
    //唯一不能加进去的代码就是会改变显示列表的代码。显示列表一旦建立，你就不能改变它。
    //比如你想加上glColor3ub(rand()%255,rand()%255,rand()%255)，使得每一次画物体时都会有不同的颜色。
    //但因为显示列表只会建立一次，所以每次画物体的时候颜色都不会改变。物体将会保持第一次建立显示列表时的颜色。
    //如果你想改变显示列表的颜色，你只有在调用显示列表之前改变颜色。后面将详细解释这一点。
        glBegin(GL_QUADS);							// 开始绘制四边形
            // 底面
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
            // 前面
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
            // 后面
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
            // 右面
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
            // 左面
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
        glEnd();								// 四边形绘制结束
    //用glEngList()命令，我们告诉OpenGL我们已经完成了一个显示列表。在glNewList()和glEngList()之间的任何东西就是显示列表的一部分。
    glEndList();									// 第一个显示列表结束
    //现在我们来建立第二个显示列表。在上一个显示列表的指针上加1，就得到了第二个显示列表的指针。第二个显示列表的指针命名为“top”。
    m_top = m_box+1;									// 第二个显示列表的名称
    //现在我们知道了第二个显示列表的指针，我们可以建立它了。
    glNewList(m_top,GL_COMPILE);							// 盒子顶部的显示列表
        //下面的代码画出盒子的顶部。
        glBegin(GL_QUADS);							// 开始绘制四边形
            // 上面
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
        glEnd();								// 结束绘制四边形
    //然后告诉OpenGL第二个显示列表建立完毕。
    glEndList();									// 第二个显示列表创建完毕
}

//贴图纹理的代码和之前教程里的代码是一样的。我们需要一个可以贴在立方体上的纹理。
//我决定使用mipmapping处理让纹理看上去光滑，因为我讨厌看见像素点。纹理的文件名是“cube.bmp”。
void MyGLWidget::loadGLTexture()
{
    QImage image(":/image/Cube.bmp");
    image = image.convertToFormat(QImage::Format_RGB888);
    image = image.mirrored();
    glGenTextures(1, &m_texture[0]);// 创建纹理
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 image.width(), image.height(), 0, GL_RGB, GL_UNSIGNED_BYTE,
                 image.bits());
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
}
