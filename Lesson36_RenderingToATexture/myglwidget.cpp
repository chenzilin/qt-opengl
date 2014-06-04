#include "myglwidget.h"

//放射模糊和渲染到纹理:
//如何实现放射状的滤镜效果呢，看上去很难，其实很简单。把渲染得图像作为纹理提取出来，在利用OpenGL本身自带的纹理过滤，
//就能实现这种效果，不信，你试试。

//嗨，我是Dario Corno,也因SpinningKids的rIo而为大家所知。首先，我想要解释我为什么决定写这点指南。我自1989年以来就从事scener的工作。
//我想要你们去下载一些demo(示例程序，也就是演示——译者)以帮助你理解什么是Demo并且demo的效果是什么。
//Demos是被用来展示恰似风雅的技术一样无限并且时而严酷的译码。在今天的演示中你通常总可以发现一些真正迷人的效果。
//这不是一本迷人的效果指南，但结果将非常的酷！你能够从http://www.pouet.net和 http://ftp.scene.org. 发现大量的演示收集。
//既然绪论超出了我们探讨的范围，我们可以继续我们的指南了。
//我将解释如何做一个看起来象径向模糊的eye candy 效果。有时它以测定体积的光线被提到。不要相信，它仅仅是一个冒牌的辐射状模糊;D
//辐射状模糊效果通常借助于模糊在一个方向上相对于模糊物的中心原始图象的每一个象素来做的。
//借助于现今的硬件用色彩缓冲器来手工作模糊处理是极其困难的（至少在某种程度上它被所有的gfx卡所支持）因此我们需要一些窍门来达到同样的效果。
//作为一个奖励当学习径向模糊效果时，你同样将学到如何轻松地提供材料的纹理。
//我决定在这篇指南中使用弹簧作为外形因为它是一个酷的外形，另外还因为我对立方体感到厌烦：}
//多留意这篇指南关于如何创建那个效果的指导方针是重要的。我不研究解释那些代码的详情。你应当用心记下它们中的大部分：}
//下面是变量的定义和用到的头文件。

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
}

//下面的代码的作用是重新设置OpenGL场景的大小，而不管窗口的大小是否已经改变(假定您没有使用全屏模式)。
//甚至您无法改变窗口的大小时(例如您在全屏模式下)，它至少仍将运行一次--在程序开始时设置我们的透视图。
//OpenGL场景的尺寸将被设置成它显示时所在窗口的大小。
void MyGLWidget::resizeGL(int w, int h)
{
    glViewport (0, 0, (GLsizei)(w), (GLsizei)(h));// Reset The Current Viewport
    glMatrixMode (GL_PROJECTION);// Select The Projection Matrix
    glLoadIdentity ();// Reset The Projection Matrix
    gluPerspective(50, (float)w/(float)h, 5,  2000);
    glMatrixMode (GL_MODELVIEW);// Select The Modelview Matrix
    glLoadIdentity ();// Reset The Modelview Matrix
}

