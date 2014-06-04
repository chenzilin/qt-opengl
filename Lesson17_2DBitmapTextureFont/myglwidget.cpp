#include "myglwidget.h"

//2D 图像文字:
//在这一课中，你将学会如何使用四边形纹理贴图把文字显示在屏幕上。你将学会如何把256个不同的文字从一个256x256的纹理图像中分别提取出来，
//并为每一个文字创建一个显示列表，接着创建一个输出函数来创建任意你希望的文字。

//本教程由NeHe和Giuseppe D'Agata提供。
//我知道每个人都或许厌恶字体。目前为止我写的文字教程不仅能显示文字，还能显示3D文字，有纹理贴图的文字，以及处理变量。
//但是当你将你的作品移植到不支持位图或是轮廓字体的机器上会发生什么事呢？
//由于Giuseppe D'Agata我们有了另一篇字体教程。你还会问什么？如果你记得在第一篇字体教程中我提到使用纹理在屏幕上绘制文字。
//通常当你使用纹理绘制文字时你会调用你最喜欢的图像处理程序，选择一种字体，然后输入你想显示的文字或段落。
//然后你保存位图并把它作为纹理读入到你的程序里。对一个需要很多文字或是文字在不停变化的程序来说这么做效率并不高。
//本教程只使用有一个纹理来显示任意256个不同的字符。记住平均一个字符只有16个像素宽，大概16个像素高。
//如果你使用标准的256x256的纹理那么很明显你可以放入交叉的16个文字（即一个X），且最多16行16列。
//如果你需要一个更详细的解释：纹理是256个像素宽，一个字符是16个像素宽，256除以16得16:)
//现在让我们来创建一个2D纹理字体demo！这课的程序基于第一课的代码。在程序的第一段，我们包括数学（math）和标准输入输出库（stdio）。
//我们需要数学库来使用正弦和余弦函数在屏幕上移动我们的文字，我们需要标准输入输出库来保证在我们制作纹理前要使用的位图实际存在。

//我们将要加入一个变量base来指向我们的显示列表。我们还加入texture[2]来保存我们将要创建的两个纹理。Texture 1将是字体纹理，
//texture 2将是用来创建简单3D物体的凹凸纹理。
//我们加入用来执行循环的变量loop。最后我们加入用来绕屏幕移动文字和旋转3D物体的cnt1和cnt2。

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_cnt1(0.0f), m_cnt2(0.0f)
{
    showNormal();
    startTimer(100);
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(2, &m_texture[0]);
    glDeleteLists(m_base, 256);
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
    loadGLTexture();

    buildFont();

    //现在我们做通常的GL设置。我们将背景色设为黑色，将深度清为1.0。我们选择一个深度测试模式和一个混合模式。
    //我们启用平滑着色，最后启用2维纹理映射。
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);				// 黑色背景
    glClearDepth(1.0);                                  // 设置深度缓存
    glDepthFunc(GL_LEQUAL);                             // 所作深度测试的类型
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);					// 设置混合因子
    glShadeModel(GL_SMOOTH);							// 启用阴影平滑
    glEnable(GL_TEXTURE_2D);							// 启用纹理映射
}

