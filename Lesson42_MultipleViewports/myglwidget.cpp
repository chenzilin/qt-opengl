#include "myglwidget.h"

//多视窗口
//欢迎来到充满趣味的另一课。这次我将向你展示怎样在单个窗口内显示多个视口。这些视口在窗口模式下能正确的调整大小。其中有两个窗口起用了光照。
//窗口之一用的是正交投影而其他三个则是透视投影。为了保持教程的趣味性，在本例子中我们同样需要学习迷宫代码，怎么渲染到一张纹理以及怎么得到当前窗口的分辨率。
//一旦你明白了本教程，制作分屏游戏以及多视图的3D程序就很简单了。接下来，让我们投入到代码中来吧！！！
//你可以利用最近的NeHeGL或者IPicture代码作为主要基本代码。我们需要看的第一个文件就是NeHeGL.cpp,其中有三节代码已经被修改了。我将只列出那些被修改了的代码。
//第一个且最重要的被修改了的代码就是ReshapeGL()函数。这是我们设置屏幕(主视口)分辨率的地方。现在所有的主视口设置都在画循环里完成了。
//因此这儿所有我们能做的就是设置我们的主窗口。

const int mazeWidth = 128;// 迷宫大小
const int mazeHeight = 128;

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
    delete []m_tex_data;
}

void MyGLWidget::resizeGL(int w, int h)
{
    glViewport (0, 0, (GLsizei)(w), (GLsizei)(h));				// Reset The Current Viewport
}


void MyGLWidget::initializeGL()
{
    m_tex_data=new unsigned char[mazeWidth*mazeHeight*3];// 分配保存纹理的空间
    //一分配完内存，我们就调用Reset()函数，Reset会清空贴图，设置所需颜色，并为迷宫选取随机起点。
    //一旦所有的东西都设置好了。我们建立我们的初始纹理。前两个纹理参数将纹理坐标截断在 [0,1]范围内，
    //当把一个单独的图像映射到一个物体上时，这种方式可以避免缠绕时人为因素的影响(?本句翻译不爽，请指正).
    //为了看到CLAMP参数的重要性，可以移掉这两行代码看看。如果没有Clamping,你会注意到在纹理的顶部和右边的细小线条。
    //这些线条的出现是因为线性过滤想使整个纹理平滑，包括纹理边界。如果一个靠近边界的点被画了，在纹理的对边上就会出现一条线。
    //我们打算用线性过滤来使纹理变的更平滑一点。 用什么类型的过滤是由你来决定的。如果它使程序跑起来很慢，那就换成过滤类型为GL_NEAREST
    //最后，我们利用tex_data数据（并没有利用alpha通道）建立了一个二维的RGB纹理。
    reset();                                        // 重置纹理贴图
    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mazeWidth, mazeHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, m_tex_data);
    //我们设置用于清空颜色缓冲区的颜色为黑色，清空深度缓冲区的值为1.0f. 设置深度函数为less than或者equal to, 然后激活深度测试。
    //激活GL_COLOR_MATERIAL可以让你在激活光照的情况下用glColor函数给物体上色。这个方法又称为颜色追踪, 常常是性能杀手的glMaterial的代替品。
    //希望这些信息对这个有帮助！对于那些发email问我为什么纹理的颜色如此怪异或者问纹理颜色受当前glColor影响的人，请确认一下你没有激活GL_COLOR_MATERIAL.
    //*多谢James Trotter对GL_COLOR_MATERIAL功能的解释。我曾说过它会对你的纹理上色...实际上，它是对你的物体上色。
    //最后我们激活2维纹理映射。
    glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth (1.0f);
    glDepthFunc (GL_LEQUAL);
    glEnable (GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_TEXTURE_2D);
    //下面的代码建立了一个二次曲面物体并得到指向它的指针。一旦我们有这个指针后，我们设置它的法线类型为平滑类型，然后要求生成纹理坐标。
    //这样我们的光照才能正确的工作，并且我们的纹理能自动的映射到二次曲面物体。
    m_quadric=gluNewQuadric();
    gluQuadricNormals(m_quadric, GLU_SMOOTH);
    gluQuadricTexture(m_quadric, GL_TRUE);
    //Light0被激活，但是如果我们不激活光照，它不会起任何作用。Light0是预定义的灯光，方向指向屏幕内。如果你不喜欢的话，可以手工自己设置
    glEnable(GL_LIGHT0);
}

