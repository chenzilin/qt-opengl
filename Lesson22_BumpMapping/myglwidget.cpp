#include "myglwidget.h"

//凹凸映射，多重纹理扩展:
//这是一课高级教程，请确信你对基本知识已经非常了解了。这一课是基于第六课的代码的，它将建立一个非常酷的立体纹理效果。

//这一课由Jens Schneider所写，它基本上是由第6课改写而来的，在这一课里，你将学习：
//怎样控制多重纹理
//怎样创建一个“假”的凹凸映射
//怎样做一个标志，它看起来在你的场景上方
//怎样使矩阵变化更有效率
//基本的多通道渲染
//因为上面提到的很多方面是高级渲染得内容，我们在讲述的时候会先说明理论，接着在分析代码 。如果你已经熟悉了这些理论，你可以跳过他们，直接看代码。
//当你遇到什么问题的时候，不妨回过头来看看这些理论。
//最后这份代码超过了1200行，大部分我们在前面的教程中遇到过了。我不会解释每一行代码，只在重要的地方做些提示，好了，让我们开始吧。

//下面的函数定义用来使用OpenGL的扩展函数，你可以把PFN-who-ever-reads-this看成是预先定义的函数类型，
//因为我们不清楚是否能得到这些函数的实体，所以先把他们都设置为NULL。glMultiTexCoordifARB函数是glTexCoordif函数的扩展，
//它门的功能相似，其中i为纹理坐标的维数，f为数据的类型。最后两个函数用来激活纹理处理单元，可以使用特定的纹理单元来邦定纹理。
//顺便说一句，ARB是"Architectural Review Board"的缩写，用来定义这个组织提出的对OpenGL的扩展，并不强制OpenGL的实现必须包含这个功能，
//但他们希望这个功能得到广泛的支持。当前，只有multitexture被加入到ARB中，这从另一个方面支持multitexture的扩展将大大的提高渲染速度。
//MAX_EMBOSS常量定义了突起的最大值
#define MAX_EMBOSS (GLfloat)0.01f						// 定义了突起的最大值

//好了，现在我们准备使用GL_ARB_multitexture这个扩展，它非常简单。
//大部分图形卡不止一个纹理单元，为了利用这个功能，你必须检查GL_ARB_multitexture是否被支持，
//它可以使你同时把2个或多个不同的纹理映射到OpenGL图元上。开起来这个功能好像没有太大的作用，但当你使用多个纹理时，
//如果能同时把这些纹理值混合，而不使用费时的乘法运算，你将会得到很高的速度提高。
//现在回到我们的代码，__ARB_ENABLE用来设置是否使用ARB扩展。如果你想看你的OpenGL扩展，只要把#define EXT_INFO前的注释去掉就行了。
//接着，我们在运行检查我们的扩展，以保证我们的程序可以在不同的系统上运行。所以我们需要一些内存保存扩展名的字符串，他们是下面两行。
//接着我们用一个变量multitextureSupported来标志当前系统是否能使用multitexture扩展，并用maxTexelUnits记录运行系统的纹理单元，
//这个值最少是1。
#define __ARB_ENABLE true					// 使用它设置是否使用ARB扩展
#define EXT_INFO							// 把注释去掉,可以在启动时查看你的扩展
#define MAX_EXTENSION_SPACE 10240			// 保存扩展字符
#define MAX_EXTENSION_LENGTH 256			// 每个扩展字符串最大的长度


PFNGLMULTITEXCOORD1FARBPROC	glMultiTexCoord1fARB	= NULL;
PFNGLMULTITEXCOORD2FARBPROC	glMultiTexCoord2fARB	= NULL;
PFNGLMULTITEXCOORD3FARBPROC	glMultiTexCoord3fARB	= NULL;
PFNGLMULTITEXCOORD4FARBPROC	glMultiTexCoord4fARB	= NULL;
PFNGLACTIVETEXTUREARBPROC	glActiveTextureARB	= NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC	glClientActiveTextureARB= NULL;

GLfloat LightAmbient[]	= { 0.2f, 0.2f, 0.2f};					// 环境光
GLfloat LightDiffuse[]	= { 1.0f, 1.0f, 1.0f};					// 漫射光
GLfloat LightPosition[]	= { 0.0f, 0.0f, 2.0f};					// 光源位置
GLfloat Gray[]		= { 0.5f, 0.5f, 0.5f, 1.0f};

//下面一块代码用来保存立方体的纹理和坐标，每5个数字描述一个顶点，包含2D的纹理坐标和3D的顶点坐标。
GLfloat cubeData[]=
{
    // 前面
    0.0f, 0.0f,		-1.0f, -1.0f, +1.0f,
    1.0f, 0.0f,		+1.0f, -1.0f, +1.0f,
    1.0f, 1.0f,		+1.0f, +1.0f, +1.0f,
    0.0f, 1.0f,		-1.0f, +1.0f, +1.0f,
    // 背面
    1.0f, 0.0f,		-1.0f, -1.0f, -1.0f,
    1.0f, 1.0f,		-1.0f, +1.0f, -1.0f,
    0.0f, 1.0f,		+1.0f, +1.0f, -1.0f,
    0.0f, 0.0f,		+1.0f, -1.0f, -1.0f,
    // 上面
    0.0f, 1.0f,		-1.0f, +1.0f, -1.0f,
    0.0f, 0.0f,		-1.0f, +1.0f, +1.0f,
    1.0f, 0.0f,		+1.0f, +1.0f, +1.0f,
    1.0f, 1.0f,		+1.0f, +1.0f, -1.0f,
    // 下面
    1.0f, 1.0f,		-1.0f, -1.0f, -1.0f,
    0.0f, 1.0f,		+1.0f, -1.0f, -1.0f,
    0.0f, 0.0f,		+1.0f, -1.0f, +1.0f,
    1.0f, 0.0f,		-1.0f, -1.0f, +1.0f,
    // 右面
    1.0f, 0.0f,		+1.0f, -1.0f, -1.0f,
    1.0f, 1.0f,		+1.0f, +1.0f, -1.0f,
    0.0f, 1.0f,		+1.0f, +1.0f, +1.0f,
    0.0f, 0.0f,		+1.0f, -1.0f, +1.0f,
    // 左面
    0.0f, 0.0f,		-1.0f, -1.0f, -1.0f,
    1.0f, 0.0f,		-1.0f, -1.0f, +1.0f,
    1.0f, 1.0f,		-1.0f, +1.0f, +1.0f,
    0.0f, 1.0f,		-1.0f, +1.0f, -1.0f
};