//我们先绘制3D物体最后绘制文字，这样文字将显示在3D物体上面，而不会被3D物体遮住。
//我之所以加入一个3D物体是为了演示透视投影和正交投影可同时使用。
void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				// 清除屏幕和深度缓存
    glLoadIdentity();								// 重置当前的模型观察矩阵
    //我们选择bumps.bmp纹理来创建简单的小3D物体。为了看见3D物体，我们往屏幕内移动5个单位。
    //我们绕z轴旋转45度。这将使我们的四边形顺时针旋转45度，让我们的四边形看起来更像钻石而不是矩形。
    glBindTexture(GL_TEXTURE_2D, m_texture[1]);			// 设置为图像纹理
    glTranslatef(0.0f,0.0f,-5.0f);						// 移入屏幕5个单位
    glRotatef(45.0f,0.0f,0.0f,1.0f);					// 沿Z轴旋转45度
    //在旋转45度后，我们让物体同时绕x轴和y轴旋转cnt1x30度。这使我们的物体象在一个点上旋转的钻石那样旋转。
    glRotatef(m_cnt1*30.0f,1.0f,1.0f,0.0f);					//  沿(1,1,0)轴旋转30度
    //我们关闭混合（我们希望3D物体看上去像实心的），设置颜色为亮白色。然后我们绘制一个单独的用了纹理映像的四边形。
    glDisable(GL_BLEND);							// 关闭混合
    glColor3f(1.0f,1.0f,1.0f);							//设置颜色为白色
    glBegin(GL_QUADS);								// 绘制纹理四边形
        glTexCoord2d(0.0f,0.0f);
        glVertex2f(-1.0f, 1.0f);
        glTexCoord2d(1.0f,0.0f);
        glVertex2f( 1.0f, 1.0f);
        glTexCoord2d(1.0f,1.0f);
        glVertex2f( 1.0f,-1.0f);
        glTexCoord2d(0.0f,1.0f);
        glVertex2f(-1.0f,-1.0f);
    glEnd();
    //在画完第一个四边形后，我们立即同时绕x轴和y轴旋转90度。然后我们画下一个四边形。
    //第二个四边形从第一个四边形的中间切过去，来形成一个好看的形状。
    glRotatef(90.0f,1.0f,1.0f,0.0f);					//  沿(1,1,0)轴旋转90度
    glBegin(GL_QUADS);							// 绘制第二个四边形，与第一个四边形垂直
        glTexCoord2d(0.0f,0.0f);
        glVertex2f(-1.0f, 1.0f);
        glTexCoord2d(1.0f,0.0f);
        glVertex2f( 1.0f, 1.0f);
        glTexCoord2d(1.0f,1.0f);
        glVertex2f( 1.0f,-1.0f);
        glTexCoord2d(0.0f,1.0f);
        glVertex2f(-1.0f,-1.0f);
    glEnd();
    //在绘制完有纹理贴图的四边形后，我们开启混合并绘制文字。
    glEnable(GL_BLEND);							// 启用混合操作
    glLoadIdentity();								// 重置视口
    //我们使用同其它字体教程一样的生成很棒的颜色的代码。颜色会随着文字的移动而逐渐改变。
    // 根据字体位置设置颜色
    glColor3f(1.0f*float(cos(m_cnt1)),1.0f*float(sin(m_cnt2)),1.0f-0.5f*float(cos(m_cnt1+m_cnt2)));
    //我们来绘制文字。我们仍然使用glPrint()。第一个参数是x坐标，第二个是y坐标，第三个（"NeHe"）是要绘制的文字，
    //最后一个是使用的字符集（0-普通，1-斜体）。
    //正如你猜的，我们使用SIN和COS连同计数器cnt1和cnt2来移动文字。如果你不清楚SIN和COS的作用，阅读之前的教程。
    glPrint(int((280+250*cos(m_cnt1))),int(235+200*sin(m_cnt2)), "NeHe", 0);
    glColor3f(1.0f*float(sin(m_cnt2)),1.0f-0.5f*float(cos(m_cnt1+m_cnt2)),1.0f*float(cos(m_cnt1)));
    glPrint(int((280+230*cos(m_cnt2))),int(235+200*sin(m_cnt1)), "OpenGL",1);
    //我们将屏幕底部作者名字的颜色设为深蓝色和白色。然后用亮白色文字再次绘制他的名字。亮白色文字是有点偏蓝色的文字。
    //这创造出一种附有阴影的样子。（如果混合没打开则没有这种效果）。
    glColor3f(0.0f,0.0f,1.0f);
    glPrint(int(240+200*cos((m_cnt2+m_cnt1)/5)),2, "Giuseppe D'Agata",0);
    glColor3f(1.0f,1.0f,1.0f);
    glPrint(int(242+200*cos((m_cnt2+m_cnt1)/5)),2, "Giuseppe D'Agata",0);
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