void MyGLWidget::initializeGL()
{
    // Start Of User Initialization
    m_angle = 0.0f;	// Set Starting Angle To Zero
    m_blurTexture = emptyTexture();// Create Our Empty Texture
    glViewport(0 , 0, this->size().width() , this->size().height());// Set Up A Viewport
    glMatrixMode(GL_PROJECTION);// Select The Projection Matrix
    glLoadIdentity();// Reset The Projection Matrix
    gluPerspective(50, (float)this->size().width()/(float)this->size().height(), 5,  2000);// Set Our Perspective
    glMatrixMode(GL_MODELVIEW);// Select The Modelview Matrix
    glLoadIdentity();// Reset The Modelview Matrix
    glEnable(GL_DEPTH_TEST);// Enable Depth Testing
    GLfloat global_ambient[4]={0.2f, 0.2f,  0.2f, 1.0f};// Set Ambient Lighting To Fairly Dark Light (No Color)
    GLfloat light0pos[4]={0.0f, 5.0f, 10.0f, 1.0f};// Set The Light Position
    GLfloat light0ambient[4]= {0.2f, 0.2f,  0.2f, 1.0f};// More Ambient Light
    GLfloat light0diffuse[4]= {0.3f, 0.3f,  0.3f, 1.0f};// Set The Diffuse Light A Bit Brighter
    GLfloat light0specular[4]={0.8f, 0.8f,  0.8f, 1.0f};// Fairly Bright Specular Lighting
    GLfloat lmodel_ambient[]= {0.2f,0.2f,0.2f,1.0f};// And More Ambient Light
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,lmodel_ambient);// Set The Ambient Light Model
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);// Set The Global Ambient Light Model
    glLightfv(GL_LIGHT0, GL_POSITION, light0pos);// Set The Lights Position
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0ambient);// Set The Ambient Light
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0diffuse);// Set The Diffuse Light
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0specular);// Set Up Specular Lighting
    glEnable(GL_LIGHTING);// Enable Lighting
    glEnable(GL_LIGHT0);// Enable Light0
    glShadeModel(GL_SMOOTH);// Select Smooth Shading
    glMateriali(GL_FRONT, GL_SHININESS, 128);
    glClearColor(0.0f, 0.0f, 0.0f, 0.5);// Set The Clear Color To Black
}

//瞧，这是以前从未见过的最短的绘制程序，有很棒的视觉效果！
//我们调用RenderToTexture 函数。幸亏我们视口改变这个函数才着色被拉伸的弹簧。 对于我们的纹理拉伸的弹簧被着色，并且这些缓冲器被清除。
//我们之后绘制“真正的”弹簧 (你在屏幕上看到的3D实体) 通过调用 ProcessHelix( )。
//最后我们在弹簧前面绘制一些混合的方块。有织纹的方块将被拉伸以适应在真正的3D弹簧
//上面。
void MyGLWidget::paintGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.5);// 将清晰的颜色设定为黑色
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// 清除屏幕和深度缓冲器
    glLoadIdentity();// 重置视图
    renderToTexture();// 着色纹理
    processHelix();// 绘制我们的螺旋
    drawBlur(25,0.02f);// 绘制模糊效果
}

void MyGLWidget::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Escape:
        {
            qApp->exit();
            break;
        }
    }
    QGLWidget::keyPressEvent(event);
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    m_angle += (float)(15.0) / 5.0f;// Update angle Based On The
    updateGL();
    QGLWidget::timerEvent(event);
}

//函数EmptyTexture()创建了一个空的纹理并返回纹理的编号。我们刚分配了一些自由空间（准确的是128*128*4无符号整数）。
//128*128是纹理的大小（128象素宽和高），4意味着为每一个象素我们想用4byte来存储红，绿，蓝和ALPHA组件。
// 创建一个空的纹理
GLuint MyGLWidget::emptyTexture()
{
    GLuint txtnumber;							// 纹理ID
    unsigned int* data;						// 存储数据
    // 为纹理数据（128*128*4）建立存储区
    data = (unsigned int*)new GLuint[((128 * 128)* 4 * sizeof(unsigned int))];
    //在分配完空间之后我们用ZeroMemory函数清0，返回指针（数据）和被清0的存贮区的大小。
    //另一半需注意的重要的事情是我们设置GL_LINEAR的放大率和缩放率的方法。因为我们将被我们的纹理要求投入全部的精力并且如果被滥用，
    //GL_NEAREST会看起来非常糟糕。
    glGenTextures(1, &txtnumber);// 创建一个纹理
    glBindTexture(GL_TEXTURE_2D, txtnumber);// 构造纹理
    glTexImage2D(GL_TEXTURE_2D, 0, 4, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);// 用数据中的信息构造纹理
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    delete [] data;// 释放数据
    return txtnumber;// 返回纹理ID
}

