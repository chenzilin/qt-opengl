#include "myglwidget.h"

//旋转:
//在这一课里，我将教会你如何旋转三角形和四边形。左图中的三角形沿Y轴旋转，四边形沿着X轴旋转。
MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_rotate_tri(0.0f), m_rotate_quad(0.0f)
{
    showNormal();
    startTimer(50);
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

//上一课中我教给您三角形和四边形的着色。这一课我将教您如何将这些彩色对象绕着坐标轴旋转。
//其实只需在上节课的代码上增加几行就可以了。下面我将整个例程重写一遍。方便您知道增加了什么，修改了什么。
//我们增加两个变量来控制这两个对象的旋转。
//这两个变量加在程序的开始处其他变量的后面( bool fullscreen=TRUE;下面的两行)。
//它们是浮点类型的变量，使得我们能够非常精确地旋转对象。浮点数包含小数位置，这意味着我们无需使用1、2、3...的角度。
//你会发现浮点数是OpenGL编程的基础。新变量中叫做 rtri 的用来旋转三角形， rquad 旋转四边形
void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// 清除屏幕及深度缓存
    glLoadIdentity();// 重置模型观察矩阵
    glTranslatef(-1.5f,0.0f,-6.0f);// 左移 1.5 单位，并移入屏幕 6.0

    //下一行代码是新的。glRotatef(Angle,Xvector,Yvector,Zvector)负责让对象绕某个轴旋转。这个命令有很多用处。
    //Angle 通常是个变量代表对象转过的角度。 Xvector , Yvector 和 Zvector 三个参数则共同决定旋转轴的方向。
    //比如(1,0,0)所描述的矢量经过X坐标轴的1个单位处并且方向向右。(-1,0,0)所描述的矢量经过X坐标轴的1个单位处，但方向向左。
    //D. Michael Traub:提供了对 Xvector , Yvector 和 Zvector 的上述解释。
    //为了更好的理解X, Y 和 Z的旋转，我举些例子...
    //X轴－您正在使用一台台锯。锯片中心的轴从左至右摆放(就像OpenGL中的X轴)。
    //尖利的锯齿绕着X轴狂转，看起来要么向上转，要么向下转。取决于锯片开始转时的方向。
    //这与我们在OpenGL中绕着X轴旋转什么的情形是一样的。(译者注：这会儿您要把脸蛋凑向显示器的话，保准被锯开了花 ^-^。)
    //Y轴－假设您正处于一个巨大的龙卷风中心，龙卷风的中心从地面指向天空(就像OpenGL中的Y轴)。
    //垃圾和碎片围着Y轴从左向右或是从右向左狂转不止。这与我们在OpenGL中绕着Y轴旋转什么的情形是一样的。
    //Z轴－您从正前方看着一台风扇。风扇的中心正好朝着您(就像OpenGL中的Z轴)。风扇的叶片绕着Z轴顺时针或逆时针狂转。
    //这与我们在OpenGL中绕着Z轴旋转什么的情形是一样的。
    //下面的一行代码中，如果rtri等于7，我们将三角形绕着Y轴从左向右旋转7 。您也可以改变参数的值，让三角形绕着X和Y轴同时旋转。
    glRotatef(m_rotate_tri,0.0f,1.0f,0.0f);				// Y轴旋转三角形
    //下面的代码没有变化。在屏幕的左面画了一个彩色渐变三角形，并绕着Y轴从左向右旋转。
    glBegin(GL_TRIANGLES);					// 绘制三角形
        glColor3f(1.0f,0.0f,0.0f);			// 设置当前色为红色
        glVertex3f( 0.0f, 1.0f, 0.0f);			// 上顶点
        glColor3f(0.0f,1.0f,0.0f);			// 设置当前色为绿色
        glVertex3f(-1.0f,-1.0f, 0.0f);			// 左下
        glColor3f(0.0f,0.0f,1.0f);			// 设置当前色为蓝色
        glVertex3f( 1.0f,-1.0f, 0.0f);			// 右下
    glEnd();						// 三角形绘制结束

    //您会注意下面的代码中我们增加了另一个glLoadIdentity()调用。目的是为了重置模型观察矩阵。
    //如果我们没有重置，直接调用glTranslate的话，会出现意料之外的结果。因为坐标轴已经旋转了，很可能没有朝着您所希望的方向。
    //所以我们本来想要左右移动对象的，就可能变成上下移动了，取决于您将坐标轴旋转了多少角度。
    //试试将glLoadIdentity() 注释掉之后，会出现什么结果。
    //重置模型观察矩阵之后，X，Y，Z轴都以复位，我们调用glTranslate。
    //您会注意到这次我们只向右一了1.5单位，而不是上节课的3.0单位。因为我们重置场景的时候，焦点又回到了场景的中心(0.0处)。
    //这样就只需向右移1.5单位就够了。
    //当我们移到新位置后，绕X轴旋转四边形。正方形将上下转动。
    glLoadIdentity();					// 重置模型观察矩阵
    glTranslatef(1.5f,0.0f,-6.0f);				// 右移1.5单位,并移入屏幕 6.0
    glRotatef(m_rotate_quad,1.0f,0.0f,0.0f);			//  X轴旋转四边形
    //下一段代码保持不变。在屏幕的右侧画一个蓝色的正方形
    glColor3f(0.5f,0.5f,1.0f);				// 一次性将当前色设置为蓝色
    glBegin(GL_QUADS);					// 绘制正方形
        glVertex3f(-1.0f, 1.0f, 0.0f);			// 左上
        glVertex3f( 1.0f, 1.0f, 0.0f);			// 右上
        glVertex3f( 1.0f,-1.0f, 0.0f);			// 左下
        glVertex3f(-1.0f,-1.0f, 0.0f);			// 右下
    glEnd();						// 正方形绘制结束
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
    //下两行是新增的。倘若把 rtri 和 rquad 想象为容器，那么在程序的开始我们创建了容器( GLfloat rtri , 和 GLfloat rquad )。
    //当容器创建之后，里面是空的。下面的第一行代码是向容器中添加0.2。
    //因此每次当我们运行完前面的代码后，都会在这里使 rtri 容器中的值增长0.2。
    //后面一行将 rquad 容器中的值减少0.15。同样每次当我们运行完前面的代码后，都会在这里使 rquad 容器中的值下跌0.15。
    //下跌最终会导致对象旋转的方向和增长的方向相反。
    //尝试改变下面代码中的+和-，来体会对象旋转的方向是如何改变的。
    //并试着将0.2改成1.0。这个数字越大，物体就转的越快，这个数字越小，物体转的就越慢。
    m_rotate_tri+=0.2f;						// 增加三角形的旋转变量
    m_rotate_quad-=0.15f;						// 减少四边形的旋转变量
    updateGL();
    QGLWidget::timerEvent(event);
}

//在这一课中，我试着尽量详细的解释如何让对象绕某个轴转动。改改代码，试着让对象绕着Z轴、X+Y轴或者所有三个轴来转动:)。
//如果您有什么意见或建议请给我EMAIL。如果您认为有什么不对或可以改进，请告诉我。我想做最好的OpenGL教程并对您的反馈感兴趣。