//我们所做的最后一件事是以不同的速率递增我们的计数器。这使得文字移动，3D物体自转。
void MyGLWidget::timerEvent(QTimerEvent *event)
{
    m_cnt1+=0.01f;								// 增加计数器值
    m_cnt2+=0.0081f;								// 增加计数器值
    updateGL();
    QGLWidget::timerEvent(event);
}

//接下来是读取纹理代码。这跟前面纹理影射教程中的一模一样。
//下面的代码同样对之前教程的代码改动很小。如果你不清楚下面每行的用途，回头复习一下。
//注意TextureImage[ ]将保存2个rgb图像记录。复查处理读取或存储纹理的纹理很重要。一个错误的数字可能导致内存溢出或崩溃！
void MyGLWidget::loadGLTexture()
{
    QImage fontImage(":/image/Font.bmp");
    QImage bumpsImage(":/image/Bumps.bmp");
    fontImage = fontImage.convertToFormat(QImage::Format_RGB888);
    bumpsImage = bumpsImage.convertToFormat(QImage::Format_RGB888);
    fontImage = fontImage.mirrored();
    bumpsImage = bumpsImage.mirrored();
    glGenTextures(2, &m_texture[0]);
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fontImage.width(), fontImage.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, fontImage.bits());

    glBindTexture(GL_TEXTURE_2D, m_texture[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bumpsImage.width(), bumpsImage.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, bumpsImage.bits());
}

//现在我们将创建字体。我将以同样的细节来解释这段代码。这并没那么复杂，但是有些数学要了解，我知道不是每个人都喜欢数学。
void MyGLWidget::buildFont() // 创建我们的字符显示列表
{
    //下面两个变量将用来保存字体纹理中每个字的位置。cx将用来保存纹理中水平方向的位置，cy将用来保存纹理中竖直方向的位置。
    float	cx;                             // 字符的X坐标
    float	cy;								// 字符的Y坐标
    //接着我们告诉OpenGL我们要建立256个显示列表。变量base将指向第一个显示列表的位置。
    //第二个显示列表将是base+1，第三个是base+2，以此类推。
    //下面的第二行代码选择我们的字体纹理（texture[0]）。
    m_base = glGenLists(256);							// 创建256个显示列表
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);					// 选择字符图象
    for (quint32 i = 0, iend = 256; i < iend; i++)						// 循环256个显示列表
    {
        //下面的第一行或许看上去让人有点困惑。%符号表示loop除以16的余数。cx将我们通过字体纹理从左至右移动。
        //你将注意到在后面的代码中我们用1减去cy从而从上到下而不是从下到上移动我们。%符号很难解释，但我将尝试去解释。
        //我们真正关心的是（loop%16）。/16只是将结果转化为纹理坐标。所以如果loop等于16，cx将等于16/16的余数也就是0。
        //但cy将等于16/16也就是1。所以我们将下移一个字符的高度，且我们将不往右移。
        //如果loop等于17，cx将等于17/16也就是1.0625。余数0.625也等于1/16。意味着我们将右移一个字符。
        //cy将仍是1因为我们只关心小数点左边的数字。18/16将右移2个字符，但仍下移一个字符。
        //如果loop是32，cx将再次等于0，因为32除以16没有余数，但cy将等于2。
        //因为小数点左边的数字现在是2，将下移2个字符。这么讲清楚吗？
        cx=float(i%16)/16.0f;					// 当前字符的X坐标
        cy=float(i/16)/16.0f;					// 当前字符的Y坐标
        //Ok。现在我们通过从字体纹理中依据cx和cy的值选择一个单独的字符创建了2D字体。
        //在下面的行里我们给base的值加上loop，若不这么做，每个字都将建在第一个显示列表里。
        //我们当然不想要那样的事发生，所以通过给base加上loop，我们创建的每个字都被存在下个可用的显示列表里。
        glNewList(m_base+i,GL_COMPILE);				//开始创建显示列表
        //现在我们已选择了我们要创建的显示列表，我们创建字符。这是通过绘制四边形，
        //然后给他贴上字体纹理中的单个字符的纹理来完成的。
            glBegin(GL_QUADS);					// 使用四边形显示每一个字符
                //cx和cy应该保存一个从0.0到1.0的非常小的浮点数。如果cx和cy同时为0，
                //下面第一行的代码将为：glTexCoord2f(0.0f,1-0.0f-0.0625f)。
                //记得0.0625正是我们纹理的1/16，或者说是一个字符的宽/高。下面的纹理坐标将是我们纹理的左下角。
                //注意我们使用glVertex2i(x,y)而不是glVertex3f(x,y,z)。我们的字体是2D字体，所以我们不需要z值。
                //因为我们使用的是正交投影，我们不需要移进屏幕。在一个正交投影平面绘图你所需的是指定x和y坐标。
                //因为我们的屏幕是以像素形式从0到639（宽）从0到479（高），我们既不需用浮点数也不用负数:)
                //我们设置正交投影屏幕的方式是，(0,0)将是屏幕的左下角，(640,480)是屏幕的右上角。
                //x轴上0是屏幕的左边界，639是右边界。y轴上0时下便捷，479是上便捷。基本上我们避免了负坐标。
                //对那些不在乎透视，更愿意同像素而不是单元打交道的人来说更方便:)
                glTexCoord2f(cx, 1-cy-0.0625f);		// 左下角的纹理坐标
                glVertex2i(0,0);				// 左下角的坐标
                //下一个纹理坐标现在是上个纹理坐标右边1/16（刚好一个字符宽）。所以这将是纹理的右下角。
                glTexCoord2f(cx+0.0625f,1-cy-0.0625f);	// 右下角的纹理坐标
                glVertex2i(16,0);				// 右下角的坐标
                //第三个纹理坐标在我们的字符的最右边，但上移了纹理的1/16（刚好一个字符高）。这将是一个单独字符的右上角。
                glTexCoord2f(cx+0.0625f,1-cy);		// 右上角的纹理坐标
                glVertex2i(16,16);				// 右上角的坐标
                //最后我们左移来设置字符左上角的最后一个纹理坐标。
                glTexCoord2f(cx,1-cy);			// 左上角的纹理坐标
                glVertex2i(0,16);				// 左上角的坐标
            glEnd();						// 四边形字符绘制完成
        glTranslated(16,0,0);					// 绘制完一个字符，向右平移16个单位
        glEndList();							// 字符显示列表结束
    }// 循环建立256个显示列表
}