//这个函数简单规格化法线向量的长度。向量被当作有3个浮点类型的元素的数组来表示，第一个元素表示X轴，第二个表示Y，第三个表示Z。
//一个规格化的向量[Nv]被Vn表达为Vn=[Vox/|Vo|，Voy/|Vo|，Voz/|Vo|]，这里Vo是最初的向量，|Vo|是该向量的系数（或长度），
//X，Y，Z它的组件。之后由向量的长度区分每一个法线向量组件。
// 归一化一个法向量
void MyGLWidget::reduceToUnit(float vector[3])
{
    // 一定长度的单位法线向量
    float length;// 保存长度
    // 计算向量
    length = (float)qSqrt((vector[0]*vector[0]) + (vector[1]*vector[1]) + (vector[2]*vector[2]));
    if(length == 0.0f)// 避免除0错误
    {
        length = 1.0f;// 如果为0设置为1
    }
    vector[0] /= length;// 归一化向量
    vector[1] /= length;
    vector[2] /= length;
}

//下面各项计算所给的3个顶点向量（总在3个浮点数组中）。我们有两个参数：v[3][3]和out[3]。当然第一个参数是一个m=3，
//n=3每一行代表三角形一个顶点的浮点矩阵。Out是我们要放置作为结果的法线向量的位置。
//相当简单的数学。我们将使用著名的交叉乘积运算。理论上说交叉乘积是两个向量——它返回另一个直交向量到两个原始向量——之间的操作。
//法线向量是一个垂直物体表面的直交向量，是与该表面相对的（通常一个规格化的长度）。设想两个向量是在一个三角形的一侧的上方，
//那么这个三角形两边的直交向量（由交叉乘积计算）就是那个三角形的法线。
//解释比实行还难。
//我们将着手从现存的顶点0到顶点1，从顶点1到顶点2找到那个向量。这是基本上通过减法——下一个顶点的每个组件减一个顶点的每个组件——作好了的。
//现在我们已经为我们的三角形的边找到了那个向量。通过交叉相乘我们为那个三角形找到了法线向量。
//看代码。
//V[0][ ]是第一个顶点，v[1][ ]是第二个顶点，v[2][ ]是第三个顶点。每个顶点包括：v[ ][0]是顶点的x坐标，v[ ][1]是顶点的y坐标，
//v[ ][2]是顶点的z坐标。
//通过简单的减法从一个顶点的每个坐标到另一个顶点每个坐标我们得到了那个VECTOR。v1[0] = v[0][0] - v[1][0]，
//这计算现存的从一个顶点到另一个顶点的向量的X组件，v1[1] = v[0][1] - v[1][1]将计算Y组件，v1[2] = v[0][2] - v[1][2]
//计算Z组件等等。
//现在我们有了两个向量，所以我们计算它们的交叉乘积得到那个三角形的法线。
//交叉相乘的规则是：
//out[x] = v1[y] * v2[z] - v1[z] * v2[y]
//out[y] = v1[z] * v2[x] - v1[x] * v2[z]
//out[z] = v1[x] * v2[y] - v1[y] * v2[x]
//我们最终得到了这个三角形的法线in out[ ]。

// 用三点计算一个立方体法线
void MyGLWidget::calcNormal(float v[3][3], float out[3])
{
    float v1[3],v2[3];// 向量 1 (x,y,z) 和向量 2 (x,y,z)
    static const int x = 0;	// 定义 X坐标
    static const int y = 1;	// 定义 Y 坐标
    static const int z = 2;	// 定义 Z 坐标
    // 用减法在两点之间得到向量
    // 从一点到另一点的X，Y，Z坐标
    // 计算点1到点0的向量
    v1[x] = v[0][x] - v[1][x];
    v1[y] = v[0][y] - v[1][y];
    v1[z] = v[0][z] - v[1][z];
    // 计算点2到点1的向量
    v2[x] = v[1][x] - v[2][x];
    v2[y] = v[1][y] - v[2][y];
    v2[z] = v[1][z] - v[2][z];
    // 计算交叉乘积为我们提供一个表面的法线
    out[x] = v1[y]*v2[z] - v1[z]*v2[y];
    out[y] = v1[z]*v2[x] - v1[x]*v2[z];
    out[z] = v1[x]*v2[y] - v1[y]*v2[x];
    reduceToUnit(out);//  规格化向量
}

