#include "myglwidget.h"

//3D空间中移动图像:
//你想知道如何在3D空间中移动物体，你想知道如何在屏幕上绘制一个图像，而让图像的背景色变为透明，你希望有一个简单的动画。
//这一课将教会你所有的一切。前面的课程涵盖了基础的OpenGL，每一课都是在前一课的基础上创建的。
//这一课是前面几课知识的综合，当你学习这课时，请确保你已经掌握了前面几课的知识。

//欢迎进入第九课。到现在为止，您应该很好的理解OpenGL了。『CKER：如果没有的话，一定是我翻译的罪过......』。
//您已经学会了设置一个OpenGL窗口的每个细节。学会在旋转的物体上贴图并打上光线以及混色(透明)处理。
//这一课应该算是第一课中级教程。您将学到如下的知识：在3D场景中移动位图，并去除位图上的黑色象素(使用混色)。
//接着为黑白纹理上色，最后您将学会创建丰富的色彩，并把上过不同色彩的纹理相互混合，得到简单的动画效果。
//我们在第一课的代码基础上进行修改。先在程序源码的开始处增加几个变量。出于清晰起见，我重写了整段代码。

// twinkle用来跟踪 闪烁 效果是否启用
// num 跟踪屏幕上所绘制的星星数。

const int starCount = 50;
MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_twinkle(false),
    m_zoom(-15.0f), m_tilt(90.0f), m_spin(0.0f)
{
    showNormal();
    startTimer(100);
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(1, &m_texture[0]);
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

//现在设置OpenGL的渲染方式。这里不打算使用深度测试，如果您使用第一课的代码的话，
//请确认是否已经去掉了glDepthFunc(GL_LEQUAL); 和 glEnable(GL_DEPTH_TEST);两行。
//否则，您所见到的效果将会一团糟。这里我们使用了纹理映射，因此请您确认您已经加上了这些第一课中所没有的代码。
//您会注意到我们通过混色来启用了纹理映射。
void MyGLWidget::initializeGL()
{
    loadTexture();

    glEnable(GL_TEXTURE_2D);				// 启用纹理映射
    glShadeModel(GL_SMOOTH);				// 启用阴影平滑
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);			// 黑色背景
    glClearDepth(1.0f);					// 设置深度缓存
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// 真正精细的透视修正
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);			// 设置混色函数取得半透明效果
    glEnable(GL_BLEND);					// 启用混色
    //以下是新增的代码。设置了每颗星星的起始角度、距离、和颜色。您会注意到修改结构的属性有多容易。
    //全部50颗星星都会被循环设置。要改变star[1]的角度我们所要做的只是star[1].angle={某个数值}；就这么简单！
    for (int i = 0, iend = starCount; i<iend; i++)				// 创建循环设置全部星星
    {
        stars star;
        star.angle=0.0f;				// 所有星星都从零角度开始
        //第i颗星星离中心的距离是将i的值除以星星的总颗数，然后乘上5.0f。
        //基本上这样使得后一颗星星比前一颗星星离中心更远一点。
        //这样当i为50时(最后一颗星星),i 除以 iend正好是1.0f。
        //之所以要乘以5.0f是因为1.0f*5.0f 就是 5.0f。『CKER：废话，废话！这老外怎么跟孔乙己似的！:)』5.0f已经很接近屏幕边缘。
        //我不想星星飞出屏幕，5.0f是最好的选择了。当然如果如果您将场景设置的更深入屏幕里面的话，也许可以使用大于5.0f的数值，
        //但星星看起来就更小一些(都是透视的缘故)。
        //您还会注意到每颗星星的颜色都是从0～255之间的一个随机数。
        //也许您会奇怪为何这里的颜色得取值范围不是OpenGL通常的0.0f～1.0f之间。
        //这里我们使用的颜色设置函数是glColor4ub，而不是以前的glColor4f。ub意味着参数是Unsigned Byte型的。
        //一个byte的取值范围是0～255。这里使用byte值取随机整数似乎要比取一个浮点的随机数更容易一些。
        star.dist=(float(i)/iend)*5.0f;	// 计算星星离中心的距离
        star.r=qrand()%256;			// 设置随机红色分量
        star.g=qrand()%256;			// 设置随机红色分量
        star.b=qrand()%256;			// 设置随机红色分量
        m_stars.push_back(star);
    }

}