//下一部分代码，用来这运行时确定是否支持多重纹理的扩展。
//首先，我们假定一个字符串包含了所有的扩展名，各个扩展名之间用'\n'分开。我们所要做的就是在其中查找是否有我们需要的扩展。
//如果成功找到则返回TRUE，否则返回FALSE。
bool isInString(char *string, const char *search)
{
    int pos=0;
    int maxpos=strlen(search)-1;
    int len=strlen(string);
    for (int i=0; i<len; i++)
    {
        if ((i==0) || ((i>1) && string[i-1]=='\n'))
        {
            // 新的扩展名开始与这里
            pos=0;							// 开始新的比较
            while (string[i]!='\n')
            {					// 比较整个扩展名
                if (string[i]==search[pos])
                {
                    pos++;			// 下一个字符
                }
                if ((pos > maxpos) && string[i+1]=='\n')
                {
                    return true;	// 如果整个扩展名相同则成功返回
                }
                i++;
            }
        }
    }
    return false;								// 没找到
}

//凹凸贴图
//Michael I. Gold
//NVIDIA 公司

//凹凸贴图

//真实的凹凸贴图是逐像素计算的

//光照计算是按每个象素点的法向量计算的
//巨大的计算量
//更多的信息可以看: Blinn, J. Simulation of Wrinkled Surfaces. Computer Graphics. 12, 3 (August 1978), 286-292
//凹凸贴图是在效果和精度之间的一个折中

//只能对漫射光计算，不能使用反射光
//欺骗视觉的采样
//可能运行于当前的硬件上
//如果它看起来很好，就干吧
//漫射光的计算

//C = (L*N)*Dl*Dm

//L 顶点到灯之间的单位向量
//N 顶点的单位法向量
//Dl 灯光的漫射光颜色
//Dm 顶点材质的漫射属性
//凸值 逐像素改变N值
//凹凸映射 改变（L*N）的值
//近似的漫射因子 L*N

//纹理图代表高度场

//[0,1] 之间的高度代表凹凸方程
//首先导出偏离度m
//m 增加/减少基础的漫射因子Fd
//(Fd+m) 在每一像素上近似等于 (L*N)
//偏移量m的导出

//偏移量m的近似导出

//查找(s,t)纹理的高度H0
//查找(s+ds, t+dt)纹理的高度H1
//M近似等于H1-H0
//计算凹凸量
//1) 原始凸起(H0).
//2) 原始的凸起(H0)向光源移动一小段距离形成第二个凸起(H1)
//3) 用H1凸起减去H0凸起 (H1-H0)
//计算灯光亮度

//计算片断的颜色Cf

//Cf = (L*N) x Dl x Dm
//(L*N) ~ (Fd + (H1-H0))
//Ct= Dm x Dl
//Cf = (Fd + (H0-H1) x Ct
//Fd等于顶点法线与灯光的向量的乘积
//上面就是全部么? 太简单了!

//我们还没有完成所有的任务，还必须做以下内容:
//创建一个纹理
//计算纹理坐标偏移量ds, dt
//计算漫射因子Fd
//ds, dt ，Fd都从N和L导出
//现在我们开始做一些数学计算
//创建纹理

//保存纹理!
//当前的多重纹理硬件只支持两个纹理
//偏移值保存在alpha通道里
//最大凸起值为 = 1.0
//水平面值为 = 0.5
//最小值为= 0.0
//颜色存储在RGB通道中
//设置内部颜色格式为RGBA8 !!
//计算纹理偏移量

//把灯光方向向量变换到一个笛卡尔坐标系中
//顶点法线为z轴
//从法线和视口的“上”向量导出坐标系
//顶点法线为z轴
//叉乘得到X轴
//丢弃“上”向量，利用z，y轴导出x轴
//创建3x3变换矩阵Mn
//变换灯光方向向量到这个坐标系中
//计算纹理偏移量

//使用法向坐标系中的向量作为偏移量
//L'= Mn x L
//使用L’.x, L’.y 作为 ds, dt
//使用 L’.z 作为漫射因子!
//如果灯光方向接近垂直，则L’.x, L’.y 非常小
//如果灯光方向接近水平，则L’.x, L’.y 非常大
//L’.z小于零的含义?
//灯光在法线的对面
//在TNT上的实现

//计算向量，纹理坐标
//设置漫射因子
//从纹理单元0取出表面颜色和H0值
//从纹理单元1取出H1值
//ARB_multitexture 扩展
//混合纹理扩展 (TBD)
//混合0 alpha设置:
//(1-T0a) + T1a - 0.5
//T1a-T0a 映射到[-1,1],但硬件把它映射到[0，1]
//T1a为H1的值，T0a为H0的值
//0.5 平衡损失的掐除值
//使用漫射光颜色调制（相乘）片断颜色T0c
//混合1 颜色设置：
//(T0c * C0a + T0c * Fda - 0.5 )*2
//0.5 平衡损失的掐除值
//乘以2加亮图像颜色
//结束理论讲解（凹凸映射）
//虽然我们做了一些改动，使得这个程序的实现与TNT的实现不一样，但它能工作与各种不同的显卡上。在这里我们将学到两三件事，
//凹凸映射在大多数显卡上是一个多通道算法（在TNT系列，可以使用一个2纹理通道实现），现在你应该能想到多重纹理的好处了吧。
//我们将使用一个三通道非多重纹理的算法实现，这个算法可以被改写为使用一个2纹理通道实现的算法。
//现在必须告诉你，我们将要做一些矩阵和向量的乘法，但那没有什么可担心的，所有的矩阵和向量都使用齐次坐标。
// 计算向量v=v*M（左乘）
void VMatMult(GLfloat *M, GLfloat *v)
{
    GLfloat res[3];
    res[0]=M[ 0]*v[0]+M[ 1]*v[1]+M[ 2]*v[2]+M[ 3]*v[3];
    res[1]=M[ 4]*v[0]+M[ 5]*v[1]+M[ 6]*v[2]+M[ 7]*v[3];
    res[2]=M[ 8]*v[0]+M[ 9]*v[1]+M[10]*v[2]+M[11]*v[3];
    v[0]=res[0];
    v[1]=res[1];
    v[2]=res[2];
    v[3]=M[15];
}