//下面的例子正好用gluLookAt设立了一个观察点。我们设置一个观察点放置在0，5，50位置——正照看0，0，0
//并且所属的向上的向量正仰望（0，1，0）！
// 绘制一个螺旋
void MyGLWidget::processHelix()
{
    GLfloat x;// 螺旋x坐标
    GLfloat y;// 螺旋y坐标
    GLfloat z;// 螺旋z坐标
    GLfloat phi;// 角
    GLfloat theta;// 角
    GLfloat v,u;// 角
    GLfloat r;// 螺旋半径
    int twists = 5;// 5个螺旋
    GLfloat glfMaterialColor[]={0.4f,0.2f,0.8f,1.0f};// 设置材料色彩
    GLfloat specular[]={1.0f,1.0f,1.0f,1.0f};// 设置镜象灯光
    glLoadIdentity();// 重置Modelview矩阵
    gluLookAt(0, 5, 50, 0, 0, 0, 0, 1, 0);// 场景（0，0，0）的视点中心 (0,5,50)，Y轴向上
    glPushMatrix();// 保存Modelview矩阵
        glTranslatef(0,0,-50);// 移入屏幕50个单位
        glRotatef(m_angle/2.0f,1,0,0);// 在X轴上以1/2角度旋转
        glRotatef(m_angle/3.0f,0,1,0);// 在Y轴上以1/3角度旋转
        glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,glfMaterialColor);
        glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,specular);
        //然后我们计算螺旋的公式并给弹簧着色。十分简单，我就不再解释了，因为它不是这篇指南的主要目的。
        //这段螺旋代码经过软件赞助者的许可被借用（并作了一点优化）。这是写作的简单的方法，但不是最块的方法。使用顶点数组可以使它更快！
        r=1.5f;	// 半径
        glBegin(GL_QUADS);// 开始绘制立方体
            for(phi=0; phi <= 360; phi+=20.0)// 以20度的间隔绘制
            {
                for(theta=0; theta<=360*twists; theta+=20.0)
                {
                    v=(phi/180.0f*3.142f);// 计算第一个点 ( 0 )的角度
                    u=(theta/180.0f*3.142f);// 计算第一个点 ( 0 )的角度

                    x=float(qCos(u)*(2.0f+qCos(v) ))*r;// 计算x的位置（第一个点）
                    y=float(qSin(u)*(2.0f+qCos(v) ))*r;// 计算y的位置（第一个位置）
                    z=float((( u-(2.0f*3.142f)) + qSin(v) ) * r);// 计算z的位置（第一个位置）

                    m_vertexes[0][0]=x;// 设置第一个顶点的x值
                    m_vertexes[0][1]=y;// 设置第一个顶点的y值
                    m_vertexes[0][2]=z;// 设置第一个顶点的z值

                    v=(phi/180.0f*3.142f);// 计算第二个点（ 0 ）的角度
                    u=((theta+20)/180.0f*3.142f);// 计算第二个点（ 20 ）的角度

                    x=float(qCos(u)*(2.0f+qCos(v) ))*r;// 计算x位置（第二个点)
                    y=float(qSin(u)*(2.0f+qCos(v) ))*r;// 计算y位置（第二个点)
                    z=float((( u-(2.0f*3.142f)) + qSin(v) ) * r);// 计算z位置（第二个点)

                    m_vertexes[1][0]=x;// 设置第二个顶点的x值
                    m_vertexes[1][1]=y;// 设置第二个顶点的y值
                    m_vertexes[1][2]=z;// 设置第二个顶点的z值

                    v=((phi+20)/180.0f*3.142f);// 计算第三个点 ( 20 )的角度
                    u=((theta+20)/180.0f*3.142f);// 计算第三个点 ( 20 )的角度

                    x=float(qCos(u)*(2.0f+qCos(v) ))*r;// 计算x位置 (第三个点)
                    y=float(qSin(u)*(2.0f+qCos(v) ))*r;// 计算y位置 (第三个点)
                    z=float((( u-(2.0f*3.142f)) + qSin(v) ) * r);// 计算z位置 (第三个点)

                    m_vertexes[2][0]=x;// 设置第三个顶点的x值
                    m_vertexes[2][1]=y;// 设置第三个顶点的y值
                    m_vertexes[2][2]=z;// 设置第三个顶点的z值

                    v=((phi+20)/180.0f*3.142f);// 计算第四个点( 20 )的角度
                    u=((theta)/180.0f*3.142f);// 计算第四个点( 0 )的角度

                    x=float(qCos(u)*(2.0f+qCos(v) ))*r;// 计算x位置 (第四个点)
                    y=float(qSin(u)*(2.0f+qCos(v) ))*r;// 计算y位置 (第四个点)
                    z=float((( u-(2.0f*3.142f)) + qSin(v) ) * r);// 计算z位置 (第四个点))

                    m_vertexes[3][0]=x;// 设置第四个顶点的x值
                    m_vertexes[3][1]=y;// 设置第四个顶点的y值
                    m_vertexes[3][2]=z;// 设置第四个顶点的z值

                    calcNormal(m_vertexes, m_normal);// 计算立方体的法线
                    glNormal3f(m_normal[0],m_normal[1],m_normal[2]);// 设置法线
                    // 渲染四边形
                    glVertex3f(m_vertexes[0][0],m_vertexes[0][1],m_vertexes[0][2]);
                    glVertex3f(m_vertexes[1][0],m_vertexes[1][1],m_vertexes[1][2]);
                    glVertex3f(m_vertexes[2][0],m_vertexes[2][1],m_vertexes[2][2]);
                    glVertex3f(m_vertexes[3][0],m_vertexes[3][1],m_vertexes[3][2]);
                }
            }
        glEnd();// 绘制结束
    glPopMatrix();// 取出矩阵
}