//下一段代码将完成绘图。一切都几乎是新的，所以我将尽可能详细的解释每一行。
//一个小提示：很多都可加入这段代码，像是变量的支持，字体大小、间距的调整，和很多为恢复到我们决定打印前的状况所做的检查。
//glPrint()有三个参数。第一个是屏幕上x轴上的位置（从左至右的位置），下一个是y轴上的位置
//（从下到上...0是底部，越往上越大）。然后是字符串（我们想打印的文字），最后是一个叫做set的变量。
//如果你看过Giuseppe D'Agata制作的位图，你会注意到有两个不同的字符集。第一个字符集是普通的，第二个是斜体的。
//如果set为0，第一个字符集被选中。若set为1则选择第二个字符集。
void MyGLWidget::glPrint(GLint x, GLint y, char *string, int set)
{
    //我们要做的第一件事是确保set的值非0即1。如果set大于1，我们将使它等于1。
    if (set>1)								// 如果字符集大于1
    {
        set=1;								// 设置其为1
    }
    //现在我们选择字体纹理。我们这么做是防止在我们决定往屏幕上输出东西时选择了不同的纹理。
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);				// 绑定为字体纹理
    //现在我们禁用深度测试。我这么做是因为混合的效果会更好。如果你不禁用深度测试，文字可能会被什么东西挡住，
    //或得不到正确的混合效果。如果你不打算混合文字（那样文字周围的黑色区域就不会显示）你可以启用深度测试。
    glDisable(GL_DEPTH_TEST);						// 禁止深度测试
    //下面几行十分重要！我们选择投影矩阵。之后使用一个叫做glPushMatrix()的命令。
    //glPushMatrix存储当前矩阵（投影）。有些像计算器的存储按钮。
    glMatrixMode(GL_PROJECTION);						// 选择投影矩阵
    glPushMatrix();								// 保存当前的投影矩阵
    //现在我们保存了投影矩阵，重置矩阵并设置正交投影屏幕。第一和第三个数字（0）表示屏幕的底边和左边。
    //如果愿意我们可以将屏幕的左边设为-640，但如果不需要我们为什么要设负数呢。
    //第二和第四个数字表示屏幕的上边和右边。将这些值设为你当前使用的分辨率是明智的做法。
    //我们不需要用到深度，所以我们将z值设为-1与1。
    glLoadIdentity();								// 重置投影矩阵
    glOrtho(0,640,0,480,-1,1);							// 设置正投影的可视区域
    //现在我们选择模型视点矩阵，用glPushMatrix()保存当前设置。然后我们重置模型视点矩阵以便在正交投影视点下工作。
    glMatrixMode(GL_MODELVIEW);							// 选择模型变换矩阵
    glPushMatrix();								// 保存当前的模型变换矩阵
    glLoadIdentity();								// 重置模型变换矩阵
    //在保存了透视参数，设置了正交投影屏幕后，现在我们可以绘制文字了。我们从移动到绘制文字的位置开始。
    //我们使用 glTranslated()而不是glTranslatef()因为我们处理的是像素，所以浮点值并不重要。
    //毕竟，你不可能用半个像素:)
    glTranslated(x,y,0);							// 把字符原点移动到(x,y)位置
    //下面这行选择我们要使用的字符集。如果我们想使用第二个字符集，
    //我们在当前的显示列表基数上加上128（128时我们256个字符的一半）。通过加上128，我们跳过了头128个字符。
    glListBase(m_base-32+(128*set));						// 选择字符集
    //现在剩下的就是在屏幕上绘制文字了。我们同其它字体教程一样来完成这步。
    //我们使用glCallLists()。strlen(string)是字符串的长度（我们想绘制多少字符），
    //GL_UNSIGNED_BYTE意味着每个字符被表示为一个无符号字节（一个字节是一个从0到255的值）。
    //最后，字符串保存我们想打印的文字。
    GLint len = strlen(string);
    glCallLists(len, GL_UNSIGNED_BYTE, string);					// 把字符串写入到屏幕
    //现在我们所要做的是恢复透视视图。我们选择投影矩阵并用glPopMatrix()恢复我们先前用glPushMatrix()保存的设置。
    //用相反的顺序恢复设置很重要。
    glMatrixMode(GL_PROJECTION);						// 选择投影矩阵
    glPopMatrix();								// 设置为保存的矩阵
    //现在我们选择模型视点矩阵，做相同的工作。我们使用glPopMatrix()恢复模型视点矩阵到我们设置正交投影显示之前。
    glMatrixMode(GL_MODELVIEW);							// 选择模型矩阵
    glPopMatrix();								// 设置为保存的矩阵
    //最后，我们启用深度测试。如果你没有在上面的代码中关闭深度测试，你不需要这行。
    glEnable(GL_DEPTH_TEST);						// 启用深度测试
}

//我认为现在我可以正式说我的网站已经把所有绘制文字的方法教给大家了{笑}。总之，我认为我的教程很不错。
//这课的代码可在任何能运行OpenGL的电脑上运行，它很容易使用，且这样绘制文字对系统的资源消耗很少。
//我要感谢这篇教程的原作者Giuseppe D'Agata。我做了大量的修改，并将它转变为新式的代码，
//但要是没有他寄给我这份代码我是不会完成这篇教程的。他的代码有更多的选项，像是改变文字间距等等。
//但我用很cool的3D物体来弥补了{笑}。
//我希望你们喜欢这篇教程。若有什么问题，给我或Giuseppe D'Agata发email。
//Giuseppe D'Agata
//Jeff Molofee (NeHe)