//开始理论讲解（凹凸映射）
//开始，让我们看看它的算法
//所有的向量必须在物体空间或则世界空间中
//计算向量v，由灯的位置减去当前顶点的位置
//归一化向量v
//把向量v投影到切空间中
//安向量v在切空间中的投影偏移纹理坐标
//这看起来不错，它基本上和Michael I. Gold介绍的方法差不多。但它有一个缺点，它只对xy平面进行投影，这对我们的应用还是不够的。
//但这个实现在计算漫射光的方法和我们是一样的，我们不能存储漫射因子，所以我们不能使用Michael I. Gold介绍的方法，
//因为我们想让它在任何显卡上运行而不仅仅是TNT系列。为什么不光照计算留到最后呢？这在简单的几何体绘制上是可行的，
//如果你需要渲染几千个具有凹凸贴图的三角形，你会感到绘制的速度不够快，到那时你需要改变这种渲染过程，寻找其它的方法。
//在我们的实现里，它看起来和上面的实现差不多，除了投影部分，我们将使用我们自己的近似。

//我们使用模型坐标，这种设定可以使得灯光位置相对于物体不变。
//我们计算当前的顶点坐标
//接着计算法线，并使它单位化
//创建一个正投影矩阵，把灯光方向变为切空间
//计算纹理坐标的偏移量，ds = s点乘v*MAX_EMBOSS, dt=t点乘v*MAX_EMBOSS
//在通道2中，把偏移量添加到纹理坐标
//为什么更好:
//更快
//看起来好看
//这个方法可以工作与各种表面
//可以运行于各种显卡
//最大化的兼容
//缺陷:
//并不是完全的物理模拟
//残留一些人为的假相

// 设置纹理偏移，都为单位长度
// n : 表面的法向量
// c : 当前的顶点纹理坐标，返回纹理坐标的偏移量
// l : 灯光的位置
// s : s方向
// t : t方向
void SetUpBumps(GLfloat *n, GLfloat *c, GLfloat *l, GLfloat *s, GLfloat *t)
{
    GLfloat v[3];								// 灯光方向
    GLfloat lenQ;								// 灯光方向向量的长度，使用它来单位化
    // 计算灯光方向
    v[0]=l[0]-c[0];
    v[1]=l[1]-c[1];
    v[2]=l[2]-c[2];
    lenQ=(GLfloat) sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    v[0]/=lenQ;
    v[1]/=lenQ;
    v[2]/=lenQ;
    // 把方向向量投影到s,t方向
    c[0]=(s[0]*v[0]+s[1]*v[1]+s[2]*v[2])*MAX_EMBOSS;
    c[1]=(t[0]*v[0]+t[1]*v[1]+t[2]*v[2])*MAX_EMBOSS;
}
//那看起来复杂么，但为了理解这个效果理论是必须的。（我在写这篇教程的时候也学习了它）。
//我在程序运行的时候，总喜欢在屏幕上显示标志，现在我们有了两个，使用doLogo函数创建它。
//下面的函数显示两个标志：一个OpenGL的标志，一个多重纹理的标志，如果可以使用多重纹理，则标志使用alpha混合，并看起来半透明。
//为了让它在屏幕的边沿显示我们使用混合并禁用光照和深度测试。

// map from Qt's ARGB endianness-dependent format to GL's big-endian RGBA layout
void qgl_byteSwapImage(QImage &img, GLenum pixel_type)
{
    const int width = img.width();
    const int height = img.height();

    if (pixel_type == GL_UNSIGNED_INT_8_8_8_8_REV
        || (pixel_type == GL_UNSIGNED_BYTE && QSysInfo::ByteOrder == QSysInfo::LittleEndian))
    {
        for (int i = 0; i < height; ++i) {
            uint *p = (uint *) img.scanLine(i);
            for (int x = 0; x < width; ++x)
                p[x] = ((p[x] << 16) & 0xff0000) | ((p[x] >> 16) & 0xff) | (p[x] & 0xff00ff00);
        }
    } else {
        for (int i = 0; i < height; ++i) {
            uint *p = (uint *) img.scanLine(i);
            for (int x = 0; x < width; ++x)
                p[x] = (p[x] << 8) | ((p[x] >> 24) & 0xff);
        }
    }
}

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_z(-5.0f), m_xrot(0.0f), m_yrot(0.0f),
    m_xspeed(0.0f), m_yspeed(0.0f), m_multitextureSupported(false), m_useMultitexture(true),
    m_emboss(false), m_bumps(false), m_maxTexelUnits(1), m_filter(1)
{
    showNormal();
    startTimer(50);
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(3, &m_texture[0]);
    glDeleteTextures(3, &m_bump[0]);
    glDeleteTextures(3, &m_invbump[0]);
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
    m_multitextureSupported = initMultitexture();

    initLights();

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

//所有的绘制函数都已经完成，接下来只要在绘制函数中调用即可
void MyGLWidget::paintGL()
{
    if (m_bumps)
    {
        if (m_useMultitexture && m_maxTexelUnits>1)
        {
            doMesh2TexelUnits();
        }
        else
        {
            doMesh1TexelUnits();
        }
    }
    else
    {
        doMeshNoBumps();
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
            m_xspeed-=0.01f;
            break;
        }
        case Qt::Key_Down:
        {
            m_xspeed+=0.01f;
            break;
        }
        case Qt::Key_Right:
        {
            m_yspeed+=0.01f;
            break;
        }
        case Qt::Key_Left:
        {
            m_yspeed-=0.01f;
            break;
        }
        case Qt::Key_PageUp:
        {
            m_z-=0.02f;
            break;
        }
        case Qt::Key_PageDown:
        {
            m_z+=0.02f;
            break;
        }
        case Qt::Key_B:
        {
            m_bumps = !m_bumps;
            break;
        }
        case Qt::Key_F:
        {
            m_filter++;
            m_filter%=3;
            break;
        }
        case Qt::Key_M:
        {
            m_useMultitexture = ((!m_useMultitexture) && m_multitextureSupported);
            break;
        }
        case Qt::Key_E:
        {
            m_emboss = !m_emboss;
            break;
        }
    }
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    m_xrot+=m_xspeed;
    m_yrot+=m_yspeed;
    if (m_xrot>360.0f)
    {
        m_xrot-=360.0f;
    }
    if (m_xrot<0.0f)
    {
        m_xrot+=360.0f;
    }
    if (m_yrot>360.0f)
    {
        m_yrot-=360.0f;
    }
    if (m_yrot<0.0f)
    {
        m_yrot+=360.0f;
    }
    updateGL();
    QGLWidget::timerEvent(event);
}