//这两个事例（ViewOrtho and ViewPerspective）被编码以使它变得很容易地在一个直交的情形下绘制并且不费力的返回透视图。
//ViewOrtho简单地设立了这个射影矩阵，然后增加一份现行射影矩阵的拷贝到OpenGL栈上。
//这个恒等矩阵然后被装载并且当前屏幕正投影观察决议被提出。
//利用2维坐标以屏幕左上角0，0和屏幕右下角639，479来绘制是可能的。
//最后，modelview矩阵为透视材料激活。
//ViewPerspective设置射影矩阵模式取回ViewOrtho在堆栈上推进的非正交矩阵。然后样本视图被选择因此我们可以透视材料。
//我建议你保留这两个过程，能够着色2D而不需担心射影矩阵很不错。

// 设置一个z正视图
void MyGLWidget::viewOrtho()
{
    glMatrixMode(GL_PROJECTION);// 选择投影矩阵
    glPushMatrix();	// 保存当前矩阵
    glLoadIdentity();// 重置矩阵
    glOrtho( 0, 640 , 480 , 0, -1, 1 );// 选择标准模式
    glMatrixMode(GL_MODELVIEW);	// 选择样本视图矩阵
    glPushMatrix();	// 保存当前矩阵
    glLoadIdentity();// 重置矩阵
}

// 设置透视视图
void MyGLWidget::viewPerspective()
{
    glMatrixMode( GL_PROJECTION );// 选择投影矩阵
    glPopMatrix();// 取出矩阵
    glMatrixMode( GL_MODELVIEW );// 选择模型变换矩阵
    glPopMatrix();//弹出矩阵
}

