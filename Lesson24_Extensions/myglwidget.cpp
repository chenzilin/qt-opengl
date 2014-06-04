#include "myglwidget.h"

//扩展，剪裁和TGA图像文件的加载:
//在这一课里，你将学会如何读取你显卡支持的OpenGL的扩展，并在你指定的剪裁区域把它显示出来。

//这个教程有一些难度，但它会让你学到很多东西。我听到很多朋友问我扩展方面的内容和怎样找到它们。这个教程将交给你这一切。
//我将教会你怎样滚动屏幕的一部分和怎样绘制直线，最重要的是从这一课起，我们将不使用AUX库，以及*.bmp文件。我将告诉你如何使用Targa(TGA)图像文件。
//因为它简单并且支持alpha通道，它可以使你更容易的创建酷的效果。
//接下来我们要做的第一件事就是不包含glaux.h头文件和glaux.lib库。另外，在使用glaux库时，经常会发生一些可疑的警告，现在我们可以测定告别它了。

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_scroll(0),
    m_maxtokens(0), m_swidth(0), m_sheight(0)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(1, &m_textures[0]);
    glDeleteLists(m_base, 256);
}

//窗口改变大小的函数使用正投影，把视口范围设置为(0,0)-(640,480)
void MyGLWidget::resizeGL(int w, int h)
{
    m_swidth=w; // 设置剪切矩形为窗口大小
    m_sheight=h;
    if (h==0)// 防止高度为0时，被0除
    {
            h=1;
    }
    glViewport(0,0,w,h); // 设置窗口可见区
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f,640,480,0.0f,-1.0f,1.0f); // 设置视口大小为640x480
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

//初始化操作非常简单，我们载入字体纹理，并创建字符显示列表，如果顺利，则成功返回。
void MyGLWidget::initializeGL()
{
    loadGLTexture();// 载入字体纹理
    buildFont();// 创建字体
    glShadeModel(GL_SMOOTH); // 使用平滑着色
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f); // 设置黑色背景
    glClearDepth(1.0f); // 设置深度缓存中的值为1
    glBindTexture(GL_TEXTURE_2D, m_textures[0]);//绑定字体纹理
}