void MyGLWidget::loadGLTexture()
{
    QImage baseImage(":/image/Base.bmp");
    baseImage = baseImage.convertToFormat(QImage::Format_RGB888);
    baseImage = baseImage.mirrored();
    glGenTextures(3, &m_texture[0]);

    // 创建使用临近过滤器过滤得纹理
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, baseImage.width(),
                 baseImage.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, baseImage.bits());

    // 创建使用线形过滤器过滤得纹理
    glBindTexture(GL_TEXTURE_2D, m_texture[1]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, baseImage.width(),
                 baseImage.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, baseImage.bits());

    // 创建使用线形Mipmap过滤器过滤得纹理
    glBindTexture(GL_TEXTURE_2D, m_texture[2]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, baseImage.width(),baseImage.height(),
                      GL_RGB, GL_UNSIGNED_BYTE, baseImage.bits());

    //现在我们加载凹凸映射纹理。这个纹理必须使用50%的亮度（原因我们在后面介绍），我们使用glPixelTransferf函数完成这个功能。
    //另一个限制是我们不希望纹理重复贴图，只希望它粘贴一次，从纹理坐标(0,0)-(1,1)，所有大于它的纹理坐标都被映射到边缘，为了完成这个功能，
    //我们使用glTexParameteri函数。
    QImage bumpImage(":/image/Bump.bmp");
    bumpImage = bumpImage.convertToFormat(QImage::Format_RGB888);
    bumpImage = bumpImage.mirrored();

    glPixelTransferf(GL_RED_SCALE,0.5f);					// 把颜色值变为原来的50%
    glPixelTransferf(GL_GREEN_SCALE,0.5f);
    glPixelTransferf(GL_BLUE_SCALE,0.5f);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);		//不使用重复贴图
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
    glGenTextures(3, &m_bump[0]);
    // 创建使用临近过滤器过滤得纹理
    glBindTexture(GL_TEXTURE_2D, m_bump[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bumpImage.width(), bumpImage.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, bumpImage.bits());
    // 创建使用线形过滤器过滤得纹理
    glBindTexture(GL_TEXTURE_2D, m_bump[1]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bumpImage.width(), bumpImage.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, bumpImage.bits());
    // 创建使用线形Mipmap过滤器过滤得纹理
    glBindTexture(GL_TEXTURE_2D, m_bump[2]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, bumpImage.width(), bumpImage.height(),
                      GL_RGB, GL_UNSIGNED_BYTE, bumpImage.bits());
    //反转凹凸贴图数据，创建三个反转的凹凸贴图纹理
    bumpImage.invertPixels();
    bumpImage.save("bumpImage.bmp");
    glGenTextures(3, &m_invbump[0]);						// 创建三个反转了凹凸贴图
    // 创建使用临近过滤器过滤得纹理
    glBindTexture(GL_TEXTURE_2D, m_invbump[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bumpImage.width(), bumpImage.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, bumpImage.bits());
    // 创建使用线形过滤器过滤得纹理
    glBindTexture(GL_TEXTURE_2D, m_invbump[1]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bumpImage.width(), bumpImage.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, bumpImage.bits());
    // 创建使用线形Mipmap过滤器过滤得纹理
    glBindTexture(GL_TEXTURE_2D, m_invbump[2]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, bumpImage.width(), bumpImage.height(),
                      GL_RGB, GL_UNSIGNED_BYTE, bumpImage.bits());

    //载入标志图像，图像是把颜色和alpha通道存为两张不同的bmp位图的，所以在处理的时候需要注意以下各个分量的位置。
    QImage openglAlphaLogo(":/image/OpenGL_Alpha.bmp");
    openglAlphaLogo = openglAlphaLogo.mirrored();
    QImage openglLogo(":/image/OpenGL.bmp");
    openglLogo = openglLogo.convertToFormat(QImage::Format_RGB888);
    openglLogo = openglLogo.mirrored();
    uchar *newOpenGLLogoData = new uchar[openglLogo.byteCount() + openglLogo.byteCount()/3];
    for(int i = 0, iend = openglLogo.byteCount()/3; i < iend ;i++)
    {
        newOpenGLLogoData[i*4] = openglLogo.bits()[i*3];
        newOpenGLLogoData[i*4+1] = openglLogo.bits()[i*3+1];
        newOpenGLLogoData[i*4+2] = openglLogo.bits()[i*3+2];
    }
    for(int i = 0, iend = openglAlphaLogo.byteCount(); i < iend ;i++)
    {
        newOpenGLLogoData[i*4+3] = openglAlphaLogo.bits()[i];
    }
    glGenTextures(1, &m_glLogo);						// 创建标志纹理
    // 使用线形过滤器
    glBindTexture(GL_TEXTURE_2D, m_glLogo);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, openglLogo.width(),
                 openglLogo.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 newOpenGLLogoData);
    delete newOpenGLLogoData;

    // 载入扩展标志纹理
    QImage multiAlphaLogo(":/image/Multi_On_Alpha.bmp");
    multiAlphaLogo = multiAlphaLogo.mirrored();
    QImage multiLogo(":/image/Multi_On.bmp");
    multiLogo = multiLogo.convertToFormat(QImage::Format_RGB888);
    multiLogo = multiLogo.mirrored();
    uchar *newMultiLogoData = new uchar[multiLogo.byteCount() + multiLogo.byteCount()/3];
    for(int i = 0, iend = multiLogo.byteCount()/3; i < iend ;i++)
    {
        newMultiLogoData[i*4] = multiLogo.bits()[i*3];
        newMultiLogoData[i*4+1] = multiLogo.bits()[i*3+1];
        newMultiLogoData[i*4+2] = multiLogo.bits()[i*3+2];
    }
    for(int i = 0, iend = multiAlphaLogo.byteCount(); i < iend ;i++)
    {
        newMultiLogoData[i*4+3] = multiAlphaLogo.bits()[i];
    }
    glGenTextures(1, &m_multiLogo);						// 创建标志纹理
    // 使用线形过滤器
    glBindTexture(GL_TEXTURE_2D, m_multiLogo);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, multiLogo.width(),
                 multiLogo.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 newMultiLogoData);
    delete newMultiLogoData;

}