//现在是解释那个冒牌的辐射状的模糊效果是如何作的时候了。
//我们需要绘制这个场景——它从中心开始在所有方向上模糊出现。窍门是在没有主要的性能瓶颈的情况下做出的。
//我们不能读写象素，并且如果我们想和非kick-butt视频卡兼容，我们不能使用扩展名何驱动程序特殊命令。没办法了吗？
//不，解决方法非常简单，OpenGL赋予我们“模糊”纹理的能力。OK……并非真正的模糊，但我们利用线性过滤去依比例决定一个纹理，
//结果（有些想象成分）看起来象高斯模糊。
//因此如果我们正确地在3D场景中放了大量的被拉伸的纹理并依比例决定会有什么发生？
//答案比你想象的还简单。
//问题一：透视一个纹理
//有一个后缓冲器在象素格式下问题容易解决。在没有后缓冲器的情况下透视一个纹理在眼睛看来是一个真正的痛苦。
//透视纹理刚好借助一个函数来完成。我们需要绘制我们的实体然后利用glCopytexImage函数复制这个结果（在交换前，后缓冲器之前）后到纹理。
//问题二：在3D实体前精确地协调纹理。
//我们知道：如果我们在没有设置正确的透视的情况下改变了视口，我们就得到一个我们的实体的一个被拉伸的透视图。
//例如如果我们设置一个是视口足够宽我们就得到一个垂直地被拉伸的透视图。
//解决方法是首先设置一个视口正如我们的纹理（128×128）。透视我们的实体到这个纹理之后，我们利用当前屏幕决议着色这个纹理到屏幕。
//这种方法OpenGL缩减这个实体去适应纹理，并且我们拉伸纹理到全屏大小时，OpenGL重新调整纹理的大小去完美的适应在我们的3d实体顶端。
//希望我没有丢掉任何一点。另一个灵活的例子是，如果你取一个640×480大小screenshot，然后调整成为256x256的位图，
//你可以以一个纹理装载这个位图，并拉伸它使之适合640x480的屏幕。这个质量可能不会以前一样好，
//但是这个纹理排列起的效果应当接近最初的640x480图象。
//On to the fun stuff! 这个函数相当简单，并且是我的首选的“设计窍门”之一。它设置一个与我们的BlurTexture度数相匹配的大小的视口。
//然后它被弹簧的着色程序调用。弹簧将由于视口被拉伸适应128*128的纹理。
//在弹簧被拉伸至128x128视口大小之后，我们约定BlurTexture 且用glCopyTexImage2D从视口拷贝色彩缓冲器到BlurTexture。
//参数如下：
//GL_TEXTURE_2D指出我们正使用一个2Dimensional纹理，0是我们想要拷贝缓冲器到mip的绘图等级，默认等级是0。
//GL_LUMINANCE指出被拷贝的数据格式。我之所以使用GL_LUMINANCE因为最终结果看起来比较好。这种情形缓冲器的亮度部分将被拷贝到纹理。
//其它参数可以是GL_ALPHA, GL_RGB, GL_INTENSITY等等。
//其次的两个参数告诉OpenGL从（0，0）开始拷贝到哪里。宽度和高度(128,128)是从左到右有多少象素要拷贝并且上下拷贝多少。
//最后一个参数仅用来指出我们是否想要一个边界——哪个不想要。
//既然在我们的BlurTexture我们已经有了一个色彩缓冲器的副本（和被拉伸的弹簧一致），我们可以清除那个缓冲器，
//向后设置那个视口到适当的度数（640x480全屏）。
//重要：
//这个窍门能用在只有双缓冲器象素格式的情况下。原因是所有这些操作从观察者面前被隐藏起来。（在后缓冲器完成）。

// 着色到一个纹理
void MyGLWidget::renderToTexture()
{
  glViewport(0,0,128,128);	// 设置我们的视口
  processHelix();// 着色螺旋
  glBindTexture(GL_TEXTURE_2D, m_blurTexture);	// 绑定模糊纹理
  // 拷贝我们的视口到模糊纹理 (从 0,0 到 128,128... 无边界)
  glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 0, 0, 128, 128, 0);
  glClearColor(0.0f, 0.0f, 0.5f, 0.5);//调整清晰的色彩到中等蓝色
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// 清屏和深度缓冲
  glViewport(0 , 0, 640 ,480);// 调整视口 (0,0 to 640x480)
}