//下一段包括了所有的绘图代码。任何您所想在屏幕上显示的东东都将在此段代码中出现。
//以后的每个教程中我都会在例程的此处增加新的代码。如果您对OpenGL已经有所了解的话，您可以在glLoadIdentity()调用之后，
//试着添加一些OpenGL代码来创建基本的形。
//如果您是OpenGL新手，等着我的下个教程。目前我们所作的全部就是将屏幕清除成我们前面所决定的颜色，清除深度缓存并且重置场景。
//我们仍没有绘制任何东东。
void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//清除屏幕和深度缓存
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);		// 选择纹理
    for (int i = 0, iend = starCount;i<iend; i++)				// 循环设置所有的星星
    {
        glLoadIdentity();				// 绘制每颗星星之前，重置模型观察矩阵
        glTranslatef(0.0f,0.0f,m_zoom);			// 深入屏幕里面
        glRotatef(m_tilt,1.0f,0.0f,0.0f);			// 倾斜视角
        //现在我们来移动星星。星星开始时位于屏幕的中心。我们要做的第一件事是把场景沿Y轴旋转。
        //如果我们旋转90度的话，X轴不再是自左至右的了，他将由里向外穿出屏幕。为了让大家更清楚些，举个例子。
        //假想您站在房子中间。再设想您左侧的墙上写着-x，前面的墙上写着-z，右面墙上就是+x咯，您身后的墙上则是+z。
        //加入整个房子向右转90度，但您没有动，那么前面的墙上将是-x而不再是-z了。所有其他的墙也都跟着移动。
        //-z出现在右侧，+z出现在左侧，+x出现在您背后。神经错乱了吧？通过旋转场景，我们改变了x和z平面的方向。
        //第二行代码沿x轴移动一个正值。通常x轴上的正值代表移向了屏幕的右侧(也就是通常的x轴的正向)，但这里由于我们绕y轴旋转了坐标系，
        //x轴的正向可以是任意方向。如果我们转180度的话，屏幕的左右侧就镜像反向了。
        //因此，当我们沿 x轴正向移动时，可能向左，向右，向前或向后。
        glRotatef(m_stars[i].angle,0.0f,1.0f,0.0f);	// 旋转至当前所画星星的角度
        glTranslatef(m_stars[i].dist,0.0f,0.0f);	// X轴正向移动
        //接着的代码带点小技巧。星星实际上是一个平面的纹理。现在您在屏幕中心画了个平面的四边形然后贴上纹理，这看起来很不错。
        //一切都如您所想的那样。但是当您当您沿着y轴转上个90度的话，纹理在屏幕上就只剩右侧和左侧的两条边朝着您。
        //看起来就是一条细线。这不是我们所想要的。我们希望星星永远正面朝着我们，而不管屏幕如何旋转或倾斜。
        //我们通过在绘制星星之前，抵消对星星所作的任何旋转来实现这个愿望。您可以采用逆序来抵消旋转。
        //当我们倾斜屏幕时，我们实际上以当前角度旋转了星星。通过逆序，我们又以当前角度"反旋转"星星。
        //也就是以当前角度的负值来旋转星星。就是说，如果我们将星星旋转了10度的话，又将其旋转-10度来使星星在那个轴上重新面对屏幕。
        //下面的第一行抵消了沿y轴的旋转。然后，我们还需要抵消掉沿x轴的屏幕倾斜。要做到这一点，我们只需要将屏幕再旋转-tilt倾角。
        //在抵消掉x和y轴的旋转后，星星又完全面对着我们了。
        glRotatef(-m_stars[i].angle,0.0f,1.0f,0.0f);	// 取消当前星星的角度
        glRotatef(-m_tilt,1.0f,0.0f,0.0f);		// 取消屏幕倾斜
        //如果 twinkle 为 TRUE，我们在屏幕上先画一次不旋转的星星：将星星总数(num) 减去当前的星星数(loop)再减去1，
        //来提取每颗星星的不同颜色(这么做是因为循环范围从0到num-1)。举例来说，结果为10的时候，我们就使用10号星星的颜色。
        //这样相邻星星的颜色总是不同的。这不是个好法子，但很有效。最后一个值是alpha通道分量。这个值越小，这颗星星就越暗。
        //由于启用了twinkle，每颗星星最后会被绘制两遍。程序运行起来会慢一些，这要看您的机器性能如何了。
        //但两遍绘制的星星颜色相互融合，会产生很棒的效果。同时由于第一遍的星星没有旋转，启用twinkle后的星星看起来有一种动画效果。
        //(如果您这里看不懂得话，就自己去看程序的运行效果吧。)
        //值得注意的是给纹理上色是件很容易的事。尽管纹理本身是黑白的，纹理将变成我们在绘制它之前选定的任意颜色。
        //此外，同样值得注意的是我们在这里使用的颜色值是byte型的，而不是通常的浮点数。甚至alpha通道分量也是如此。
        if (m_twinkle)					// 启用闪烁效果
        {
            // 使用byte型数值指定一个颜色
            glColor4ub(m_stars[(iend-i)-1].r,m_stars[(iend-i)-1].g,m_stars[(iend-i)-1].b,255);
            glBegin(GL_QUADS);		// 开始绘制纹理映射过的四边形
                glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,-1.0f, 0.0f);
                glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,-1.0f, 0.0f);
                glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, 1.0f, 0.0f);
                glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 0.0f);
            glEnd();				// 四边形绘制结束
        }
        //现在绘制第二遍的星星。唯一和前面的代码不同的是这一遍的星星肯定会被绘制，并且这次的星星绕着z轴旋转。
        glRotatef(m_spin,0.0f,0.0f,1.0f);			// Z轴旋转星星
        // 使用byte型数值指定一个颜色
        glColor4ub(m_stars[i].r,m_stars[i].g,m_stars[i].b,255);
        glBegin(GL_QUADS);				// 开始绘制纹理映射过的四边形
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,-1.0f, 0.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,-1.0f, 0.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, 1.0f, 0.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 0.0f);
        glEnd();					// 四边形绘制结束

        //以下的代码代表星星的运动。我们增加spin的值来旋转所有的星星(公转)。
        //然后，将每颗星星的自转角度增加loop/num。这使离中心更远的星星转的更快。
        //最后减少每颗星星离屏幕中心的距离。这样看起来，星星们好像被不断地吸入屏幕的中心。
        m_spin+=0.01f;					// 星星的公转
        m_stars[i].angle+=float(i)/iend;		// 改变星星的自转角度
        m_stars[i].dist-=0.01f;				// 改变星星离中心的距离
        //接着几行检查星星是否已经碰到了屏幕中心。当星星碰到屏幕中心时，
        //我们为它赋一个新颜色，然后往外移5个单位，这颗星星将踏上它回归屏幕中心的旅程。
        if (m_stars[i].dist<0.0f)			// 星星到达中心了么
        {
            m_stars[i].dist+=5.0f;			// 往外移5个单位
            m_stars[i].r=qrand()%256;		// 赋一个新红色分量
            m_stars[i].g=qrand()%256;		// 赋一个新绿色分量
            m_stars[i].b=qrand()%256;		// 赋一个新蓝色分量
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
        case Qt::Key_T:
        {
            m_twinkle=!m_twinkle;
            break;
        }
        case Qt::Key_Up:
        {
            m_tilt-=0.5f;
            break;
        }
        case Qt::Key_Down:
        {
            m_tilt+=0.5f;
            break;
        }
        case Qt::Key_PageUp:
        {
            m_zoom-=0.2f;
            break;
        }
        case Qt::Key_PageDown:
        {
            m_zoom+=0.2f;
            break;
        }
    }
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    updateGL();
    QGLWidget::timerEvent(event);
}

void MyGLWidget::loadTexture()
{
    QImage image(":/image/Star.bmp");
    image = image.convertToFormat(QImage::Format_RGB888);
    image = image.mirrored();
    glGenTextures(1, &m_texture[0]);// 创建一个纹理
    // 创建一个线性滤波纹理
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image.width(), image.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, image.bits());
}

//这一课我尽我所能来解释如何加载一个灰阶位图纹理，(使用混色)去掉它的背景色后，再给它上色，最后让它在3D场景中移动。
//我已经向您展示了如何创建漂亮的颜色与动画效果。实现原理是在原始位图上再重叠一份位图拷贝。
//到现在为止，只要您很好的理解了我所教您的一切，您应该已经能够毫无问题的制作您自己的3D Demo了。所有的基础知识都已包括在内！
