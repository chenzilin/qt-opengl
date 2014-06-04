#include "myglwidget.h"

//混合:
//在这一课里，我们在纹理的基础上加上了混合，它看起具有透明的效果，当然解释它不是那么容易，当希望你喜欢它。
//简单的透明
//OpenGL中的绝大多数特效都与某些类型的(色彩)混合有关。混色的定义为，将某个象素的颜色和已绘制在屏幕上与其对应的象素颜色相互结合。
//至于如何结合这两个颜色则依赖于颜色的alpha通道的分量值，以及/或者所使用的混色函数。Alpha通常是位于颜色值末尾的第4个颜色组成分量。
//前面这些课我们都是用GL_RGB来指定颜色的三个分量。相应的GL_RGBA可以指定alpha分量的值。
//更进一步，我们可以使用glColor4f()来代替glColor3f()。
//绝大多数人都认为Alpha分量代表材料的透明度。这就是说，alpha值为0.0时所代表的材料是完全透明的。
//alpha值为1.0时所代表的材料则是完全不透明的。
//混色的公式
//若您对数学不感冒，而只想看看如何实现透明，请跳过这一节。若您想深入理解(色彩)混合的工作原理，这一节应该适合您吧。
//『译者注:其实并不难^-^。原文中的公式如下，CKER再唠叨一下吧。
//其实混合的基本原理是就将要分色的图像各象素的颜色以及背景颜色均按照RGB规则各自分离之后，根据－图像的RGB颜色分量*alpha值+背景的RGB颜色
//分量*(1-alpha值)－这样一个简单公式来混合之后，最后将混合得到的RGB分量重新合并。』
//公式如下：
//(Rs Sr + Rd Dr, Gs Sg + Gd Dg, Bs Sb + Bd Db, As Sa + Ad Da)
//OpenGL按照上面的公式计算这两个象素的混色结果。小写的s和r分别代表源象素和目标象素。大写的S和D则是相应的混色因子。
//这些决定了您如何对这些象素混色。绝大多数情况下，各颜色通道的alpha混色值大小相同，这样对源象素就有 (As, As, As, As)，目标象素则有1,
//1, 1, 1) - (As, As, As, As)。上面的公式就成了下面的模样:
//(Rs As + Rd (1 - As), Gs As + Gd (1 - As), Bs As + Bs (1 - As), As As + Ad (1 - As))
//这个公式会生成透明/半透明的效果。
//OpenGL中的混色
//在OpenGL中实现混色的步骤类似于我们以前提到的OpenGL过程。接着设置公式，并在绘制透明对象时关闭写深度缓存。
//因为我们想在半透明的图形背后绘制 对象。这不是正确的混色方法，但绝大多数时候这种做法在简单的项目中都工作的很好。
//Rui Martins 的补充： 正确的混色过程应该是先绘制全部的场景之后再绘制透明的图形。并且要按照与深度缓存相反的次序来绘制(先画最远的物体)。
//考虑对两个多边形(1和2)进行alpha混合，不同的绘制次序会得到不同的结果。(这里假定多边形1离观察者最近，那么正确的过程应该先画多边形2，
//再画多边形1。正如您再现实中所见到的那样，从这两个<透明的>多边形背后照射来的光线总是先穿过多边形2，再穿过多边形1，最后才到达观察者的眼睛。)
//在深度缓存启用时，您应该将透明图形按照深度进行排序，并在全部场景绘制完毕之后再绘制这些透明物体。
//否则您将得到不正确的结果。我知道某些时候这样做是很令人痛苦的，但这是正确的方法。
//我们将使用第七课的代码。一开始先在代码开始处增加两个新的变量。出于清晰起见，我重写了整段代码。

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_light(false),
    m_x_rotate(0.0f), m_y_rotate(0.0f), m_x_speed(0.0f), m_y_speed(0.0f),
    m_z(-5.0f), m_light_ambient({0.5f, 0.5f, 0.5f, 1.0f}),
    m_light_diffuse({1.0f, 1.0f, 1.0f, 1.0f}),
    m_light_position({0.0f, 0.0f, 2.0f, 1.0f}),
    m_filter(0), m_blend(false)
{
    showNormal();
    startTimer(50);
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(3, &m_texture[0]);
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

//接着应该载入纹理并初始化OpenGL设置了。InitGL函数的第一行使用上面的代码载入纹理。创建纹理之后，我们调用
//glEnable(GL_TEXTURE_2D)启用2D纹理映射。阴影模式设为平滑阴影( smooth shading ）。
//背景色设为黑色，我们启用深度测试，然后我们启用优化透视计算。
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
    //加入以下两行。第一行以全亮度绘制此物体，并对其进行50%的alpha混合(半透明)。
    //当混合选项打开时，此物体将会产生50%的透明效果。第二行设置所采用的混合类型。
    //Rui Martins 的补充: alpha通道的值为 0.0意味着物体材质是完全透明的。1.0 则意味着完全不透明。
    glColor4f(1.0f,1.0f,1.0f,0.5f);			// 全亮度， 50% Alpha 混合
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);		// 基于源象素alpha通道值的半透明混合函数
    if(m_blend)				// 混合打开了么?
    {
        glEnable(GL_BLEND);		// 打开混合
        glDisable(GL_DEPTH_TEST);	// 关闭深度测试

    }
    else					// 否则
    {
        glDisable(GL_BLEND);		// 关闭混合
        glEnable(GL_DEPTH_TEST);	// 打开深度测试
    }
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
    glTranslatef(0.0f,0.0f,m_z);						// 移入屏幕z个单位
    //下三行代码放置并旋转贴图立方体。glTranslatef(0.0f,0.0f,z)将立方体沿着Z轴移动Z单位。
    //glRotatef(xrot,1.0f,0.0f,0.0f)将立方体绕X轴旋转xrot。
    //glRotatef(yrot,0.0f,1.0f,0.0f)将立方体绕Y轴旋转yrot。
    glRotatef(m_x_rotate,1.0f,0.0f,0.0f);						// X轴旋转
    glRotatef(m_y_rotate,0.0f,1.0f,0.0f);						// Y轴旋转

    //下一行与我们在第六课中的类似。有所不同的是，这次我们绑定的纹理是texture[filter]，而不是上一课中的texture[0]。
    //任何时候，我们按下F键，filter 的值就会增加。如果这个数值大于2，变量filter 将被重置为0。
    //程序初始时，变量filter 的值也将设为0。使用变量filter 我们就可以选择三种纹理中的任意一种。
    glBindTexture(GL_TEXTURE_2D, m_texture[m_filter]);				// 选择由filter决定的纹理
    //为了将纹理正确的映射到四边形上，您必须将纹理的右上角映射到四边形的右上角，纹理的左上角映射到四边形的左上角，
    //纹理的右下角映射到四边形的右下角，纹理的左下角映射到四边形的左下角。
    //如果映射错误的话，图像显示时可能上下颠倒，侧向一边或者什么都不是。
    //glTexCoord2f 的第一个参数是X坐标。 0.0f 是纹理的左侧。 0.5f 是纹理的中点， 1.0f 是纹理的右侧。
    //glTexCoord2f 的第二个参数是Y坐标。 0.0f 是纹理的底部。 0.5f 是纹理的中点， 1.0f 是纹理的顶部。
    //所以纹理的左上坐标是 X：0.0f，Y：1.0f ，四边形的左上顶点是 X： -1.0f，Y：1.0f 。其余三点依此类推。
    //试着玩玩 glTexCoord2f X, Y坐标参数。把 1.0f 改为 0.5f 将只显示纹理的左半部分，把 0.0f 改为 0.5f 将只显示纹理的右半部分。
    //glNormal3f是这一课的新东西。Normal就是法线的意思，所谓法线是指经过面(多边形）上的一点且垂直于这个面(多边形)的直线。
    //使用光源的时候必须指定一条法线。法线告诉OpenGL这个多边形的朝向，并指明多边形的正面和背面。
    //如果没有指定法线，什么怪事情都可能发生：不该照亮的面被照亮了，多边形的背面也被照亮....。对了，法线应该指向多边形的外侧。
    //看着木箱的前面您会注意到法线与Z轴正向同向。这意味着法线正指向观察者－您自己。这正是我们所希望的。
    //对于木箱的背面，也正如我们所要的，法线背对着观察者。如果立方体沿着X或Y轴转个180度的话，
    //前侧面的法线仍然朝着观察者，背面的法线也还是背对着观察者。换句话说，不管是哪个面，只要它朝着观察者这个面的法线就指向观察者。
    //由于光源紧邻观察者，任何时候法线对着观察者时，这个面就会被照亮。并且法线越朝着光源，就显得越亮一些。
    //如果您把观察点放到立方体内部，你就会法线里面一片漆黑。因为法线是向外指的。如果立方体内部没有光源的话，当然是一片漆黑。
    glBegin(GL_QUADS);
            // 前面
            glNormal3f( 0.0f, 0.0f, 1.0f);					// 法线指向观察者
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);	// 纹理和四边形的左下
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);	// 纹理和四边形的右下
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);	// 纹理和四边形的右上
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);	// 纹理和四边形的左上
            // 后面
            glNormal3f( 0.0f, 0.0f,-1.0f);					// 法线背向观察者
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);	// 纹理和四边形的右下
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);	// 纹理和四边形的右上
            glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);	// 纹理和四边形的左上
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);	// 纹理和四边形的左下
            // 顶面
            glNormal3f( 0.0f, 1.0f, 0.0f);					// 法线向上
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);	// 纹理和四边形的左上
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);	// 纹理和四边形的左下
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);	// 纹理和四边形的右下
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);	// 纹理和四边形的右上
            // 底面
            glNormal3f( 0.0f,-1.0f, 0.0f);					// 法线朝下
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);	// 纹理和四边形的右上
            glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);	// 纹理和四边形的左上
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);	// 纹理和四边形的左下
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);	// 纹理和四边形的右下

            // 右面
            glNormal3f( 1.0f, 0.0f, 0.0f);					// 法线朝右
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);	// 纹理和四边形的右下
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);	// 纹理和四边形的右上
            glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);	// 纹理和四边形的左上
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);	// 纹理和四边形的左下

            // 左面
            glNormal3f(-1.0f, 0.0f, 0.0f);					// 法线朝左
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
        case Qt::Key_B:
        {
            m_blend = !m_blend;				// 切换混合选项的 TRUE / FALSE
            if(m_blend)				// 混合打开了么?
            {
                glEnable(GL_BLEND);		// 打开混合
                glDisable(GL_DEPTH_TEST);	// 关闭深度测试

            }
            else					// 否则
            {
                glDisable(GL_BLEND);		// 关闭混合
                glEnable(GL_DEPTH_TEST);	// 打开深度测试
            }
            break;
        }
    }
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    //现在增加 xrot , yrot 和 zrot 的值。尝试变化每次各变量的改变值来调节立方体的旋转速度，或改变+/-号来调节立方体的旋转方向。
    m_x_rotate+=m_x_speed;								// X 轴旋转
    m_y_rotate+=m_y_speed;								// Y 轴旋转
    updateGL();
    QGLWidget::timerEvent(event);
}