//这段代码我们开始讲一些新的东西...我们必须知道当前窗口的大小以便正确的调整视口的大小。为了的到当前窗口的宽和高，我们需要获取窗口上下左右坐标值。
//得到这些值后我们通过窗口右边的坐标减去左边的坐标得到宽度值。底部坐标减去顶部坐标得到窗口的高度值。
//我们用RECT结构来得到窗口的那些值。RECT保存了一个矩形的坐标。也即矩形的左，右，顶部，底部的坐标。
//为获取窗口的屏幕坐标，我们用GetClientRect()函数。我们传进去的第一个参数是当前窗口的句柄。第二个参数是一个结构用于保存返回的窗口位置信息.
void MyGLWidget::paintGL()
{
    QRect rect;// 保存长方形坐标
    rect = this->rect();// 获得窗口大小
    int window_width=rect.width();
    int window_height=rect.height();
    //我们在每一帧都需要更新纹理并且要在映射纹理之前更新。更新纹理最快的方法是用命令glTexSubImage2D().
    //它能把内存中的纹理的全部或部分和屏幕中的物体建立映射。下面的代码我们表明用的2维纹理，纹理细节级别为0，没有x方向(0)或y方向(0)的偏移，
    //我们需要利用整张纹理的每一部分，图像为GL_RGB类型，对应的数据类型为GL_UNSIGNED_BYTE. tex_data是我们需要映射的具体数据。
    //这是一个非非常快的不用重建纹理而更新纹理的方法。同样需要注意的是这个命令不会为你建立一个纹理。你必须在更新纹理前把纹理建立好。
    // 设置更新的纹理
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mazeWidth, mazeHeight, GL_RGB, GL_UNSIGNED_BYTE, m_tex_data);
    //这行代码非常重要，它将清空整个屏幕。
    //……
    //一次性清空整个屏幕，然后在画每一个视口前清空它们的深度存非常重要。
    glClear (GL_COLOR_BUFFER_BIT);
    //现在是主画循环。我们要画四个视口，所以建立了一个0到3的循环。
    //首先要做的事是设置用glColor3ub(r,g,b)设置当前视口的颜色。这对某些人来说不太熟悉，它跟glColor3f(r,g,b)几乎一样但是用无符号字节代替浮点数为参数。
    //记住早些时候我说过参省一个0-255的随机颜色值会更容易。好在已经有了该命令设置正确颜色所需要的值。
    //glColor3f(0.5f,0.5f,0.5f)是指颜色中红，绿，蓝具有50%的亮度值。glColor3ub(127,127,127)同样也表示同样的意思。
    //如果loop的值为0,我们将选择r[0],b[0],b[[0]，如果loop指为1, 我们选用r[1],g[1],b[1]. 这样，每个场景都有自个的随机颜色。
    for (int loop=0; loop<4; loop++)                            // 循环绘制4个视口
    {
        glColor3ub(m_r[loop],m_g[loop],m_b[loop]);
        //在画之前首先要做的是设置当前视口，如果loop值为0,我们画第一个视口。我们想把第一个视口放在屏幕的左半部分(0),并且在屏幕的上半部分(window_height/2).
        //视口的宽度为当前主窗口的一半(window_width/2), 高度也为主窗口高度的一半(window_height/2).
        //如果主窗口为1024x768, 结果就是一个起点坐标为0,384,宽512，高384的视口。
        //设置完视口后，我们选择当前矩阵为投影矩阵，重置它并设置为2D平行投影视图。我们需要以平行投影视图来填充整个视口，因此我们给左边的值为0,
        //右边的值为window_width/2(跟视口一样)，同样给底部的值赋为window_height/2，顶部的值为0. 这样给了视口同样的高度。
        //这个平行投影视图的左上角的坐标为0,0,右下角坐标为window_width/2,window_height/2.
        if (loop==0)                                    // 绘制左上角的视口
        {
            // 设置视口区域
            glViewport (0, window_height/2, window_width/2, window_height/2);
            glMatrixMode (GL_PROJECTION);
            glLoadIdentity ();
            gluOrtho2D(0, window_width/2, window_height/2, 0);
        }
        //如果loop的值为1, 我们是在画第二个视口了。它在屏幕的右上部分。宽度和高度都跟前一个视图一样。
        //唯一不同的是glViewport()函数的第一个参数为window_width/2.这告诉程序视口起点是从窗口左起一半的地方。
        //同样的，我们设置当前矩阵为投影矩阵并重置它。但这次我们设置透视投影参数为FOV为45度，并且近截面值为0.1f，远截面值为500.0f
        if (loop==1)                                    // 绘制右上角视口
        {
            glViewport (window_width/2, window_height/2, window_width/2, window_height/2);
            glMatrixMode (GL_PROJECTION);
            glLoadIdentity ();
            gluPerspective( 45.0, (GLfloat)(mazeWidth)/(GLfloat)(mazeHeight), 0.1f, 500.0 );
        }
        //如果loop值为2,我们画第三个视口。它将在主窗口的右下部分。宽度和高度与第二个视口一样。跟第二个视口不同的是glViewport()函数的第二个参数为0.
        //这告诉程序我们想让视口位于主窗口的右下部分。
        //透视视图的设置同第二个视图。
        if (loop==2)                                    // 绘制右下角视口
        {
            glViewport (window_width/2, 0, window_width/2, window_height/2);
            glMatrixMode (GL_PROJECTION);
            glLoadIdentity ();
            gluPerspective( 45.0, (GLfloat)(mazeWidth)/(GLfloat)(mazeHeight), 0.1f, 500.0 );
        }
        //如果loop等于3,我们就画最后一个视口(第四个视口)。它将位于窗口的左下部分。宽度和高度跟前几次设置一样。唯一跟第三个视口不同的是glViewport()的第一个参数为0.
        //这告诉程序视口将在主窗口的左下部分。
        //透视投影视图设置同第二个视口。
        if (loop==3)                                    // 绘制右下角视口
        {
            glViewport (0, 0, window_width/2, window_height/2);
            glMatrixMode (GL_PROJECTION);
            glLoadIdentity ();
            gluPerspective( 45.0, (GLfloat)(mazeWidth)/(GLfloat)(mazeHeight), 0.1f, 500.0 );
        }
        //下面的代码选择模型视图矩阵为当前矩阵真，并重置它。然后清空深度缓存。我们在每个视口画之前清空深度缓存。
        //注意到我们没有清除屏幕颜色，只是深度缓存！如果你没有清除深度缓存，你将看到物体的部分消失了，等等，很明显不美观！
        glMatrixMode (GL_MODELVIEW);
        glLoadIdentity ();
        glClear (GL_DEPTH_BUFFER_BIT);
        //我们要画的第一副图为一个平坦的2维纹理方块。这个方块是在平行投影模式下画的，并且将会覆盖整个视口。
        //因为我们用了平行投影投影模式，这儿没有第三维了，因此没必要在z轴进行变换。
        //记住我们第一个视口的左上角坐标维0,0,右下部分坐标为window_width/2,window_height/2.
        //这意味我们的四边形的右上坐标为window_width/2,0,左上坐标为0,0,左下坐标为0,window_height/2.右下坐标为window_width/2,window_height/2.
        //请注意在平行投影投影模式下，我们能在象素级别上处理而不是单元级别(决定于我们的视口设置)
        if (loop==0)                                    // 绘制左上角的视图
        {
            glBegin(GL_QUADS);
                glTexCoord2f(1.0f, 0.0f); glVertex2i(window_width/2, 0              );
                glTexCoord2f(0.0f, 0.0f); glVertex2i(0,              0              );
                glTexCoord2f(0.0f, 1.0f); glVertex2i(0,              window_height/2);
                glTexCoord2f(1.0f, 1.0f); glVertex2i(window_width/2, window_height/2);
            glEnd();
        }
        //第二个要画的图像是一个带光照的平滑球体。第二个视图是带透视的，因此我们首先必须做的是往屏幕里平移14个单位，然后在x，y，z轴旋转物体。
        //我们激活光照，画球体，然后关闭光照。这个球体半径为4个单元长度，围绕z轴的细分度为32,沿z轴的细分度也为32.
        //如果你还在犯迷糊，可以试着改变stacks或者slices的值为更小。通过减小stacks/slices的值，你就减少了球体的平滑度。
        //纹理坐标是自动产生的！
        if (loop==1)                                    // 绘制右上角的视图
        {
            glTranslatef(0.0f,0.0f,-14.0f);
            glRotatef(m_xrot,1.0f,0.0f,0.0f);
            glRotatef(m_yrot,0.0f,1.0f,0.0f);
            glRotatef(m_zrot,0.0f,0.0f,1.0f);
            glEnable(GL_LIGHTING);
            gluSphere(m_quadric,4.0f,32,32);
            glDisable(GL_LIGHTING);
        }
        //要画的第三幅图跟第一幅一样。但是是带透视的。它贴到屏幕有一定的角度并且有旋转。
        //我们把它往屏幕里移动2个单位。然后往后倾斜那个方块45度角。这让方块的顶部远离我们，而方块的底部则更靠近我们。
        //然后在z轴方向上旋转方块。画方块时，我们需要手工设置贴图坐标。
        if (loop==2)                                    // 绘制右下角的视图
        {
            glTranslatef(0.0f,0.0f,-2.0f);
            glRotatef(-45.0f,1.0f,0.0f,0.0f);
            glRotatef(m_zrot/1.5f,0.0f,0.0f,1.0f);

            glBegin(GL_QUADS);
                glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, 0.0f);
                glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, 0.0f);
                glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
                glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, 0.0f);
            glEnd();
        }
        //如果我们在画第四副图，我们往屏幕里移动7个单位。然后把物体绕x,y,z轴旋转。
        //我们激活光照给物体一些不错的阴影效果，然后在z轴上平移-2个单位。我们这样做的原因是让物体绕自己的中心旋转而不是绕某一端。这圆柱体两端宽1.5个单位。
        //长度为4个单位并且绕轴上细分32个面片，沿轴细分16个面片。
        //为了能绕中心旋转，我们需要平移柱体长度的一半，4的一半也即是2。
        //在平移，旋转，然后再平移之后，我们画圆柱体，之后关闭光照。
        if (loop==3)                                    // 绘制左下角的视图
        {
            glTranslatef(0.0f,0.0f,-7.0f);
            glRotatef(-m_xrot/2,1.0f,0.0f,0.0f);
            glRotatef(-m_yrot/2,0.0f,1.0f,0.0f);
            glRotatef(-m_zrot/2,0.0f,0.0f,1.0f);

            glEnable(GL_LIGHTING);
            glTranslatef(0.0f,0.0f,-2.0f);
            gluCylinder(m_quadric,1.5f,1.5f,4.0f,32,16);
            glDisable(GL_LIGHTING);
        }
    }
    //最后要做的事就是清空渲染管道。
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
        case Qt::Key_Space:
        {
            reset();
            break;
        }
    }
    QGLWidget::keyPressEvent(event);
}

