#include "myglwidget.h"

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false)
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

//第一课中，我教您如何创建一个OpenGL窗口。这一课中，我将教您如何创建三角形和四边形。
//我们讲使用来创建GL_TRIANGLES一个三角形，GL_QUADS来创建一个四边形。
//在第一课代码的基础上，我们只需在DrawGLScene()过程中增加代码。下面我重写整个过程。
//如果您计划修改上节课的代码，只需用下面的代码覆盖原来的DrawGLScene()就可以了
void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// 清除屏幕及深度缓存
    glLoadIdentity();	// 重置当前的模型观察矩阵

    //当您调用glLoadIdentity()之后，您实际上将当前点移到了屏幕中心，X坐标轴从左至右，Y坐标轴从下至上，Z坐标轴从里至外。
    //OpenGL屏幕中心的坐标值是X和Y轴上的0.0f点。中心左面的坐标值是负值，右面是正值。
    //移向屏幕顶端是正值，移向屏幕底端是负值。移入屏幕深处是负值，移出屏幕则是正值。
    //glTranslatef(x, y, z)沿着 X, Y 和 Z 轴移动。根据前面的次序，下面的代码沿着X轴左移1.5个单位，Y轴不动(0.0f)，
    //最后移入屏幕6.0f个单位。注意在glTranslatef(x, y, z)中当您移动的时候，您并不是相对屏幕中心移动，
    //而是相对与当前所在的屏幕位置。
    glTranslatef(-1.5f,0.0f,-6.0f);	// 左移 1.5 单位，并移入屏幕 6.0
    //现在我们已经移到了屏幕的左半部分，并且将视图推入屏幕背后足够的距离以便我们可以看见全部的场景－创建三角形。
    //glBegin(GL_TRIANGLES)的意思是开始绘制三角形，glEnd() 告诉OpenGL三角形已经创建好了。
    //通常您会需要画3个顶点，可以使用GL_TRIANGLES。在绝大多数的显卡上，绘制三角形是相当快速的。
    //如果要画四个顶点，使用GL_QUADS的话会更方便。但据我所知，绝大多数的显卡都使用三角形来为对象着色。
    //最后，如果您想要画更多的顶点时，可以使用GL_POLYGON。
    //本节的简单示例中，我们只画一个三角形。如果要画第二个三角形的话，可以在这三点之后，再加三行代码(3点)。
    //所有六点代码都应包含在glBegin(GL_TRIANGLES) 和 glEnd()之间。
    //在他们之间再不会有多余的点出现，也就是说，(GL_TRIANGLES) 和 glEnd()之间的点都是以三点为一个集合的。
    //这同样适用于四边形。如果您知道实在绘制四边形的话，您必须在第一个四点之后，再加上四点为一个集合的点组。
    //另一方面，多边形可以由任意个顶点，(GL_POLYGON)不在乎glBegin(GL_TRIANGLES) 和 glEnd()之间有多少行代码。
    //glBegin之后的第一行设置了多边形的第一个顶点，glVertex 的第一个参数是X坐标，然后依次是Y坐标和Z坐标。
    //第一个点是上顶点，然后是左下顶点和右下顶点。glEnd()告诉OpenGL没有其他点了。这样将显示一个填充的三角形。
    glBegin(GL_TRIANGLES); // 绘制三角形
        glVertex3f(0.0f, 1.0f, 0.0f); // 上顶点
        glVertex3f(-1.0f,-1.0f, 0.0f); // 左下
        glVertex3f( 1.0f,-1.0f, 0.0f); // 右下
    glEnd(); // 三角形绘制结束

    //在屏幕的左半部分画完三角形后，我们要移到右半部分来画正方形。
    //为此要再次使用glTranslate。这次右移，所以X坐标值为正值。
    //因为前面左移了1.5个单位，这次要先向右移回屏幕中心(1.5个单位)，再向右移动1.5个单位。总共要向右移3.0个单位。
    glTranslatef(3.0f,0.0f,0.0f); // 右移3单位
    //现在使用GL_QUADS绘制正方形。与绘制三角形的代码相类似，画四边形也很简单。
    //唯一的区别是用GL_QUADS来替换了GL_TRIANGLES。并增加了一个点。
    //我们使用顺时针次序来画正方形－左上－右上－右下－左下。采用顺时针绘制的是对象的后表面。
    //这就是说我们所看见的是正方形的背面。逆时针画出来的正方形才是正面朝着我们的。现在这对您来说并不重要，但以后您必须知道。
    glBegin(GL_QUADS);	// 绘制正方形
        glVertex3f(-1.0f, 1.0f, 0.0f);	// 左上
        glVertex3f( 1.0f, 1.0f, 0.0f);	// 右上
        glVertex3f( 1.0f,-1.0f, 0.0f);	// 左下
        glVertex3f(-1.0f,-1.0f, 0.0f);	// 右下
    glEnd(); // 正方形绘制结束
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

//在这一课中，我已试着尽量详细的解释与多边形绘制有关的步骤。
//并创建了一个绘制三角形和正方形的OpenGL程序。如果您有什么意见或建议请给我EMAIL。
//如果您认为有什么不对或可以改进，请告诉我。我想做最好的OpenGL教程并对您的反馈感兴趣。
