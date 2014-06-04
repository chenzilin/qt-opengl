#include "myglwidget.h"

//二次几何体:
//利用二次几何体，你可以很容易的创建球，圆盘，圆柱和圆锥。

//二次曲面是一种画复合对象的方法，这种方法通常并不需要很多的三角形。我们将要使用第七课的代码。
//我们将要增加7个变量以及修改纹理以增加一些变化 ：

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

//好了我们现在开始搞InitGL()函数。我们打算增加3行代码用来初始化我们的二次曲面。
//这3行代码将在你使1号光源有效后增加，但是要在返回之前。
//第一行代码将初始化二次曲面并且创建一个指向改二次曲面的指针，如果改二次曲面不能被创建的话，那么该指针就是NULL。
//第二行代码将在二次曲面的表面创建平滑的法向量，这样当灯光照上去的时候将会好看些。
//另外一些可能的取值是：GLU_NONE和GLU_FLAT。最后我们使在二次曲面表面的纹理映射有效。
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
}

//接下来就是场景绘制函数了，在这里我只写一个简单的例子。并且当我绘制一个部分的盘子的时候，我将使用一个静态变量
//（一个局部的变量，该变量可以保留他的值不论你任何时候调用他）来表达一个非常酷的效果。为了清晰起见我将要重写DrawGLScene函数。
//你们将会注意到当我讨论这些正在使用的参数时我忽略了当前函数的第一个参数（quadratic）。
//这个参数将被除立方体外的所有对象使用。所以当我讨论这些参数的时候我忽略了它。
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
    glBindTexture(GL_TEXTURE_2D, m_texture[m_filter]);				// 选择由filter决定的纹理
    // 这部分是新增加的
    switch(m_object)						// 绘制哪一种二次几何体
    {
    case 0:// 绘制立方体
        {
            glDrawCube();
            break;
        }
    case 1:
        {
            //我们创建的第2个对象是一个圆柱体。参数1（1.0F）是圆柱体的底面半径，参数2（1.0F）是圆柱体的饿顶面半径，
            //参数3（3.0F）是圆柱体的高度。参数4（32）是纬线（环绕Z轴有多少细分），参数5（32）是经线（沿着Z轴有多少细分）。
            //细分越多该对象就越细致。我们可以用增加细分的方法来增加对象的多边形数。
            //因此你可以牺牲速度换回质量（以时间换质量），大多数的时候我们都可以很容易的找到一个合适的“度”。
            glTranslatef(0.0f,0.0f,-1.5f);
            gluCylinder(m_quadratic, 1.0f, 1.0f, 3.0f, 32, 32);
            break;
        }
    case 2:
        {
            //对象3将会创建一个CD样子的盘子。参数1（0.5F）是盘子的内圆半径，该参数可以为0，则表示在盘子中间没孔，内圆半径越大孔越大。
            //参数2（1.5F）表示外圆半径，这个参数必须比内圆半径大。参数3（32）是组成该盘子的切片的数量，
            //这个数量可以想象成披萨饼中的切片的数量。切片越多，外圆边缘就越平滑。最后一个参数（32）是组成盘子的环的数量。
            //环很像唱片上的轨迹，一环套一环。这些环从内圆半径细分到外圆半径。再说一次，细分越多，速度越慢。
            gluDisk(m_quadratic,0.5f,1.5f,32,32);
            break;
        }
    case 3:
        {
            //我们的第4个对象我知道你们为描述它耗尽精力。就是球。绘制球将会变的非常简单。参数1是球的半径。
            //如果你无法理解半径/直径等等的话，可以理解成物体中心到物体外部的距离，在这里我们使用1.3F作为半径。
            //接下来两个参数就是细分了，和圆柱体一样，参数2是纬线，参数3是经线。细分越多球看起来就越平滑，
            //通常球需要多一些的细分以使他们看起来平滑。
            gluSphere(m_quadratic,1.3f,32,32);
            break;
        }
    case 4:
        {
            //我们创建的第4个对象使用与我们曾经创建的圆柱体一样的命令来创建，如果你还记得的话，
            //我们可以通过控制参数2和参数3来控制顶面半径和地面半径。因此我们可以使顶面半径为0来绘制一个圆锥体，
            //顶面半径为0将会在顶面上创建一个点。因此在下面的代码中，我们使顶面半径等于0，这将会创建一个点，同时也就创建了我们的圆锥。
            glTranslatef(0.0f,0.0f,-1.5f);
            gluCylinder(m_quadratic,1.0f,0.0f,3.0f,32,32);
        }
    case 5:
        {
            //我们的第6个对象将被gluPartialDisk函数创建。我们打算创建的这个对象使用了一些命令，这些命令在我们创建对象之前，
            //你将会清楚的看到。但是命令gluPartialDisk拥有两个新的参数。第5个参数是我们想要绘制的部分盘子的开始角度，参数6是旋转角，
            //也就是转过的角度。我们将要增加旋转角，这将引起盘子沿顺时针方向缓慢的被绘制在屏幕上。一旦旋转角达到360度我们将开始增加开始角度
            //，这样盘子看起来就想是被逐渐的抹去一样。我们将重复这些过程。
            gluPartialDisk(m_quadratic,0.5f,1.5f,32,32,m_part1, m_part2 - m_part1);
            break;
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

//filter 变量跟踪显示时所采用的纹理类型。第一种纹理(texture 0) 使用gl_nearest(不光滑)滤波方式构建。
//第二种纹理 (texture 1) 使用gl_linear(线性滤波) 方式，离屏幕越近的图像看起来就越光滑。
//第三种纹理 (texture 2) 使用 mipmapped滤波方式,这将创建一个外观十分优秀的纹理。
//根据我们的使用类型，filter 变量的值分别等于 0, 1 或 2 。下面我们从第一种纹理开始。
//GLuint texture[3] 为三种不同纹理分配储存空间。它们分别位于在 texture[0], texture[1] 和 texture[2]中。
void MyGLWidget::loadGLTexture()
{
    //这段代码调用前面的代码载入位图，并将其转换成3个纹理。
    QImage image(":/image/Wall.bmp");
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

//现在我决定在本课里保留立方体，这样你可以看到纹理是如何映射到二次曲面对象上的。
//而且我打算将绘制立方体的代码定义为一个单独的函数，这样我们在定义函数Draw()的时候它将会变的不那么凌乱。每个人都应该记住这些代码：
void MyGLWidget::glDrawCube()
{
    glBegin(GL_QUADS);
        // 前面
        glNormal3f( 0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
        // 后面
        glNormal3f( 0.0f, 0.0f,-1.0f);
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

//这就是全部了。现在你可以在OpenGL中绘制二次曲面了。