//大部分迷宫建设，以及旋转处理等工作都是在Update()函数中完成的。
//我们需要设置一个变量dir, 用它来表示记录随机的向上，向右，向下或向左值。
void MyGLWidget::timerEvent(QTimerEvent *event)
{
    //xrot,yrot和zrot通过和一些小浮点数相乘而随着时间的消逝而增加。这样我们可以让物体绕x轴，y轴和z轴旋转。每个变量都增加不同的值使旋转好看一点
    m_xrot+=(float)(15)*0.02f;
    m_yrot+=(float)(15)*0.03f;
    m_zrot+=(float)(15)*0.015f;
    //下面的代码用来检测我们是否画完了迷宫。我们开始设置done值为true, 然后循环检查每一个房间去看是否需要增加一面墙，如果有一间房还有被访问到，我们设置done为false.
    //如果tex_data[(x + (width*y))*3]的值为0, 我们就明白这个房间还没被访问到，而且没有在里面没有画一个象素。
    //如果这儿有一个象素,那么它的值为255。我们只需要检查它的颜色红色分量值。因为我们知道这个值只能为0(空)或者255(更新过)
    bool done = true;// 循环所有的纹理素，如果为0则表示没有绘制完所有的迷宫，返回
    for (int x=0; x<mazeWidth; x+=2)
    {
        for (int y=0; y<mazeHeight; y+=2)
        {
            if (m_tex_data[((x+(mazeWidth*y))*3)]==0)
                done=FALSE;
        }
    }
    //检查完所有的房间之后，如果done为true.那么迷宫就算建完了，SetWindowsText就会改变窗口的标题。我们改变标题为"迷宫建造完成！"。
    //然后我们停顿5000毫秒使看这个例子的人有时间来看标题栏上的字(如果在全屏状态，他们会看到动画停顿了)。
    if (done)                                        //如果完成停止五秒后重置
    {
        setWindowTitle("Maze Complete!");
        Sleep(5000);
        setWindowTitle("Building Maze!");
        reset();
    }
    else
    {
        setWindowTitle("Building Maze!");
    }
    //下面的代码也许让人看着糊涂，但其实并不难懂。我们检查当前房间的右边房间是否被访问过或者是否当前位置的右边是迷宫的右边界（当前房间右边的房间就不存在），
    //同样检查左边的房间是否访问过或者是否达到左边界。其它方向也作如此检查。
    //如果房间颜色的红色分量的值为255,就表示这个房间已经被访问过了(因为它已经被函数UpdateTex更新过)。如果mx(当前x坐标）小于2, 就表示我们已经到了迷宫最左边不能再左了。
    //如果往四个方向都不能移动了或以已经到了边界，就给mx和my一个随机值，然后检查这个值对应点是否被访问，如果没有，我们就重新寻找一个新的随机变量，
    //直到该变量对应的单元早已经被访问。因为需要从旧的路径中分叉出新的路径，所以我们必须保持搜素知道发觉有一老的路径可以从那里开始新的路径。
    //为了使代码尽量简短，我没有打算去检查mx-2是否小于0。如果你想有100%的错误检测，你可以修改这段代码阻止访问不属于当前贴图的内存。
    //检测是否走过这里
    if (((m_tex_data[(((m_mx+2)+(mazeWidth*m_my))*3)]==255) || m_mx>(mazeWidth-4)) &&
            ((m_tex_data[(((m_mx-2)+(mazeWidth*m_my))*3)]==255) || m_mx<2) &&
        ((m_tex_data[((m_mx+(mazeWidth*(m_my+2)))*3)]==255) || m_my>(mazeHeight-4))
            && ((m_tex_data[((m_mx+(mazeWidth*(m_my-2)))*3)]==255) || m_my<2))
    {
        do
        {
            m_mx=int(rand()%(mazeWidth/2))*2;
            m_my=int(rand()%(mazeHeight/2))*2;
        }
        while (m_tex_data[((m_mx+(mazeWidth*m_my))*3)]==0);
    }
    //下面这行代码赋给dir变量0-3之间的随机值，这个值告诉我们该往右，往上，往左还是往下画迷宫。
    //在得到随机的方向之后，我们检查dir的值是否为0(往右移)，如果是并且我们不在迷宫的右边界，
    //然后检查当前房间的右边房间，如果没被访问，我们就调用UpdateTex(mx+1,my)在两个房间之间建立一堵墙，然后mx增加2移到新的房间.
    int dir=int(rand()%4);// 随机一个走向

    if ((dir==0) && (m_mx<=(mazeWidth-4)))                            // 向右走，更新数据
    {
        if (m_tex_data[(((m_mx+2)+(mazeWidth*m_my))*3)]==0)
        {
            updateTex(m_mx+1,m_my);
            m_mx+=2;
        }
    }
    //如果dir的值为1(往下)，并且我们不在迷宫底部，我们检查当前房间的下面房间是否被访问过。
    //如果没被访问过，我们就在两个房间(当前房间和当前房间下面的房间)建立一堵墙。然后my增加2移到新的房间.
    if ((dir==1) && (m_my<=(mazeHeight-4)))                            //  向下走，更新数据
    {
        if (m_tex_data[((m_mx+(mazeWidth*(m_my+2)))*3)]==0)
        {
            updateTex(m_mx,m_my+1);
            m_my+=2;
        }
    }
    //如果dir的值为2(向左)并且我们不在左边界，我们就检查左边的房间是否被访问，如果没被访问，我们也在两个房间(当前房间和左边的房间)之间建立一堵墙，然后mx减2移到新的房间.
    if ((dir==2) && (m_mx>=2))                                // 向左走，更新数据
    {
        if (m_tex_data[(((m_mx-2)+(mazeWidth*m_my))*3)]==0)
        {
            updateTex(m_mx-1,m_my);
            m_mx-=2;
        }
    }
    //如果dir的值为3并且不在迷宫的最顶部，我们检?榈鼻胺考涞纳厦媸欠癖环梦剩绻挥校蛟诹礁龇考?(当前房间和当前房间上面个房间)之间建立一堵墙，然后my增加2移到新的房间。
    if ((dir==3) && (m_my>=2))                                // 向上走，更新数据
    {
        if (m_tex_data[((m_mx+(mazeWidth*(m_my-2)))*3)]==0)
        {
            updateTex(m_mx,m_my-1);
            m_my-=2;
        }
    }
    //移到新的房间后，我们必须标志当前房间为正在访问状态。我们通过调用以当前位置mx, my为参数的UpdateTex()函数来达到这个目的。
    updateTex(m_mx,m_my);                                    // 更新纹理
    updateGL();
    QGLWidget::timerEvent(event);
}