//接下来是绘制一个立方体的函数，它使用常规的方法绘制。
void MyGLWidget::doCube ()
{
    int i;
    glBegin(GL_QUADS);
        // 前面
        glNormal3f( 0.0f, 0.0f, +1.0f);
        for (i=0; i<4; i++)
        {
            glTexCoord2f(cubeData[5*i],cubeData[5*i+1]);
            glVertex3f(cubeData[5*i+2],cubeData[5*i+3],cubeData[5*i+4]);

        }
        // 后面
        glNormal3f( 0.0f, 0.0f,-1.0f);
        for (i=4; i<8; i++)
        {
            glTexCoord2f(cubeData[5*i],cubeData[5*i+1]);
            glVertex3f(cubeData[5*i+2],cubeData[5*i+3],cubeData[5*i+4]);
        }
        // 上面
        glNormal3f( 0.0f, 1.0f, 0.0f);
        for (i=8; i<12; i++)
        {
            glTexCoord2f(cubeData[5*i],cubeData[5*i+1]);
            glVertex3f(cubeData[5*i+2],cubeData[5*i+3],cubeData[5*i+4]);
        }
        // 下面
        glNormal3f( 0.0f,-1.0f, 0.0f);
        for (i=12; i<16; i++)
        {
            glTexCoord2f(cubeData[5*i],cubeData[5*i+1]);
            glVertex3f(cubeData[5*i+2],cubeData[5*i+3],cubeData[5*i+4]);
        }
        // 右面
        glNormal3f( 1.0f, 0.0f, 0.0f);
        for (i=16; i<20; i++)
        {
            glTexCoord2f(cubeData[5*i],cubeData[5*i+1]);
            glVertex3f(cubeData[5*i+2],cubeData[5*i+3],cubeData[5*i+4]);
        }
        // 左面
        glNormal3f(-1.0f, 0.0f, 0.0f);
        for (i=20; i<24; i++)
        {
            glTexCoord2f(cubeData[5*i],cubeData[5*i+1]);
            glVertex3f(cubeData[5*i+2],cubeData[5*i+3],cubeData[5*i+4]);
        }
    glEnd();
}

void MyGLWidget::doLogo()
{
    // 必须最后在调用这个函数，以公告板的形式显示两个标志
    glDepthFunc(GL_ALWAYS);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glLoadIdentity();
    glBindTexture(GL_TEXTURE_2D, m_glLogo);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f,0.0f);	glVertex3f(0.23f, -0.4f,-1.0f);
        glTexCoord2f(1.0f,0.0f);	glVertex3f(0.53f, -0.4f,-1.0f);
        glTexCoord2f(1.0f,1.0f);	glVertex3f(0.53f, -0.25f,-1.0f);
        glTexCoord2f(0.0f,1.0f);	glVertex3f(0.23f, -0.25f,-1.0f);
    glEnd();
    if (m_useMultitexture)
    {
        glBindTexture(GL_TEXTURE_2D, m_multiLogo);
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f,0.0f);	glVertex3f(-0.53f, -0.25f,-1.0f);
            glTexCoord2f(1.0f,0.0f);	glVertex3f(-0.33f, -0.25f,-1.0f);
            glTexCoord2f(1.0f,1.0f);	glVertex3f(-0.33f, -0.15f,-1.0f);
            glTexCoord2f(0.0f,1.0f);	glVertex3f(-0.53f, -0.15f,-1.0f);
        glEnd();
    }
    glDepthFunc(GL_LEQUAL);
}

//现在到了绘制凹凸贴图的函数了，我们先来看看不使用多重映射的方法，它通过三个通道实现。
//在第一步，我们先取得模型变换矩阵的逆矩阵！
void MyGLWidget::doMesh1TexelUnits()
{
    GLfloat c[4]={0.0f,0.0f,0.0f,1.0f};					// 保存当前的顶点
    GLfloat n[4]={0.0f,0.0f,0.0f,1.0f};					// 保存法线
    GLfloat s[4]={0.0f,0.0f,0.0f,1.0f};					// s纹理坐标方向
    GLfloat t[4]={0.0f,0.0f,0.0f,1.0f};					// t纹理坐标方向
    GLfloat l[4];                                       // 保存灯光方向
    GLfloat Minv[16];                                   // 保存模型变换矩阵的逆
    int i;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);			// 清空背景颜色和深度缓存
    // 创建模型变换矩阵的逆
    glLoadIdentity();
    glRotatef(-m_yrot,0.0f,1.0f,0.0f);
    glRotatef(-m_xrot,1.0f,0.0f,0.0f);
    glTranslatef(0.0f,0.0f,-m_z);
    glGetFloatv(GL_MODELVIEW_MATRIX,Minv);
    glLoadIdentity();
    glTranslatef(0.0f,0.0f,m_z);
    glRotatef(m_xrot,1.0f,0.0f,0.0f);
    glRotatef(m_yrot,0.0f,1.0f,0.0f);
    // 设置灯光的位置
    l[0]=LightPosition[0];
    l[1]=LightPosition[1];
    l[2]=LightPosition[2];
    l[3]=1.0f;
    VMatMult(Minv,l);
//  通道1:
//  使用凹凸纹理
//  禁止混合
//  禁止光照
//  使用无偏移的纹理坐标
//  绘制几何体
//  这将渲染一个无凹凸贴图的几何体
    glBindTexture(GL_TEXTURE_2D, m_bump[m_filter]);
    glDisable(GL_BLEND);
    glDisable(GL_LIGHTING);
    doCube();
