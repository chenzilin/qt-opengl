#include "myglwidget.h"

//物理模拟简介:
//还记得高中的物理吧，直线运动，自由落体运动，弹簧。在这一课里，我们将创造这一切。

//现在我们已经有了一个简单的物理模拟引擎了，它包含有物体和模拟两个类，下面我们基于它们创建三个具体的模拟对象:
//1. 具有恒定速度的物体
//2. 具有恒定加速度的物体
//3. 具有与距离成反比的力的物体
//在程序中控制一个模拟对象：
//在我们写一个具体的模拟类之前，让我们看看如何在程序中模拟一个对象，在这个教程里，
//模拟引擎和操作模拟的程序在两个文件里，在程序中我们使用如下的函数，操作模拟：

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false)
{
    showNormal();
    m_constantVelocity = new ConstantVelocity();
    m_motionUnderGravitation =  new MotionUnderGravitation(Vector3D(0.0f, -9.81f, 0.0f));
    m_massConnectedWithSpring = new MassConnectedWithSpring(2.0f);
    m_slowMotionRatio = 10.0f;
    m_timeElapsed = 0;
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(1, &m_texture[0]);
    glDeleteLists(m_base, 256);
}

void MyGLWidget::resizeGL(int w, int h)
{
    glViewport (0, 0, (GLsizei)(w), (GLsizei)(h));				// Reset The Current Viewport
    glMatrixMode (GL_PROJECTION);										// Select The Projection Matrix
    glLoadIdentity();													// Reset The Projection Matrix
    gluPerspective(45.0f, (GLfloat)(w)/(GLfloat)(h),			// Calculate The Aspect Ratio Of The Window
                   1.0f, 100.0f);
    glMatrixMode (GL_MODELVIEW);										// Select The Modelview Matrix
    glLoadIdentity();													// Reset The Modelview Matrix
}

void MyGLWidget::initializeGL()
{
    loadGLTexture();
    buildFont();                                                // Build The Font
    glClearColor (0.0f, 0.0f, 0.0f, 0.5f);						// Black Background
    glClearDepth(1.0);                                          // 设置深度缓存
    glDepthFunc(GL_LEQUAL);                                     // 所作深度测试的类型
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);                           // 设置混合因子
    glShadeModel (GL_SMOOTH);									// Select Smooth Shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);			// Set Perspective Calculations To Most Accurate
}

void MyGLWidget::paintGL()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity ();
    // Reset The Modelview Matrix
    // Position Camera 40 Meters Up In Z-Direction.
    // Set The Up Vector In Y-Direction So That +X Directs To Right And +Y Directs To Up On The Window.
    gluLookAt(0, 0, 40, 0, 0, 0, 0, 1, 0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer
    // Drawing The Coordinate Plane Starts Here.
    // We Will Draw Horizontal And Vertical Lines With A Space Of 1 Meter Between Them.
    glColor3ub(0, 0, 255);										// Draw In Blue
    glBegin(GL_LINES);

    // Draw The Vertical Lines
    for (float x = -20; x <= 20; x += 1.0f)						// x += 1.0f Stands For 1 Meter Of Space In This Example
    {
        glVertex3f(x, 20, 0);
        glVertex3f(x,-20, 0);
    }

    // Draw The Horizontal Lines
    for (float y = -20; y <= 20; y += 1.0f)						// y += 1.0f Stands For 1 Meter Of Space In This Example
    {
        glVertex3f( 20, y, 0);
        glVertex3f(-20, y, 0);
    }

    glEnd();
    // Drawing The Coordinate Plane Ends Here.

    // Draw All Masses In constantVelocity Simulation (Actually There Is Only One Mass In This Example Of Code)
    glColor3ub(255, 0, 0);										// Draw In Red
    int a;
    for (a = 0; a < m_constantVelocity->numOfMasses; ++a)
    {
        Mass* mass = m_constantVelocity->getMass(a);
        Vector3D* pos = &mass->pos;

        glPrint(pos->x  * (640/40) + 320, 240 + pos->y * (480/40), pos->z, "Mass with constant vel");

        glPointSize(4);
        glBegin(GL_POINTS);
            glVertex3f(pos->x, pos->y, pos->z);
        glEnd();
    }
    // Drawing Masses In constantVelocity Simulation Ends Here.

    // Draw All Masses In motionUnderGravitation Simulation (Actually There Is Only One Mass In This Example Of Code)
    glColor3ub(255, 255, 0);									// Draw In Yellow
    for (a = 0; a < m_motionUnderGravitation->numOfMasses; ++a)
    {
        Mass* mass = m_motionUnderGravitation->getMass(a);
        Vector3D* pos = &mass->pos;

        glPrint(pos->x * (640/40) + 320, 240 + pos->y * (480/40), pos->z, "Motion under gravitation");

        glPointSize(4);
        glBegin(GL_POINTS);
            glVertex3f(pos->x, pos->y, pos->z);
        glEnd();
    }
    // Drawing Masses In motionUnderGravitation Simulation Ends Here.

    // Draw All Masses In massConnectedWithSpring Simulation (Actually There Is Only One Mass In This Example Of Code)
    glColor3ub(0, 255, 0);										// Draw In Green
    for (a = 0; a < m_massConnectedWithSpring->numOfMasses; ++a)
    {
        Mass* mass = m_massConnectedWithSpring->getMass(a);
        Vector3D* pos = &mass->pos;

        glPrint(pos->x * (640/40) + 320 , 240 + pos->y * (480/40), pos->z, "Mass connected with spring");

        glPointSize(8);
        glBegin(GL_POINTS);
            glVertex3f(pos->x, pos->y, pos->z);
        glEnd();

        // Draw A Line From The Mass Position To Connection Position To Represent The Spring
        glBegin(GL_LINES);
            glVertex3f(pos->x, pos->y, pos->z);
            pos = &m_massConnectedWithSpring->connectionPos;
            glVertex3f(pos->x, pos->y, pos->z);
        glEnd();
    }
    glLoadIdentity();								// 重置视口
    // Drawing Masses In massConnectedWithSpring Simulation Ends Here.
    glColor3ub(255, 255, 255);
    QString timeStr = QString("Time elapsed (seconds):%1").arg(m_timeElapsed);
    QString slowMotionStr = QString("Slow motion ratio:%1").arg(m_slowMotionRatio);
    glPrint(40, 450, 0, timeStr.toLatin1().data());// Print timeElapsed
    glPrint(40, 440, 0, slowMotionStr.toLatin1().data());// Print slowMotionRatio
    glPrint(40, 430, 0, "Press F3 for normal motion");
    glPrint(40, 420, 0, "Press F4 for slow motion");
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
        case Qt::Key_F3:
        {
            m_slowMotionRatio = 1.0f;
            break;
        }
        case Qt::Key_F4:
        {
            m_slowMotionRatio = 10.0f;
            break;
        }
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
    update(15);
    updateGL();
    QGLWidget::timerEvent(event);
}