//下面的小段代码设置纹理中位置dmx,dmy的颜色值为纯白色。tex_data是指向我们的纹理数据的指针。
//每一个象素都由3字节组成(1字节红色分量，1字节绿色分量，一字节兰色分量). 红色分量的偏移为0，
//我们要修改的象素的在纹理数据中的偏移为dmx(象素的x坐标）加上dmy(象素y坐标)与贴图宽度的乘积,最后的结果乘3(3字节每象素)。
//下面第一行代码设置red(0)颜色分量为255, 第二行设置green(1)颜色分量为255,最后一行设置blue(2)颜色分量为255,最后的结果为在dmx,dmy处的象素颜色为白色。
void MyGLWidget::updateTex(int dmx, int dmy) // 更新纹理
{
   m_tex_data[0+((dmx+(mazeWidth*dmy))*3)]=255;                        // 设置颜色为白色
   m_tex_data[1+((dmx+(mazeWidth*dmy))*3)]=255;
   m_tex_data[2+((dmx+(mazeWidth*dmy))*3)]=255;
}

//重置有相当多的工作量。它清空纹理，给每一个视口设置随机颜色，删除迷宫中的墙并为迷宫的生成设置新的随机起点。
//第一行代码清空tex_data指向的贴图数据。我们需要清空width(贴图宽）*height(贴图高)*3(红，绿，兰)。
//（代码已经够清楚了，呜呼，干吗要翻译这段？） 清空内存空间就是设置所有的字节为0。如果3个颜色分量都清零，那么整个贴图就完全变黑了！
void MyGLWidget::reset()
{
    memset(m_tex_data, 0, mazeWidth*mazeHeight*3);
    //现在我们来给每一个视口设置随机的颜色。对于不了解这些的人来说，这里的随机并不是真正那种随机！
    //如果你写了一个简单的程序来打印出10个随机数字，每次你运行程序，你都会得到同样的10个数字。为了使事情（看起来）更加随机，我们可以设置随机数种子。
    //同样的，如果你设置种子为1,你总是会得到同样的结果。然而，如果我们设置srand为开机后当前时钟计数(这可能是任意的数)，我们的程序每次运行都会有不同的结果。
    //我们有四个视口，因此我们需要从0-3的循环来处理。我们给每一个颜色(red,green,blue)从128-255中间的随机值。
    //要加128的目的是需要更亮的颜色。最小值为0，最大值为255,而128则表示大约有50%的亮度。
    qsrand(QTime::currentTime().msecsSinceStartOfDay());
    for (int loop=0; loop<4; loop++)// 循环随机生成颜色
    {
        m_r[loop]=rand()%128+128;
        m_g[loop]=rand()%128+128;
        m_b[loop]=rand()%128+128;
    }
    //下一步，我们设置一个随机的起点。我们的起点必须是一个房间。在纹理中每两个象素就是一个房间。
    //为确保起点是房间而不是墙，我们在0至贴图宽度一半的范围内挑选一个数，并和2相乘。
    //通过这种方法我们只能得到如0,2,6,8之类的数，也就是说我们总是得到一个随机的房间，决不会着陆到一堵墙上如1,3,5,7,9等等。
    m_mx=int(rand()%(mazeWidth/2))*2;
    m_my=int(rand()%(mazeHeight/2))*2;
}

//希望这个教程能解答所有你在做多视口中碰到的任何问题。代码并不难懂。它几乎跟标准的基本代码没什么区别。
//我们唯一真正修改的是视口设置是在画的主循环中。在所有视口画之前清空一次屏幕，然后清空各自深度缓存。
//你可以用这些代码来在各自的视口中显示各种各样的图片，或在多视图中显示特定的物体。要做什么起决于你自己
//我希望你们喜欢这个教程...如果你发现代码中的任何错误，或者你感觉你能让这个教程更好，请通知我(同样的，如果你看过我的翻译，发现有不当之处，请通知我)