//绘制代码几乎是全新的:)，token为一个指向字符串的指针，它将保存OpenGL扩展的全部字符串，cnt纪录扩展的个数。
//接下来清楚背景，并显示OpenGL的销售商，实现它的公司和当前的版本。
void MyGLWidget::paintGL()
{
    char *token;// 保存扩展字符串
    int cnt=0;// 纪录扩展字符串的个数
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清楚背景和深度缓存
    glColor3f(1.0f,0.5f,0.5f); // 设置为红色
    glPrint(50,16,1,"Renderer");
    glPrint(80,48,1,"Vendor");
    glPrint(66,80,1,"Version");
    //下面的代码显示OpenGL实现方面的相关信息，完成之后我们用蓝色在屏幕的下方写
    //上“NeHe Productions”，当然你可以使用任何你想使用的字符，比如"DancingWind Translate"。
    glColor3f(1.0f,0.7f,0.4f);// 设置为橘黄色
    glPrint(200,16,1,(char *)glGetString(GL_RENDERER));// 显示OpenGL的实现组织
    glPrint(200,48,1,(char *)glGetString(GL_VENDOR));// 显示销售商
    glPrint(200,80,1,(char *)glGetString(GL_VERSION));// 显示当前版本
    glColor3f(0.5f,0.5f,1.0f);// 设置为蓝色
    glPrint(192,432,1,"NeHe Productions");// 在屏幕的底端写上NeHe Productions字符串
    //现在我们绘制显示扩展名的白色线框方块，并用一个更大的白色线框方块把所有的内容包围起来。
    glLoadIdentity(); // 重置模型变换矩阵
    glColor3f(1.0f,1.0f,1.0f);// 设置为白色
    glBegin(GL_LINE_STRIP);
        glVertex2d(639,417);
        glVertex2d(0,417);
        glVertex2d(0,480);
        glVertex2d(639,480);
        glVertex2d(639,128);
    glEnd();
    glBegin(GL_LINE_STRIP);
        glVertex2d(0,128);
        glVertex2d(639,128);
        glVertex2d(639,1);
        glVertex2d(0,1);
        glVertex2d(0,417);
    glEnd();
    //glScissor函数用来设置剪裁区域，如果启用了GL_SCISSOR_TEST,绘制的内容只能在剪裁区域中显示。
    //下面的代码设置窗口的中部为剪裁区域，并获得扩展名字符串。
    glScissor(1, int(0.135416f*m_sheight), m_swidth-2,int(0.597916f*m_sheight));// 定义剪裁区域
    glEnable(GL_SCISSOR_TEST);// 使用剪裁测试
    char* text=(char*)malloc(strlen((char *)glGetString(GL_EXTENSIONS))+1);// 为保存OpenGL扩展的字符串分配内存空间
    strcpy(text,(char *)glGetString(GL_EXTENSIONS));
    //下面我们创建一个循环，循环显示每个扩展名，并纪录扩展名的个数
    token=strtok(text," ");// 按空格分割text字符串，并把分割后的字符串保存在token中
    while(token!=NULL)// 如果token不为NULL
    {
        cnt++;// 增加计数器
        if(cnt>m_maxtokens)// 纪录最大的扩展名数量
        {
            m_maxtokens=cnt;
        }
        //我们已经获得第一个扩展名，下一步我们把它显示在屏幕上。
        //我们已经显示了三行文本，它们在Y轴上占用了3*32=96个像素的宽度，所以我们显示的
        //第一个行文本的位置是(0,96)，一次类推第i行文本的位置是(0,96+(cnt*32)),但我们需要考
        //虑当前滚动过的位置，默认为向上滚动，所以我们得到显示第i行文本的位置为(0,96+(cnt*32)=scroll)。
        //当然它们不会都显示出来，记得我们使用了剪裁，只显示(0,96)-(0,96+32*9)之间的文本，其它的都被剪裁了。
        //更具我们上面的讲解，显示的第一个行如下：
        //1 GL_ARB_multitexture
        glColor3f(0.5f,1.0f,0.5f);// 设置颜色为绿色
        QString number = QString::number(cnt);
        glPrint(0,96+(cnt*32)- m_scroll, 0, number.toLatin1().data());// 绘制第几个扩展名
        glColor3f(1.0f,1.0f,0.5f);// 设置颜色为黄色
        glPrint(50,96+(cnt*32)- m_scroll,0, token);
        //当我们显示完所有的扩展名,我们需要检查一下是否已经分析完了所有的字符串。
        //我们使用strtok(NULL," ")函数代替strtok(text," ")函数，
        //把第一个参数设置为NULL会检查当前指针位置到字符串末尾是否包含" "字符，如果包含返回其位置，否则返回NULL。
        //我们举例说明上面的过程，例如字符串"GL_ARB_multitexture GL_EXT_abgr GL_EXT_bgra",
        //它是以空格分割字符串的，第一次调用strtok("text"," ")返回text的首位置，
        //并在空格" "的位置加入一个NULL。以后每次调用，删除NULL，返回空格位置的下一个位置，
        //接着搜索下一个空格的位置，并在空格的位置加入一个NULL。直道返回NULL。
        //返回NULL时循环停止，表示已经显示完所有的扩展名。
        token=strtok(NULL," ");//查找下一个扩展名
    }
    //下面的代码让OpenGL返回到默认的渲染状态，并释放分配的内存资源
    glDisable(GL_SCISSOR_TEST);// 禁用剪裁测试
    free(text);//释放内存
    glFlush(); //执行当前所有渲染命令
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
            m_scroll-=2;
            break;
        }
        case Qt::Key_Down:
        {
            m_scroll+=2;
            break;
        }
    }
    QGLWidget::keyPressEvent(event);
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    updateGL();
    QGLWidget::timerEvent(event);
}