//  通道2:
//  使用反转的纹理凹凸贴图
//  设置混合因子为1，1
//  使用光照
//  使用偏移纹理坐标
//  绘制几何体
//  这将绘制一个具有凹凸贴图的几何体，但没有颜色
    glBindTexture(GL_TEXTURE_2D,m_invbump[m_filter]);
    glBlendFunc(GL_ONE,GL_ONE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBegin(GL_QUADS);
    // 前面
        n[0]=0.0f;
        n[1]=0.0f;
        n[2]=1.0f;
        s[0]=1.0f;
        s[1]=0.0f;
        s[2]=0.0f;
        t[0]=0.0f;
        t[1]=1.0f;
        t[2]=0.0f;
        for (i=0; i<4; i++)
        {
            c[0]=cubeData[5*i+2];
            c[1]=cubeData[5*i+3];
            c[2]=cubeData[5*i+4];
            SetUpBumps(n,c,l,s,t);
            // 设置纹理坐标为偏移后的纹理坐标
            glTexCoord2f(cubeData[5*i]+c[0], cubeData[5*i+1]+c[1]);
            glVertex3f(cubeData[5*i+2], cubeData[5*i+3], cubeData[5*i+4]);
        }
        // 后面
        n[0]=0.0f;
        n[1]=0.0f;
        n[2]=-1.0f;
        s[0]=-1.0f;
        s[1]=0.0f;
        s[2]=0.0f;
        t[0]=0.0f;
        t[1]=1.0f;
        t[2]=0.0f;
        for (i=4; i<8; i++)
        {
            c[0]=cubeData[5*i+2];
            c[1]=cubeData[5*i+3];
            c[2]=cubeData[5*i+4];
            SetUpBumps(n,c,l,s,t);
            glTexCoord2f(cubeData[5*i]+c[0], cubeData[5*i+1]+c[1]);
            glVertex3f(cubeData[5*i+2], cubeData[5*i+3], cubeData[5*i+4]);
        }
        // 上面
        n[0]=0.0f;
        n[1]=1.0f;
        n[2]=0.0f;
        s[0]=1.0f;
        s[1]=0.0f;
        s[2]=0.0f;
        t[0]=0.0f;
        t[1]=0.0f;
        t[2]=-1.0f;
        for (i=8; i<12; i++)
        {
            c[0]=cubeData[5*i+2];
            c[1]=cubeData[5*i+3];
            c[2]=cubeData[5*i+4];
            SetUpBumps(n,c,l,s,t);
            glTexCoord2f(cubeData[5*i]+c[0], cubeData[5*i+1]+c[1]);
            glVertex3f(cubeData[5*i+2], cubeData[5*i+3], cubeData[5*i+4]);
        }
        // 下面
        n[0]=0.0f;
        n[1]=-1.0f;
        n[2]=0.0f;
        s[0]=-1.0f;
        s[1]=0.0f;
        s[2]=0.0f;
        t[0]=0.0f;
        t[1]=0.0f;
        t[2]=-1.0f;
        for (i=12; i<16; i++)
        {
            c[0]=cubeData[5*i+2];
            c[1]=cubeData[5*i+3];
            c[2]=cubeData[5*i+4];
            SetUpBumps(n,c,l,s,t);
            glTexCoord2f(cubeData[5*i]+c[0], cubeData[5*i+1]+c[1]);
            glVertex3f(cubeData[5*i+2], cubeData[5*i+3], cubeData[5*i+4]);
        }
        // 右面
        n[0]=1.0f;
        n[1]=0.0f;
        n[2]=0.0f;
        s[0]=0.0f;
        s[1]=0.0f;
        s[2]=-1.0f;
        t[0]=0.0f;
        t[1]=1.0f;
        t[2]=0.0f;
        for (i=16; i<20; i++)
        {
            c[0]=cubeData[5*i+2];
            c[1]=cubeData[5*i+3];
            c[2]=cubeData[5*i+4];
            SetUpBumps(n,c,l,s,t);
            glTexCoord2f(cubeData[5*i]+c[0], cubeData[5*i+1]+c[1]);
            glVertex3f(cubeData[5*i+2], cubeData[5*i+3], cubeData[5*i+4]);
        }
        // 左面
        n[0]=-1.0f;
        n[1]=0.0f;
        n[2]=0.0f;
        s[0]=0.0f;
        s[1]=0.0f;
        s[2]=1.0f;
        t[0]=0.0f;
        t[1]=1.0f;
        t[2]=0.0f;
        for (i=20; i<24; i++)
        {
            c[0]=cubeData[5*i+2];
            c[1]=cubeData[5*i+3];
            c[2]=cubeData[5*i+4];
            SetUpBumps(n,c,l,s,t);
            glTexCoord2f(cubeData[5*i]+c[0], cubeData[5*i+1]+c[1]);
            glVertex3f(cubeData[5*i+2], cubeData[5*i+3], cubeData[5*i+4]);
        }
    glEnd();
    //通道3:
    //使用颜色纹理Use (colored) base-texture
    //使用混合因子GL_DST_COLOR, GL_SRC_COLOR
    //这个混合等于把颜色值乘以2
    //使用光照
    //绘制几何体
    //这个过程将结束立方体的渲染，因为我们可以在是否使用多重渲染之间切换，所以必须把纹理环境参数设为GL_MODULATE，这是默认的值。
    if (!m_emboss)
    {
        glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glBindTexture(GL_TEXTURE_2D, m_texture[m_filter]);
        glBlendFunc(GL_DST_COLOR,GL_SRC_COLOR);
        glEnable(GL_LIGHTING);
        doCube();
    }
    //最后的通道:
    //绘制标志
    doLogo();
}