//filter 变量跟踪显示时所采用的纹理类型。第一种纹理(texture 0) 使用gl_nearest(不光滑)滤波方式构建。
//第二种纹理 (texture 1) 使用gl_linear(线性滤波) 方式，离屏幕越近的图像看起来就越光滑。
//第三种纹理 (texture 2) 使用 mipmapped滤波方式,这将创建一个外观十分优秀的纹理。
//根据我们的使用类型，filter 变量的值分别等于 0, 1 或 2 。下面我们从第一种纹理开始。
//GLuint texture[3] 为三种不同纹理分配储存空间。它们分别位于在 texture[0], texture[1] 和 texture[2]中。
void MyGLWidget::loadGLTexture()
{
    //这段代码调用前面的代码载入位图，并将其转换成3个纹理。
    QImage image(":/image/Glass.bmp");
    image = image.convertToFormat(QImage::Format_RGB888);
    image = image.mirrored();

    glGenTextures(3, &m_texture[0]);					// 创建纹理
    //第六课中我们使用了线性滤波的纹理贴图。这需要机器有相当高的处理能力，但它们看起来很不错。
    //这一课中，我们接着要创建的第一种纹理使用 GL_NEAREST方式。从原理上讲，这种方式没有真正进行滤波。
    //它只占用很小的处理能力，看起来也很差。唯一的好处是这样我们的工程在很快和很慢的机器上都可以正常运行。
    //您会注意到我们在 MIN 和 MAG 时都采用了GL_NEAREST,你可以混合使用 GL_NEAREST 和 GL_LINEAR。
    //纹理看起来效果会好些，但我们更关心速度，所以全采用低质量贴图。MIN_FILTER在图像绘制时小于贴图的原始尺寸时采用。
    //MAG_FILTER在图像绘制时大于贴图的原始尺寸时采用。
    // 创建 Nearest 滤波贴图
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image.width(),
                 image.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, image.bits());
    // 创建线性滤波纹理
    glBindTexture(GL_TEXTURE_2D, m_texture[1]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image.width(),
                 image.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, image.bits());
    //下面是创建纹理的新方法。 Mipmapping!『译者注：这个词的中文我翻不出来，不过没关系。看完这一段，您就知道意思最重要。』
    //您可能会注意到当图像在屏幕上变得很小的时候，很多细节将会丢失。刚才还很不错的图案变得很难看。
    //当您告诉OpenGL创建一个 mipmapped的纹理后，OpenGL将尝试创建不同尺寸的高质量纹理。当您向屏幕绘制一个 mipmapped纹理的时候，
    //OpenGL将选择它已经创建的外观最佳的纹理(带有更多细节)来绘制，而不仅仅是缩放原先的图像(这将导致细节丢失)。
    //我曾经说过有办法可以绕过OpenGL对纹理宽度和高度所加的限制——64、128、256，等等。
    //办法就是 gluBuild2DMipmaps。据我的发现，您可以使用任意的位图来创建纹理。OpenGL将自动将它缩放到正常的大小。
    //因为是第三个纹理，我们将它存到texture[2]。这样本课中的三个纹理全都创建好了。
    // 创建 MipMapped 纹理
    glBindTexture(GL_TEXTURE_2D, m_texture[2]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image.width(),
                      image.width(), GL_RGB, GL_UNSIGNED_BYTE, image.bits());
}