//接下来是读取纹理代码。这跟前面纹理影射教程中的一模一样。
//下面的代码同样对之前教程的代码改动很小。如果你不清楚下面每行的用途，回头复习一下。
//注意TextureImage[ ]将保存2个rgb图像记录。复查处理读取或存储纹理的纹理很重要。一个错误的数字可能导致内存溢出或崩溃！
void MyGLWidget::loadGLTexture()
{
    QImage fontImage(":/image/Font.bmp");
    fontImage = fontImage.convertToFormat(QImage::Format_RGB888);
    fontImage = fontImage.mirrored();
    glGenTextures(1, &m_texture[0]);
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fontImage.width(), fontImage.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, fontImage.bits());
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
                glVertex2i(8,0);				// 右下角的坐标
                //第三个纹理坐标在我们的字符的最右边，但上移了纹理的1/16（刚好一个字符高）。这将是一个单独字符的右上角。
                glTexCoord2f(cx+0.0625f,1-cy);		// 右上角的纹理坐标
                glVertex2i(8,8);				// 右上角的坐标
                //最后我们左移来设置字符左上角的最后一个纹理坐标。
                glTexCoord2f(cx,1-cy);			// 左上角的纹理坐标
                glVertex2i(0,8);				// 左上角的坐标
            glEnd();						// 四边形字符绘制完成
        glTranslated(8,0,0);					// 绘制完一个字符，向右平移16个单位
        glEndList();							// 字符显示列表结束
    }// 循环建立256个显示列表
}

void MyGLWidget::glPrint(GLint x, GLint y, GLint z, char *string, int set)
{
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
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
    glTranslated(x,y,z);							// 把字符原点移动到(x,y)位置
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
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

//这个函数在每一帧的开始更新，参数为相隔的时间。
void MyGLWidget::update(quint32 milliseconds)// 执行模拟
{
    // dt Is The Time Interval (As Seconds) From The Previous Frame To The Current Frame.
    // dt Will Be Used To Iterate Simulation Values Such As Velocity And Position Of Masses.

    float dt = milliseconds / 1000.0f;// 转化为秒

    dt /= m_slowMotionRatio;// 除以模拟系数

    m_timeElapsed += dt;// 更新流失的时间

    //在下面的代码中，我们定义一个处理间隔，没隔这么长时间，让物理引擎模拟一次。
    float maxPossible_dt = 0.1f;// 设置模拟间隔

    int numOfIterations = (int)(dt / maxPossible_dt) + 1;//计算在流失的时间里模拟的次数
    if (numOfIterations != 0)									// Avoid Division By Zero
        dt = dt / numOfIterations;								// dt Should Be Updated According To numOfIterations

    for (int a = 0; a < numOfIterations; ++a)// 模拟它们
    {
        m_constantVelocity->operate(dt);
        m_motionUnderGravitation->operate(dt);
        m_massConnectedWithSpring->operate(dt);
    }
}

//好了上面就是一个简单的物理模拟，希望你能喜欢：）