//这个函数将在多重纹理功能的支持下载两个通道中完成凹凸贴图的绘制，我们支持两个纹理单元，与一个纹理单元不同的是，
//我们给一个顶点设置两个纹理坐标。
void MyGLWidget::doMesh2TexelUnits()
{
    GLfloat c[4]={0.0f,0.0f,0.0f,1.0f};					// 保存当前的顶点
    GLfloat n[4]={0.0f,0.0f,0.0f,1.0f};					// 保存法线
    GLfloat s[4]={0.0f,0.0f,0.0f,1.0f};					// s纹理坐标方向
    GLfloat t[4]={0.0f,0.0f,0.0f,1.0f};					// t纹理坐标方向
    GLfloat l[4];							// 保存灯光方向
    GLfloat Minv[16];							// 保存模型变换矩阵的逆
    int i;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);			// 清空背景颜色和深度缓存
    // 创建模型变换矩阵的逆
    glLoadIdentity();
    glRotatef(-m_yrot,0.0f,1.0f,0.0f);
    glRotatef(-m_xrot,1.0f,0.0f,0.0f);
    glTranslatef(0.0f,0.0f,-m_z);
    glGetFloatv(GL_MODELVIEW_MATRIX,Minv);
    glLoadIdentity();
    glTranslatef(0.0f,0.0f,m_z);
    glRotatef(m_xrot,1.0f,0.0f,0.0f);
    glRotatef(m_yrot,0.0f,1.0f,0.0f);
    // 设置灯光的位置
    l[0]=LightPosition[0];
    l[1]=LightPosition[1];
    l[2]=LightPosition[2];
    l[3]=1.0f;
    VMatMult(Minv,l);
    //通道1:
    //无凹凸贴图
    //无光照
    //设置纹理混合器0
    //使用凹凸纹理
    //使用无偏移的纹理坐标
    //使用替换方式粘贴纹理
    //设置纹理混合器1
    //偏移纹理坐标
    //使用相加的纹理操作
    //这将绘制一个灰度的立方体
    // 纹理单元 #0
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_bump[m_filter]);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
    glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
    // 纹理单元 #1
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_invbump[m_filter]);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
    glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
    // 禁用混合和光照
    glDisable(GL_BLEND);
    glDisable(GL_LIGHTING);
    //现在按面一个一个的渲染立方体，和doMesh1TexelUnits函数中所作的操作差不多，只是用glMultiTexCoor2fARB替换glTexCoord2f，
    //在这个函数中，你必须把纹理坐标发向不同的纹理处理单元，可用的参数值为GL_TEXTUREi_ARB0到GL_TEXTUREi_ARB31。
    glBegin(GL_QUADS);
    // 前面
        n[0]=0.0f;
        n[1]=0.0f;
        n[2]=1.0f;
        s[0]=1.0f;
        s[1]=0.0f;
        s[2]=0.0f;
        t[0]=0.0f;
        t[1]=1.0f;
        t[2]=0.0f;
        for (i=0; i<4; i++)
        {
            c[0]=cubeData[5*i+2];
            c[1]=cubeData[5*i+3];
            c[2]=cubeData[5*i+4];
            SetUpBumps(n,c,l,s,t);
            glMultiTexCoord2fARB(GL_TEXTURE0_ARB,cubeData[5*i], cubeData[5*i+1]);
            glMultiTexCoord2fARB(GL_TEXTURE1_ARB,cubeData[5*i]+c[0], cubeData[5*i+1]+c[1]);
            glVertex3f(cubeData[5*i+2], cubeData[5*i+3], cubeData[5*i+4]);
        }
        // 后面
        n[0]=0.0f;
        n[1]=0.0f;
        n[2]=-1.0f;
        s[0]=-1.0f;
        s[1]=0.0f;
        s[2]=0.0f;
        t[0]=0.0f;
        t[1]=1.0f;
        t[2]=0.0f;
        for (i=4; i<8; i++)
        {
            c[0]=cubeData[5*i+2];
            c[1]=cubeData[5*i+3];
            c[2]=cubeData[5*i+4];
            SetUpBumps(n,c,l,s,t);
            glMultiTexCoord2fARB(GL_TEXTURE0_ARB,cubeData[5*i], cubeData[5*i+1]);
            glMultiTexCoord2fARB(GL_TEXTURE1_ARB,cubeData[5*i]+c[0], cubeData[5*i+1]+c[1]);
            glVertex3f(cubeData[5*i+2], cubeData[5*i+3], cubeData[5*i+4]);
        }
        // 上面
        n[0]=0.0f;
        n[1]=1.0f;
        n[2]=0.0f;
        s[0]=1.0f;
        s[1]=0.0f;
        s[2]=0.0f;
        t[0]=0.0f;
        t[1]=0.0f;
        t[2]=-1.0f;
        for (i=8; i<12; i++)
        {
            c[0]=cubeData[5*i+2];
            c[1]=cubeData[5*i+3];
            c[2]=cubeData[5*i+4];
            SetUpBumps(n,c,l,s,t);
            glMultiTexCoord2fARB(GL_TEXTURE0_ARB,cubeData[5*i], cubeData[5*i+1]);
            glMultiTexCoord2fARB(GL_TEXTURE1_ARB,cubeData[5*i]+c[0], cubeData[5*i+1]+c[1]);
            glVertex3f(cubeData[5*i+2], cubeData[5*i+3], cubeData[5*i+4]);
        }
        // 下面
        n[0]=0.0f;
        n[1]=-1.0f;
        n[2]=0.0f;
        s[0]=-1.0f;
        s[1]=0.0f;
        s[2]=0.0f;
        t[0]=0.0f;
        t[1]=0.0f;
        t[2]=-1.0f;
        for (i=12; i<16; i++)
        {
            c[0]=cubeData[5*i+2];
            c[1]=cubeData[5*i+3];
            c[2]=cubeData[5*i+4];
            SetUpBumps(n,c,l,s,t);
            glMultiTexCoord2fARB(GL_TEXTURE0_ARB,cubeData[5*i], cubeData[5*i+1]);
            glMultiTexCoord2fARB(GL_TEXTURE1_ARB,cubeData[5*i]+c[0], cubeData[5*i+1]+c[1]);
            glVertex3f(cubeData[5*i+2], cubeData[5*i+3], cubeData[5*i+4]);
        }
        // 右面
        n[0]=1.0f;
        n[1]=0.0f;
        n[2]=0.0f;
        s[0]=0.0f;
        s[1]=0.0f;
        s[2]=-1.0f;
        t[0]=0.0f;
        t[1]=1.0f;
        t[2]=0.0f;
        for (i=16; i<20; i++)
        {
            c[0]=cubeData[5*i+2];
            c[1]=cubeData[5*i+3];
            c[2]=cubeData[5*i+4];
            SetUpBumps(n,c,l,s,t);
            glMultiTexCoord2fARB(GL_TEXTURE0_ARB,cubeData[5*i], cubeData[5*i+1]);
            glMultiTexCoord2fARB(GL_TEXTURE1_ARB,cubeData[5*i]+c[0], cubeData[5*i+1]+c[1]);
            glVertex3f(cubeData[5*i+2], cubeData[5*i+3], cubeData[5*i+4]);
        }
        // 左面
        n[0]=-1.0f;
        n[1]=0.0f;
        n[2]=0.0f;
        s[0]=0.0f;
        s[1]=0.0f;
        s[2]=1.0f;
        t[0]=0.0f;
        t[1]=1.0f;
        t[2]=0.0f;
        for (i=20; i<24; i++)
        {
            c[0]=cubeData[5*i+2];
            c[1]=cubeData[5*i+3];
            c[2]=cubeData[5*i+4];
            SetUpBumps(n,c,l,s,t);
            glMultiTexCoord2fARB(GL_TEXTURE0_ARB,cubeData[5*i], cubeData[5*i+1]);
            glMultiTexCoord2fARB(GL_TEXTURE1_ARB,cubeData[5*i]+c[0], cubeData[5*i+1]+c[1]);
            glVertex3f(cubeData[5*i+2], cubeData[5*i+3], cubeData[5*i+4]);
        }
    glEnd();
    //通道2：
    //使用基本纹理
    //使用光照
    //使用普通的纹理混合操作
    //这将完成最后的凹凸贴图
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glDisable(GL_TEXTURE_2D);
    glActiveTextureARB(GL_TEXTURE0_ARB);
    if (!m_emboss)
    {
        glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glBindTexture(GL_TEXTURE_2D,m_texture[m_filter]);
        glBlendFunc(GL_DST_COLOR,GL_SRC_COLOR);
        glEnable(GL_BLEND);
        glEnable(GL_LIGHTING);
        doCube();
    }
    //最后的通道：
    //绘制标志
    doLogo();
}