//但是怎样才能在使用纹理贴图的时候指定混合时的颜色呢?很简单，在调整贴图模式时，
//文理贴图的每个象素点的颜色都是由alpha通道参数与当前地象素颜色相乘所得到的。
//比如，绘制的颜色是 (0.5, 0.6, 0.4),我们会把颜色相乘得到(0.5, 0.6, 0.4, 0.2) (alpha参数在没有指定时，缺省为零)。
//就是如此！OpenGL实现Alpha混合的确很简单！

//原文注 (11/13/99)
//我(NeHe)混色代码进行了修改，以使显示的物体看起来更逼真。同时对源象素和目的象素使用alpha参数来混合，会导致物体的人造痕迹看起来很明显。
//会使得物体的背面沿着侧面的地方显得更暗。基本上物体会看起来很怪异。
//我所用的混色方法也许不是最好的，但的确能够工作。启用光源之后，物体看起来很逼真。
//感谢Tom提供的原始代码，他采用的混色方法是正确的，但物体看起来并不象所期望的那样吸引人:)
//代码所作的再次修改是因为在某些显卡上glDepthMask()函数存在寻址问题。
//这条命令在某些卡上启用或关闭深度缓冲测试时似乎不是很有效，所以我已经将启用或关闭深度缓冲测试的代码转成老式的glEnable和glDisable。
//纹理贴图的Alpha混合
//用于纹理贴图的alpha参数可以象颜色一样从问题贴图中读取。
//方法如下，您需要在载入所需的材质同时取得其的alpha参数。然后在调用glTexImage2D()时使用GL_RGBA的颜色格式。