//DrawBlur函数仅在我们的3D场景前绘制一些混合的方块——用BlurTexture我们以前已实现。
//这样，借由阿尔发和缩放这个纹理，我们得到了真正看起来象辐射状的模糊的效果。
//我首先禁用GEN_S 和 GEN_T（我沉溺于球体影射，因此我的程序通常启用这些指令：P）。
//我们启用2D纹理，禁用深度测试，调整正确的函数，起用混合然后约束BlurTexture。
//下一件我们要作的事情是转换到标准视图，那样比较容易绘制一些完美适应屏幕大小的方块。
//这是我们在3D实体顶端排列纹理的方法（通过拉伸纹理匹配屏幕比例）。这是问题二要解决的地方。

// 绘制模糊的图象
void MyGLWidget::drawBlur(int times, float inc)
{
    float spost = 0.0f;// 纹理坐标偏移量
    float alphainc = 0.9f / times;// alpha混合的衰减量
    float alpha = 0.2f;// Alpha初值
    // 禁用自动生成纹理坐标
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_2D);// 启用 2D 纹理映射
    glDisable(GL_DEPTH_TEST);// 深度测试不可用
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);// 设置混合模式
    glEnable(GL_BLEND);// 启用混合
    glBindTexture(GL_TEXTURE_2D,m_blurTexture);// 绑定混合纹理
    viewOrtho();// 切换到标准视图
    alphainc = alpha / times;// 减少alpha值
    //我们多次绘制这个纹理用于创建那个辐射效果， 缩放这个纹理坐标并且每次我们做另一个关口时增大混合因数 。
    //我们绘制25个方块，每次按照0.015f拉伸这个纹理。
    glBegin(GL_QUADS);// 开始绘制方块
        // 着色模糊物的次数
        for (int num = 0;num < times;num++)
        {
            glColor4f(1.0f, 1.0f, 1.0f, alpha);// 调整alpha值
            glTexCoord2f(0+spost,1-spost);
            glVertex2f(0,0);
            glTexCoord2f(0+spost,0+spost);
            glVertex2f(0,480);
            glTexCoord2f(1-spost,0+spost);
            glVertex2f(640,480);
            glTexCoord2f(1-spost,1-spost);
            glVertex2f(640,0);
            spost += inc;// 逐渐增加 spost (快速靠近纹理中心)
            alpha = alpha - alphainc;// 逐渐增加 alpha (逐渐淡出纹理)
        }
    glEnd();// 完成绘制方块
    viewPerspective();// 转换到一个透视视图
    glEnable(GL_DEPTH_TEST);// 深度测试可用
    glDisable(GL_TEXTURE_2D);// 2D纹理映射不可用
    glDisable(GL_BLEND);// 混合不可用
    glBindTexture(GL_TEXTURE_2D,0);// 释放模糊纹理
}

//我希望你满意这篇指南，它实在没有比透视一个纹理讲授更多其它内容，但它是一个干脆地添加到你的3D应用程序中有趣的效果。
//如果你有任何的注释建议或者如果你知道一种更好的方法执行这个效果联系我rio@spinningkids.org。
//我也想要委托你去做一列事情（家庭作业）：D
//1） 更改DrawBlur程序变为一个水平的模糊之物，垂直的模糊之物和一些更好的效果。（转动模糊之物！）。
//2） 玩转DrawBlur参数（添加，删除）变为一个好的程序和你的音乐同步。
//3） 用GL_LUMINANCE玩弄DrawBlur参数和一个SMALL纹理（惊人的光亮！）。
//4） 用暗色纹理代替亮色尝试大骗（哈哈，自己造的）测定体积的阴影。
//好了，这应该是所有的了（到此为止）。
//访问我的站点http://www.spinningkids.org/rio.
//获得更多的最新指南。