//最后绘制一个无凹凸贴图的立方体，用来观察两者之间的效果
void MyGLWidget::doMeshNoBumps()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f,0.0f,m_z);
    glRotatef(m_xrot,1.0f,0.0f,0.0f);
    glRotatef(m_yrot,0.0f,1.0f,0.0f);
    if (m_useMultitexture)
    {
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glDisable(GL_TEXTURE_2D);
        glActiveTextureARB(GL_TEXTURE0_ARB);
    }
    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D,m_texture[m_filter]);
    glBlendFunc(GL_DST_COLOR,GL_SRC_COLOR);
    glEnable(GL_LIGHTING);
    doCube();
    doLogo();
}

bool MyGLWidget::initMultitexture()
{
    char *extensions;
    extensions=strdup((char *) glGetString(GL_EXTENSIONS));			// 返回扩展名字符串
    int len=strlen(extensions);
    for (int i=0; i<len; i++)							// 使用'\n'分割各个扩展名
    {
        if (extensions[i]==' ')
        {
            extensions[i]='\n';
        }
    }
#ifdef EXT_INFO
    qDebug() << "Supported GL extensions" << extensions;
#endif
    if (isInString(extensions,"GL_ARB_multitexture")				// 是否支持多重纹理扩展？
        && __ARB_ENABLE							// 是否使用多重纹理扩展？
        && isInString(extensions,"GL_EXT_texture_env_combine"))		// 是否支持纹理环境混合

    {
        glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &m_maxTexelUnits);
        glMultiTexCoord1fARB = (PFNGLMULTITEXCOORD1FARBPROC) wglGetProcAddress("glMultiTexCoord1fARB");
        glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC) wglGetProcAddress("glMultiTexCoord2fARB");
        glMultiTexCoord3fARB = (PFNGLMULTITEXCOORD3FARBPROC) wglGetProcAddress("glMultiTexCoord3fARB");
        glMultiTexCoord4fARB = (PFNGLMULTITEXCOORD4FARBPROC) wglGetProcAddress("glMultiTexCoord4fARB");
        glActiveTextureARB   = (PFNGLACTIVETEXTUREARBPROC) wglGetProcAddress("glActiveTextureARB");
        glClientActiveTextureARB= (PFNGLCLIENTACTIVETEXTUREARBPROC) wglGetProcAddress("glClientActiveTextureARB");
#ifdef EXT_INFO
        qDebug() << "The GL_ARB_multitexture be used";
#endif
        return true;
    }
    m_useMultitexture = false;							// 如果不支持多重纹理则返回false
    return false;
}

//初始化灯光
void MyGLWidget::initLights()
{
    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
    glEnable(GL_LIGHT1);
}

//现在你应该可以熟练的使用凹凸贴图了，如果你想让你的具有凹凸贴图的程序跑起来更快，你应该注意以下几点：
//你不应该使用256x256的纹理，这会让处理变得缓慢。
//一个具有凹凸贴图的立方体是不常见的，这和你的视角有关，因为三角面过于大了，如果要获得很好的视觉效果，你需要很大的纹理贴图，
//这必然会降低渲染速度。你可以把模型 创建为一些小的三角形，从而使用小的纹理，来获得好的效果。
//你应该先创建颜色纹理，接着把它转换为具有深度的凹凸纹理
//凹凸纹理应该锐化，这可以取得更好的效果，在你的图像处理程序中可以完成这个操作。
//凹凸贴图的值因该在50%灰度图上波动(RGB=127,127,127), 亮的值代表凸起，暗的值代表凹陷。
//凹凸贴图可以为纹理图大小的1/4，而不会影响外观效果。
//现在你应该对这篇文章中内容的大慨有了一个基本的认识，希望你读的愉快。
//如果你有任何纹理，请联系我或访问我的网站http://www.glhint.de
//我必须感谢以下的人：

//Michael I. Gold ，它写出了凹凸贴图的原理
//Diego Tártara ，它写出了示例代码
//NVidia 公司，他在Internet发布了大量的源码
//最后感谢Nehe，它对我的OpenGL学习起了很大的帮助