void MyGLWidget::loadGLTexture()
{
    QImage image(":/image/Font.tga");
    image = image.convertToFormat(QImage::Format_RGB888);
    image = image.mirrored();
    image.save("font.bmp");
    glGenTextures(1, &m_textures[0]);//创建纹理
    glBindTexture(GL_TEXTURE_2D, m_textures[0]); //绑定纹理
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// 设置过滤器为线性滤波
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width(), image.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, image.bits());
}

void MyGLWidget::buildFont()
{
    m_base=glGenLists(256); // 创建256个显示列表
    glBindTexture(GL_TEXTURE_2D, m_textures[0]); //绑定纹理
    for (int i=0; i<256; i++)   //循环创建256个显示列表
    {
        float cx=float(i%16)/16.0f;//当前字符X位置
        float cy=float(i/16)/16.0f;//当前字符Y位置

        glNewList(m_base+i,GL_COMPILE); //开始创建显示列表
            glBegin(GL_QUADS);  //创建一个四边形用来包含字符图像
                glTexCoord2f(cx,1.0f-cy-0.0625f);//左下方纹理坐标
                glVertex2d(0,16);//左下方坐标
                glTexCoord2f(cx+0.0625f,1.0f-cy-0.0625f);//右下方纹理坐标
                glVertex2i(16,16);//右下方坐标
                glTexCoord2f(cx+0.0625f,1.0f-cy-0.001f);//右上方纹理坐标
                glVertex2i(16,0);//右上方坐标
                glTexCoord2f(cx,1.0f-cy-0.001f);//左上方纹理坐标
                glVertex2i(0,0);//左上方坐标
            glEnd();//四边形创建完毕
            glTranslated(14,0,0);//向右移动14个单位
        glEndList();//结束创建显示列表
    }
}

//下一段代码将完成绘图。一切都几乎是新的，所以我将尽可能详细的解释每一行。
//一个小提示：很多都可加入这段代码，像是变量的支持，字体大小、间距的调整，和很多为恢复到我们决定打印前的状况所做的检查。
//glPrint()有三个参数。第一个是屏幕上x轴上的位置（从左至右的位置），下一个是y轴上的位置
//（从下到上...0是底部，越往上越大）。然后是字符串（我们想打印的文字），最后是一个叫做set的变量。
//如果你看过Giuseppe D'Agata制作的位图，你会注意到有两个不同的字符集。第一个字符集是普通的，第二个是斜体的。
//如果set为0，第一个字符集被选中。若set为1则选择第二个字符集。
void MyGLWidget::glPrint(GLint x, GLint y, int set, char *string)
{
    //我们要做的第一件事是确保set的值非0即1。如果set大于1，我们将使它等于1。
    if (set>1)								// 如果字符集大于1
    {
        set=1;								// 设置其为1
    }
    glEnable(GL_TEXTURE_2D);								// Enable Texture Mapping
    glLoadIdentity();										// Reset The Modelview Matrix
    glTranslated(x,y,0);									// Position The Text (0,0 - Top Left)
    glListBase(m_base-32+(128*set));							// Choose The Font Set (0 or 1)

    glScalef(1.0f,2.0f,1.0f);								// Make The Text 2X Taller

    glCallLists(strlen(string),GL_UNSIGNED_BYTE, string);		// Write The Text To The Screen
    glDisable(GL_TEXTURE_2D);								// Disable Texture Mapping
}

//我希望你觉得这个教程有趣，学完了这个教程你应该知道如何获得你的显卡的发售商的名称，
//实现OpenGL的组织和你的显卡所使用的OpenGL的版本。进一步，你应该知道你的显卡支持的扩展的名称，
//并熟练的使用剪切矩形和加载TGA图像。
//如果你发现任何问题，请让我知道。我想做最好的教程，你的反馈对我很重要。
