#include "myglwidget.h"

//拾取, Alpha混合, Alpha测试, 排序:
//这又是一个小游戏，交给的东西会很多，慢慢体会吧
//欢迎来到32课. 这课大概是在我所写作已来最大的一课. 超过1000 行代码和约1540行的HTML. 这也是第一课用到我新的NeHeGL 基本代码.
//这课写了很长时间, 但我想他是值得期待的. 一些知识点用到是: Alpha 混合, Alpha 测试, 读取鼠标, 同时用到Ortho 和透视,
//显示客户鼠标, 按深度排列物体, 动画帧从单张材质图 和更多要点, 你将学到更多精选的内容!
//最初的版本是在屏幕上显示三个物体，当你单击他们将改变颜色. 很有趣!?! 不怎样! 象往常一样,
//我想给你们这些家伙留下一个超极好的课程. 我想使课程有趣, 丰富，当然..美观. 所以, 经过几个星期的编码之后,
//这课程完成了! 即使你不编码，你仍会喜欢这课. 这是个完整的游戏. 游戏的目标是射击更多的靶子, 在你失去一定数的靶子后，
//你将不能再用鼠标单击物体.
//我确信会有批评，但我非常乐观对这课! 我已在从深度里选择和排序物体这个主题里找到快乐!
//一些需要注意的代码. 我仅仅会在lesson32.cpp里讨论. 有一些不成熟的改动在 NeHeGL 代码里.
//最重要的改动是我加入鼠标支持在 WindowProc(). 我也加入 int mouse_x, mouse_y 在存鼠标运动.
//在 NeHeGL.h 以下两条代码被加入: extern int mouse_x; & extern int mouse_y;
//课程用到的材质是用 Adobe Photoshop 做的. 每个 .TGA 文件是32位图片有一个alpha 通道.
//若你不确信自已能在一个图片加入alpha通道, 找一本好书，上网，或读 Adobe Photoshop帮助.
//全部的过程非常相似，我做了透明图在透明图课程. 调入你物体在 Adobe Photoshop (或一些其它图形处理程序，且支持alpha 通道).
//用选择颜色工具选你图片的背景. 复制选区. 新建一个图. 粘贴生成新文件. 取消图片选择，你图的背景应是黑色. 使周围是白色.
//选全部图复制. 回到最初的图且建一个alpha 通道. 粘贴黑和白透明图你就完成建立alpha通道.存图片为32位t .TGA文件.
//使确定保存透明背景是选中的，保存!
//如以往我希望你喜欢这课程. 我感兴趣你对他的想法. 若你有些问题或你发现一些问题,告诉我. 我匆忙的完成这课程
//所以若你发现哪部分很难懂，给我发些邮件,然后我会用不同的方式或更详细的解释!

typedef int (*compfn)(const void*, const void*);// Typedef For Our Compare Function

//这些代码调用排序程序. 它比较距离在两个结构并返回-1 若第一个结构的距离小于第二个 ,
//1 i若 第一个结构的距离大于第二个 0 否则 (若 距离相等)
int Compare(struct objects *elem1, struct objects *elem2)// 比较 函数
{
   if ( elem1->distance < elem2->distance)
   {
      // 若 第一个结构的距离小于第二个
      return -1;// 返回 -1
   }
   else if (elem1->distance > elem2->distance)
   {
       // 若 第一个结构的距离大于第二个
       return 1;// 返回1
   }
   else
   {
       // 否则 (若 距离相等)
       return 0;// 返回 0
   }
}

// 每个物体的大小: 蓝面, 水桶, 靶子, 可乐, 瓶子
const static dimensions dimensionSize[5] = { {1.0f,1.0f}, {1.0f,1.0f}, {1.0f,1.0f}, {0.5f,1.0f}, {0.75f,1.5f} };

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_level(1), m_miss(0), m_kills(0),
    m_score(0),m_game(false)
{
    showNormal();
    startTimer(15);
    QPixmap cursorImg(":/image/Crosshair.tga");
    cursorImg = cursorImg.scaled(32, 32);
    setCursor(QCursor(cursorImg));
}

MyGLWidget::~MyGLWidget()
{
    glDeleteLists(m_base, 95);// 删掉所有95 字体显示列表
    for(int i = 0, iend = 10; i < iend; i++)
    {
        glDeleteTextures(1, &m_textures[i].texID);
    }
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
    gluPerspective(45.0f,(GLfloat)w/(GLfloat)h,1.0f,100.0f);

    glMatrixMode(GL_MODELVIEW);	//选择模型观察矩阵
    glLoadIdentity(); // 重置模型观察矩阵
}

//初始化的代码总是一样的. 首先的现两行取得我们window 的消息和我们建盘消息.
//然后我们用 srand() 建一个基于时间的多样化的游戏. 之后我们调入 TGA 图片并用LoadTGA()转换到材质 .
//先前的 5个图片是将穿过屏幕的物体. Explode 是我们爆炸动画, 大地和天空 弥补现场背景,
//crosshair是你在屏幕上看到表现鼠标当前位置的十字光标, 最后, 用来显示分数，标题和士气值的字体的图片.
//若任何调入图片的失误,则到返回 FALSE 值, 并程序结束. 值得注意的是这些基本代码不是返回整数型(INIT)的 FAILED 错误消息.
void MyGLWidget::initializeGL()
{
    qsrand( QTime::currentTime().elapsed() );// 使随机化事件
    loadTGA(&m_textures[0],":/image/BlueFace.tga");// 调入蓝面材质
    loadTGA(&m_textures[1],":/image/Bucket.tga");// 调入水桶材质
    loadTGA(&m_textures[2],":/image/Target.tga");// 调入靶子材质
    loadTGA(&m_textures[3],":/image/Coke.tga");//调入可乐材质
    loadTGA(&m_textures[4],":/image/Vase.tga");//调入花瓶材质
    loadTGA(&m_textures[5],":/image/Explode.tga");//调入爆炸材质
    loadTGA(&m_textures[6],":/image/Ground.tga");// 调入地面材质
    loadTGA(&m_textures[7],":/image/Sky.tga");// 调入天空材质
    loadTGA(&m_textures[8],":/image/Crosshair.tga");// 调入十字光标材质
    loadTGA(&m_textures[9],":/image/Font.tga");// 调入字符材质

    //若所有图片调入成功则轮到材质, 我们能继续初始化. 字体材质被调入, 因而保险能建立我们的字体. 我们跳入BuildFont()来做这些.
    //然后我们设置OpenGL. 背景色为黑, alpha 也设为0.0f. 深度缓冲区设为激活小于或等于测试.
    //glBlendFunc() 是很重要的一行代码. 我们设混合函数(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA).
    //这些加上alpha变量的屏幕上的混合物体存在物体的材质. 在设置混合模式之后, 我们激活blending(混合).
    //然后我们打开 2D 材质贴图, 最后,打开 GL_CULL_FACE. 这是去除每个物体的后面( 没有一点浪费在一些我们看不到的循环 ).
    //画一些四边形逆时针卷动 ,因而精致而适当的面片.
    //早先的教程我谈论使用glAlphaFunc()代替alpha 混合. 若你想用Alpha 函数,
    //注释出的两行混合代码和不注释的两行在glEnable(GL_BLEND)之下. 你也能注释出qsort()函数在 InitObject() 部分里的代码.
    //程序应该运行ok,但sky 材质将不在这. 因为sky的材质已是一个alpha 变量0.5f.
    //当早在我说关于Alpha函数, 我提及它只工作在alpha 变量0 或 1. 若你想它出现,你将不得不修改sky的材质alpha 通道!
    //再则, 若你决定用Alpha 函数代替, 你不得排序物体.两个方法都有好处! 再下而是从SGI 网站的快速引用:
    //"alpha 函数丢弃细节,代替画他们在结构缓冲器里. 因此排序原来的物体不是必须的 (除了一些其它像混合alpha模式是打开的).
    //不占优势的是象素必须完全白底或完全透明".


    buildFont();//建立我们的字体显示列表

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);// 黑色背景
    glClearDepth(1.0f);// 安装深度缓冲器
    glDepthFunc(GL_LEQUAL);// 深度的类型测试
    glEnable(GL_DEPTH_TEST);// 打开深度测试
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);// 打开 Alpha 混合
    glEnable(GL_BLEND);// 打开混合
//	glAlphaFunc(GL_GREATER,0.1f);// 设 Alpha 测试
//	glEnable(GL_ALPHA_TEST);// 打开 Alpha 测试
    glEnable(GL_TEXTURE_2D);// 打开材质贴图
    glEnable(GL_CULL_FACE);// 去掉画物体的背面

    // 循环在 30 个物体Objects
    for (int loop=0; loop<30; loop++)
    {
        initObject(loop);// 初始化每个物体
    }
}

void MyGLWidget::paintGL()
{
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// 弹出矩阵
    glLoadIdentity();// 重设矩阵

    //下面的代码绘制飘动的天空,它由四块纹理组成,每一块的移动速度都不一样,并把它们混合起来
    glPushMatrix();// Push The Modelview Matrix
        glBindTexture(GL_TEXTURE_2D, m_textures[7].texID);// 选天空的材质
        glBegin(GL_QUADS);// Begin Drawing Quads
            glTexCoord2f(1.0f,m_roll/1.5f+1.0f); glVertex3f( 28.0f,+7.0f,-50.0f);	// Top Right
            glTexCoord2f(0.0f,m_roll/1.5f+1.0f); glVertex3f(-28.0f,+7.0f,-50.0f);	// Top Left
            glTexCoord2f(0.0f,m_roll/1.5f+0.0f); glVertex3f(-28.0f,-3.0f,-50.0f);	// Bottom Left
            glTexCoord2f(1.0f,m_roll/1.5f+0.0f); glVertex3f( 28.0f,-3.0f,-50.0f);	// Bottom Right

            glTexCoord2f(1.5f,m_roll+1.0f); glVertex3f( 28.0f,+7.0f,-50.0f);		// Top Right
            glTexCoord2f(0.5f,m_roll+1.0f); glVertex3f(-28.0f,+7.0f,-50.0f);		// Top Left
            glTexCoord2f(0.5f,m_roll+0.0f); glVertex3f(-28.0f,-3.0f,-50.0f);		// Bottom Left
            glTexCoord2f(1.5f,m_roll+0.0f); glVertex3f( 28.0f,-3.0f,-50.0f);          // Bottom Right

            glTexCoord2f(1.0f,m_roll/1.5f+1.0f); glVertex3f( 28.0f,+7.0f,0.0f);     // Top Right
            glTexCoord2f(0.0f,m_roll/1.5f+1.0f); glVertex3f(-28.0f,+7.0f,0.0f);     // Top Left
            glTexCoord2f(0.0f,m_roll/1.5f+0.0f); glVertex3f(-28.0f,+7.0f,-50.0f);	// Bottom Left
            glTexCoord2f(1.0f,m_roll/1.5f+0.0f); glVertex3f( 28.0f,+7.0f,-50.0f);	// Bottom Right

            glTexCoord2f(1.5f,m_roll+1.0f); glVertex3f( 28.0f,+7.0f,0.0f);          // Top Right
            glTexCoord2f(0.5f,m_roll+1.0f); glVertex3f(-28.0f,+7.0f,0.0f);          // Top Left
            glTexCoord2f(0.5f,m_roll+0.0f); glVertex3f(-28.0f,+7.0f,-50.0f);		// Bottom Left
            glTexCoord2f(1.5f,m_roll+0.0f); glVertex3f( 28.0f,+7.0f,-50.0f);		// Bottom Right
        glEnd();// Done Drawing Quads

        //绘制地面
        glBindTexture(GL_TEXTURE_2D, m_textures[6].texID);// 大地材质
        glBegin(GL_QUADS);// Draw A Quad
            glTexCoord2f(7.0f,4.0f-m_roll); glVertex3f( 27.0f,-3.0f,-50.0f);// Top Right
            glTexCoord2f(0.0f,4.0f-m_roll); glVertex3f(-27.0f,-3.0f,-50.0f);// Top Left
            glTexCoord2f(0.0f,0.0f-m_roll); glVertex3f(-27.0f,-3.0f,0.0f);// Bottom Left
            glTexCoord2f(7.0f,0.0f-m_roll); glVertex3f( 27.0f,-3.0f,0.0f);// Bottom Right
        glEnd();// Done Drawing Quad

        drawTargets();// 画我们的靶子
    glPopMatrix();// Pop The Modelview Matrix

    // 十字光标 (在光标里)
    QRect windowRect = this->rect();// 取窗口位置
    glMatrixMode(GL_PROJECTION);// Select The Projection Matrix
    glPushMatrix();// Store The Projection Matrix
        glLoadIdentity();// Reset The Projection Matrix
        glOrtho(0, windowRect.right(), 0, windowRect.bottom(), -1, 1);// 设置为正投影
        glMatrixMode(GL_MODELVIEW);// Select The Modelview Matrix
        //glTranslated(m_mousePoint.x(), windowRect.bottom()-m_mousePoint.y(), 0.0f);// 移动到当前鼠标位置
        //object(16,16,8);// 画十字光标

        // 游戏状态 / 标题名称
        glPrint(240,450,"NeHe Productions");// 输出 标题名称
        QString levelStr = QString("Level:%1").arg(m_level);
        glPrint(10, 10, levelStr);// 输出 等级
        QString scoreStr = QString("Score:%1").arg(m_score);
        glPrint(250, 10, scoreStr);// 输出 分数

        //在下面的代码里, 我们查看若game 是TRUE. 若 game 是TRUE, 我们输出 ’GAME OVER’游戏结束的消息.
        //若game 是false, 我们输出 玩家的士气morale (到10溢出). 士气morale是被设计用来从10减去玩家失误的次数(miss) .
        //玩家失掉的越多, 士气越低.
        if (m_game)// 游戏是否结束?
        {
            glPrint(490, 10, "GAME OVER");// 结束消息
        }
        else
        {
            QString moraleStr = QString("Morale:%1/10").arg(10-m_miss);
            glPrint(490, 10, moraleStr);// 输出剩余生命
        }

        //最后做的事我们选投影矩阵, 恢复(取出) 我们的矩阵返回到前一个情形, 设矩阵模式为 modelview ，刷新缓冲区 ，
        //使所有物体被渲染.
        glMatrixMode(GL_PROJECTION);// Select The Projection Matrix
    glPopMatrix();// Restore The Old Projection Matrix
    glMatrixMode(GL_MODELVIEW);// Select The Modelview Matrix
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
            if(m_game)
            {
                for (int loop=0; loop<30; loop++)
                {
                    initObject(loop);// 初始化
                }// 循环所有的物体
                m_game=false;//设game为false
                m_score=0;// 分数为0
                m_level=1;// 级别为1
                m_kills=0;// 杀敌数为0
                m_miss=0;// 漏过数为0
            }
            break;
        }
    }
    QGLWidget::keyPressEvent(event);
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    update(15);
    if (m_miss>9)//如果丢失10个物体,游戏结束
    {
        m_miss=9;// Limit Misses To 10
        m_game=TRUE;// Game Over TRUE
    }
    updateGL();
    QGLWidget::timerEvent(event);
}

void MyGLWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_mousePoint = event->windowPos().toPoint();
        selection();
    }
    QGLWidget::mousePressEvent(event);
}

void MyGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_mousePoint = event->windowPos().toPoint();
    QGLWidget::mouseMoveEvent(event);
}

//我用TGA 图片的原因是他们是有alpha 通道的. 这个alpha 通道告诉 OpenGL 哪一部分图是透明的，哪一部分是白底.
//alpha 通道是被建立在图片处理程序, 并保存在.TGA图片里面. OpenGL 调入图片, 能用alpha 通道设置图片中每个象素透明的数量.
bool MyGLWidget::loadTGA(TextureImage *texture, const QString &filename)
{
    GLuint type;// 设置默认的 GL 模式 为 RBGA
    QImage image(filename);
    if(!image.hasAlphaChannel())
    {
        image = image.convertToFormat(QImage::Format_RGB888);
        type = GL_RGB;
    }
    else
    {
        image = image.convertToFormat(QImage::Format_RGBA8888);
        type = GL_RGBA;
    }
    image = image.mirrored();
    if(image.isNull())
    {
        return false;
    }
    texture->imageData = image;
    texture->height = image.height();
    texture->width = image.width();
    texture->bpp = image.format();

    // Build A Texture From The Data
    glGenTextures(1, &texture->texID);// 生成 OpenGL 材质 ID
    glBindTexture(GL_TEXTURE_2D, texture->texID);// 绑定我们的材质
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);// 线过滤器
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);// 线过滤器
    glTexImage2D(GL_TEXTURE_2D, 0, type, texture->width, texture->height, 0,
            type, GL_UNSIGNED_BYTE, texture->imageData.bits());

    return true;// 材质建立成功, 返回正确
}

//2D 字体材质代码同我已在前一课用的是一样的. 然而, 有一些小不同. 第一是你将看到仅仅唯一生成95 显示列表.
//若你看字体材质, 你会看到只有 95 字母计算空间在图片顶，左. 第二个事是你将通知分在16.0f 为 cx 和 我们只分在8.0f cy.
//我这样做的结果是因为字体材质是256 象素宽, 但仅仅一伴就高(128 象素). 所以计算cx 我们分为16.0f 和计算分cy 为一半(8.0f).
//若你不懂下面的代码, 回去读17课. 建立字体的代码的详细解释在第17课里!
void MyGLWidget::buildFont()// 建立我们字体显示列表
{
    m_base=glGenLists(95);// 建立 95 显示列表
    glBindTexture(GL_TEXTURE_2D, m_textures[9].texID);// 绑我们字体材质
    for (int loop=0; loop<95; loop++)// 循环在 95 列表
    {
        float cx=float(loop%16)/16.0f;// X 位置 在当前字母
        float cy=float(loop/16)/8.0f;// Y 位置 在当前字母

        glNewList(m_base+loop,GL_COMPILE);// 开始建立一个列表
            glBegin(GL_QUADS);// 用四边形组成每个字母
                glTexCoord2f(cx, 1.0f-cy-0.120f);
                glVertex2i(0,0);	// 质地 / 点 座标 (底 左)
                glTexCoord2f(cx+0.0625f, 1.0f-cy-0.120f);
                glVertex2i(16,0);	// 质地 / 点 座标 (底 右)
                glTexCoord2f(cx+0.0625f, 1.0f-cy);
                glVertex2i(16,16);  // 质地 / 点 座标 (顶 右)
                glTexCoord2f(cx, 1.0f-cy);
                glVertex2i(0,16);	// 质地 / 点 座标 (顶 左)
            glEnd();// 完成建立我们的 四边形 (字母)
            glTranslated(10,0,0);// 移到字体的右边
        glEndList();// 完成建军立这个显示列表
    }// 循环直到所有 256 完成建立
}

//输出的代码也在第17课, 但已修改为在屏幕输出我们的分数, 等级和士气(不断改变的值).
void MyGLWidget::glPrint(GLint x, GLint y, const QString &string)// 输出在屏慕的位置
{
    glBindTexture(GL_TEXTURE_2D, m_textures[9].texID);// 选择我们字体材质
    glPushMatrix();	// 存观看模式矩阵
        glLoadIdentity();// 设观看模式矩阵
        glTranslated(x,y,0);// 文字输出位置 (0,0 - 底 左-Bottom Left)
        glListBase(m_base-32);// 选择字体设置
        glCallLists(strlen(string.toLatin1().data()),
                    GL_UNSIGNED_BYTE, string.toLatin1().data());// 输出显示列表中的文字
    glPopMatrix();// 取出以前的模式矩阵
}

//InitObject() 代码是来建立每个物体. 我们开始设 rot 为 1. 这使物体顺时针旋转.
//然后设爆炸效果动画帧为0(我们不想爆炸效果从中间开始).我们下面设 hit 为 FALSE, 意思是物体还没被击中或正开如.
//选一个物体材质, texid 用来给一个随机的变量从 0 到 4. 0是blueface 材质 和4 是 vase 材质. 这给我们5种随机物体.
//距离变量是在-0.0f to -40.0f (4000/100 is 40)的随机数 . 当我们真实的画物体,我们透过在屏幕上的另10 个单位.
//所以当物体在画时, 他们将画从-10.0f to -50.0f 单位 在屏幕(不挨着, 也不离得太远). 我分随机数为 100.0f 得到更精确的浮点数值.
//在给完随机的距离之后, 我们给物体一个随机的 y . 我们不想物体低于 -1.5f, 否则他将低于大地, 且我们不想物体高于3.0f.
//所以留在我们的区间的随机数不能高于4.5f (-1.5f+4.5f=3.0f).
//去计算 x 位置, 用一些狡猾的数学. 用我们的距离减去15.0f . 除以2 减5*level. 再 减随机数（0.0f 到5） 乘level.
//减 5*level rndom(0.0f to 5*level) 这是最高级.
//选一个方向.
//使事情简单明白x, 写一个快的例子. 距离是 -30.0f ，当前级是 1:
//object[num].x=((-30.0f-15.0f)/2.0f)-(5*1)-float(rand()%(5*1));
//object[num].x=(-45.0f/2.0f)-5-float(rand()%5);
//object[num].x=(-22.5f)-5-{lets say 3.0f};
//object[num].x=(-22.5f)-5-{3.0f};
//object[num].x=-27.5f-{3.0f};
//object[num].x=-30.5f;
//开始在屏模上移 10 个单位 , 距离是 -30.0f. 其实是 -40.0f.用透视的代码在 NeHeGL.cpp 文件.
void MyGLWidget::initObject(int num)
{
    m_object[num].rot=1;// 顺时针旋转
    m_object[num].frame=0;// 设爆炸效果动画帧为0
    m_object[num].hit=false; // 设点击检测为0
    m_object[num].texid=qrand()%5;// 设一个材质
    m_object[num].distance=-(float(qrand()%4001)/100.0f);// 随机距离
    m_object[num].y=-1.5f+(float(qrand()%451)/100.0f);// 随机 Y 位置
    // 随机开始 X 位置 基于物体的距离 和随机的延时量 (确定变量)
    m_object[num].x=((m_object[num].distance-15.0f)/2.0f)-(5*m_level)-float(qrand()%(5*m_level));
    m_object[num].dir=(qrand()%2);// 选一个随机的方向

    //检查方向
    if (m_object[num].dir==0)// 若随机的方向正确
    {
        m_object[num].rot=2;// 逆时针旋转
        m_object[num].x=-m_object[num].x;// 开始在左边 (否定 变量)
    }

    //现在我们检查texid来找出所选的的物体. 若 texid 0
    //所选的物体是 Blueface . blueface 总是在大地上面旋转. 确定开始时在地上的层, 我们设 y 是 -2.0f.
    if (m_object[num].texid==0)// 蓝色天空表面
    {
        m_object[num].y=-2.0f;// 总是在大地上面旋转
    }

    //下面检查若texid 是 1. 这样, 电脑所选物体的是 Bucket. bucket不从左到右运动, 它从天上掉下来. 首先我们不得不设 dir 是 3.
    //这告诉电脑我们的水桶bucket 是掉下来或向下运动.
    //我们最初的代码假定物体从左到右运动. 因为bucket 是向下落的, 我们得不给它一个新的随机的变量 x . 若不是这样,
    //bucket 会被看不到. 它将不在左边落下就在屏幕外面. 我们给它一个新的随机距离变量在屏幕上. 代替减去15, 我们仅仅减去 10.
    //这给我们一些幅度, 保持物体在屏幕??. 设我们的distance 是-30.0f, 从0.0f -40.0f的随机变量. 为什么从 0.0f 到 40.0f?
    //不是从0.0f to -40.0f? 答案很简单. rand() 函数总返回正数. 所以总是正数. 另外，回到我们的故事. 我们有个正数
    //从0.0f 到 40.0f.我们加距离 最小 10.0f 除以 2. 举个例子，设x变量为 15 ，距离是 -30.0f:
    //object[num].x=float(rand()%int(-30.0f-10.0f))+((-30.0f-10.0f)/2.0f);
    //object[num].x=float(rand()%int(-40.0f)+(-40.0f)/2.0f);
    //object[num].x=float(15 {assuming 15 was returned))+(-20.0f);
    //object[num].x=15.0f-20.0f;
    //object[num].x=-5.0f;
    //下面设y. 我们想水桶从天上下来. 我人不想穿过云. 所以我们设 y 为 4.5f. 刚在去的下面一点.

    if (m_object[num].texid==1)// 水桶(Bucket)
    {
        m_object[num].dir=3;// 下落
        m_object[num].x=float(qrand()%int(m_object[num].distance-10.0f))+
                ((m_object[num].distance-10.0f)/2.0f);
        m_object[num].y=4.5f;// 随机 X, 开始在屏模上方
    }

    //我们想靶子从地面突出到天上. 我们检查物体为 (texid 是 2). 若是, 设方向(dir) 是 2 (上). 用精确的数 x 位置.
    //我们不想target 开始在地上. 设 y 初值为-3.0f (在地下). 然后减一个值从0.0f 到 5 乘当前 level.
    //靶子不是立即出现. 在高级别是有延时， 通过delay, 靶子将出现在一个在另一个以后, 给你很少时间打到他们.
    if (m_object[num].texid==2)// 靶子
    {
        m_object[num].dir=2;// 开始向上飞
        m_object[num].x=float(qrand()%int(m_object[num].distance-10.0f))
                +((m_object[num].distance-10.0f)/2.0f);
        m_object[num].y=-3.0f-float(qrand()%(5*m_level));// 随机 X, 开始在下面的大地 + 随机变量
    }

    //所有其它的物体从右到左旅行, 因而不必给任何变量付值来改变物体. 它们应该刚好工作在所给的随机变量.
    //现在来点有趣的材料! "为了alpha 混合技术正常的工作, 透明的原物必须不断地排定在从后向前画". 当画alpha 混合物体是,
    //在远处的物体是先画的,这是非常重要的, 下面画紧临的上面的物体.
    //理由是简单的... Z 缓冲区防止 OpenGL 从已画好的混合东西再画象素. 这就是为什么会发生物体画在透明混合之后而不再显示出来.
    //为什么你最后看到的是一个四边形与物体重叠... 很不好看!
    //我们已知道每个物体的深度. 因而在初始化一个物体之后, 我们能通过把物体排序，而用qsort 函数(快速排序sort)，来解决这个问题 .
    //通过物体排序, 我们能确信第一个画的是最远的物体. 这意味着当我们画物体时, 起始于第一个物体, 物体通过用距离将被先画.
    //紧挨着那个物体(晚一会儿画) 将看到先前的物体在他们的后面, 再将适度的混合!
    //这文中的这行线注释是我在 MSDN 里发现这些代码，在网上花时间查找之后找到的解答 . 他们工作的很好,允许各种的排序结构.
    //qsort 传送 4 个参数. 第一个参数指向物体数组 (被排序的数组d). 第二个参数是我们想排序数组的个数...
    //当然，我们想所有的排序的物体普遍的被显示(各个level). 第三个参数规定物体结构的大不,
    //第四个参数指向我们的 Compare() 函数.
    //大概有更好的排序结构的方法, 但是 qsort() 工作起来... 快速方便，简单易用!
    //这个是重要的知识点, 若你们想用 glAlphaFunc() 和 glEnable(GL_ALPHA_TEST), 排序是没必要的.
    //然而, 用Alpha 功能你被限制在完全透明或完全白底混合, 没有中间值.
    //用 Blendfunc()排序用一些更多的工作，但他顾及半透明物体.
    // 排序物体从距离:我们物体数组的开始地址 *** MSDN 代码修改为这个 TUT ***
    // 各种的数按
    // 各自的要素的
    // 指针比较的函数
    qsort((void *) &m_object, m_level, sizeof(struct objects), (compfn)Compare );
}

//现在为急速原始物体... 是实际被选?形锾宓拇?. 第一行为我们选择物体的信息分配内存.
//hits 是当选择时碰撞迅检测的次数.
void MyGLWidget::selection()
{
    GLuint	buffer[512];//  设选择缓冲
    GLint	hits;// 选择物体的数

    // 游戏是否结束?
    if (m_game)
    {
        return;
    }

    QSound::play(":/voice/Shot.wav");// 放音乐 Gun Shot

    //设视点. viewport[] 包括当前 x, y, 当前的视点(OpenGL Window)长度，宽度.
    //glGetIntegerv(GL_VIEWPORT, viewport) 取当前视点存在viewport[]. 最初，等于 OpenGL 窗口维数.
    //glSelectBuffer(512, buffer) 说 OpenGL 用这个内存.
    // 视点的大小. [0] 是 <x>, [1] 是 <y>, [2] 是 <length>, [3] 是 <width>
    GLint	viewport[4];

    // 这是设视点的数组在屏幕窗口的位置
    glGetIntegerv(GL_VIEWPORT, viewport);
    glSelectBuffer(512, buffer);// 告诉 OpenGL 使我们的数组来选择

    //存opengl的模式. 在这个模式什么也不画. 代替, 在选择模式物体渲染信息存在缓存.
    //下面初实化name 堆栈,通过调入glInitNames() 和glPushName(0). I它重要的是标记若程序不在选择模式,
    //一个到glPushName()调用将忽略. 当然在选择的模试, 但这一些是是紧记的.
    // 设 OpenGL 选择模式. 将不画东西. 物体 ID’的广度放在内存
    (void) glRenderMode(GL_SELECT);

    glInitNames();// 设名字堆栈
    glPushName(0);// Push 0 (最少一个) 在栈上

    //之后, 不得不限制在光标的下面画图. 为了做这些得用到投影矩阵. 然后把它推到堆栈中.重设矩阵则用到 glLoadIdentity().
    //用gluPickMatrix()限制的画. 第1个参数是当前鼠标的 x-座标, 第2个参数是当前鼠标的 y-座标, 然后宽和高的选区.
    //最后当前的 viewport[]. viewport[] 是指出视点的边界. x 和_y 将在选区的中心.
    glMatrixMode(GL_PROJECTION);// 选投影矩阵
    glPushMatrix();// 压入投影矩阵
        glLoadIdentity();// 重设矩阵

        //  这是建一个矩阵使鼠标在屏幕缩放
        gluPickMatrix((GLdouble) m_mousePoint.x(), (GLdouble) (viewport[3]-m_mousePoint.y()),
                1.0f, 1.0f, viewport);

        //调入 gluPerspective() 应用透视矩阵 ,被gluPickMatrix()选择矩阵限制所画区域 .
        //打开modelview 矩阵，调用DrawTargets()画我们的靶子. 画靶子在DrawTargets() 而不在 Draw()
        //是因为仅仅想选择物体的碰撞检测且, 不是天空，大地，光标.
        //之后, 打开回到发射矩阵， 从堆栈中弹出矩阵. 之扣打开回到modelview 矩阵.
        //最后一行，回到渲染模式 因而物体画的很真实的在屏幕上. hits 将采集gluPickMatrix()所需要取渲染的物体数 .
        // 应用透视矩阵
        gluPerspective(45.0f, (GLfloat) (viewport[2]-viewport[0])/(GLfloat) (viewport[3]-viewport[1]),
                0.1f, 100.0f);
        glMatrixMode(GL_MODELVIEW);// 选择模型变换矩阵
        drawTargets();// 画目标
        glMatrixMode(GL_PROJECTION);// 选择投影变换矩阵
    glPopMatrix();// 取出投影矩阵
    glMatrixMode(GL_MODELVIEW);// 选模式显示矩阵
    hits=glRenderMode(GL_RENDER);// 切换模式, 找出有多少
    //检查若多于0 个hits 记录. 若这样, 设choose 为 第一个物体的名子. depth 取得它有多远.
    //每个hit 分有4 个项目在内存. 第一，在名子堆栈上打击发生时的数字 .第二， 所选物体的最小z值.
    //第三，所选物体的最大 z 值, 最后，在同一时间里所选物体名子堆栈的内容 (物体的名子).
    //在这一课,我们仅对最小z值和物体名子感兴趣.
    if (hits > 0)// 若有大于0个 Hits
    {
        int	choose = buffer[3];// 选择第一物体
        int depth = buffer[1];// 存它有多远

        //做循环所有hits 使没有物体在第一个物体旁边. 否则, 两个物体会重叠, 一个物体碰到另一个.当你射击时,
        //重叠的物体会被误选.
        for (int loop = 1; loop < hits; loop++)// 循环所有检测到的物体
        {
            // 对于其它的物体
            if (buffer[loop*4+1] < GLuint(depth))
            {
                choose = buffer[loop*4+3];// 选择更近的物体
                depth = buffer[loop*4+1];// 保存它有多远
            }
        }

        //若物体被选.
        if (!m_object[choose].hit)
        {
            // 如果物体还没有被击中
            m_object[choose].hit=true;// 标记物体象被击中
            m_score+=1;// 增加分数
            m_kills+=1;// 加被杀数
            if (m_kills > m_level*5) // 已有新的级?
            {
                m_miss=0;// 失掉数回0
                m_kills=0;// 设 Kills数为0
                m_level+=1;// 加 Level
                if (m_level>30)// 高过 30?
                {
                    m_level = 30;// 设 Level 为 30 (你是 God 吗?)
                }
            }
        }
    }
}

// 这里用来更新
void MyGLWidget::update(quint32 milliseconds)// Perform Motion Updates Here
{
    m_roll-=milliseconds*0.00005f;// 云的旋转

    //下面的代码按物体的运动方向更新所有的运动
    for (int loop=0; loop<m_level; loop++)
    {
        if (m_object[loop].rot==1)// 若顺时针,则顺时针旋转
        {
            m_object[loop].spin-=0.2f*(float(loop+milliseconds));	// Spin Clockwise
        }

        if (m_object[loop].rot==2)// 若逆时针,则逆时针旋转
        {
            m_object[loop].spin+=0.2f*(float(loop+milliseconds));	// Spin Counter Clockwise
        }

        if (m_object[loop].dir==1)// 向右移动
        {
            m_object[loop].x+=0.012f*float(milliseconds);// Move Right
        }

        if (m_object[loop].dir==0)// 向左移动
        {
            m_object[loop].x-=0.012f*float(milliseconds);// Move Left
        }

        if (m_object[loop].dir==2)// 向上移动
        {
            m_object[loop].y+=0.012f*float(milliseconds);// Move Up
        }

        if (m_object[loop].dir==3)// 向下移动
        {
            m_object[loop].y-=0.0025f*float(milliseconds);// Move Down
        }

        // 如果到达左边界,你没有击中,则增加丢失的目标数
        if ((m_object[loop].x<(m_object[loop].distance-15.0f)/2.0f) &&
                (m_object[loop].dir==0) &&
                !m_object[loop].hit)
        {
            m_miss+=1;// Increase miss (Missed Object)
            m_object[loop].hit=true;// Set hit To True To Manually Blow Up The Object
        }

        // 如果到达右边界,你没有击中,则增加丢失的目标数
        if ((m_object[loop].x>-(m_object[loop].distance-15.0f)/2.0f) &&
                (m_object[loop].dir==1) &&
                !m_object[loop].hit)
        {
            m_miss+=1;// Increase miss (Missed Object)
            m_object[loop].hit=true;// Set hit To True To Manually Blow Up The Object
        }

        // 如果到达下边界,你没有击中,则增加丢失的目标数
        if ((m_object[loop].y<-2.0f) &&
                (m_object[loop].dir==3) &&
                !m_object[loop].hit)
        {
            m_miss+=1;// Increase miss (Missed Object)
            m_object[loop].hit=true;// Set hit To True To Manually Blow Up The Object
        }

        //如果到达左边界,你没有击中,则方向变为向下
        if ((m_object[loop].y>4.5f) &&
                (m_object[loop].dir==2))
        {
            m_object[loop].dir=3;// Change The Direction To Down
        }
    }
}

// Draw Object Using Requested Width, Height And Texture
void MyGLWidget::object(float width,float height,GLuint texid)
{
    glBindTexture(GL_TEXTURE_2D, m_textures[texid].texID);// 选合适的材质
    glBegin(GL_QUADS);// 开始画四边形
        glTexCoord2f(0.0f,0.0f); glVertex3f(-width,-height,0.0f);// Bottom Left
        glTexCoord2f(1.0f,0.0f); glVertex3f( width,-height,0.0f);// Bottom Right
        glTexCoord2f(1.0f,1.0f); glVertex3f( width, height,0.0f);// Top Right
        glTexCoord2f(0.0f,1.0f); glVertex3f(-width, height,0.0f);// Top Left
    glEnd();// Done Drawing Quad
}

//绘制爆炸的效果
void MyGLWidget::explosion(int num)
{
    float ex = (float)((m_object[num].frame/4)%4)/4.0f;// 计算爆炸时生成的x的纹理坐标
    float ey = (float)((m_object[num].frame/4)/4)/4.0f;// 计算爆炸时生成的y的纹理坐标

    glBindTexture(GL_TEXTURE_2D, m_textures[5].texID);// 选择爆炸的纹理
    glBegin(GL_QUADS);// Begin Drawing A Quad
        glTexCoord2f(ex,1.0f-(ey)); glVertex3f(-1.0f,-1.0f,0.0f);// Bottom Left
        glTexCoord2f(ex+0.25f,1.0f-(ey)); glVertex3f( 1.0f,-1.0f,0.0f);// Bottom Right
        glTexCoord2f(ex+0.25f,1.0f-(ey+0.25f)); glVertex3f( 1.0f, 1.0f,0.0f);// Top Right
        glTexCoord2f(ex,1.0f-(ey+0.25f)); glVertex3f(-1.0f, 1.0f,0.0f);// Top Left
    glEnd();// Done Drawing Quad

    //增加帧数,如果大于63,则重置动画
    m_object[num].frame+=1;	// 加当前的爆炸动画帧
    if (m_object[num].frame>63)	// 是否已完成所有的16帧?
    {
        initObject(num);// 定义物体 (给新的变量)
    }
}

//画靶子
void MyGLWidget::drawTargets()
{
    glLoadIdentity();// Reset The Modelview Matrix
    glTranslatef(0.0f,0.0f,-10.0f);// 移入屏幕 20 个单位
    for (int loop=0; loop<m_level; loop++)// 循环在 9 个物体
    {
        glLoadName(loop);// 给物体新名字
        glPushMatrix();// 存矩阵
            glTranslatef(m_object[loop].x,
                         m_object[loop].y,
                         m_object[loop].distance);// 物体的位置 (x,y)
            if (m_object[loop].hit)// 若物体已被点击
            {
                explosion(loop);// 画爆炸动画
            }
            else// Otherwise
            {
                glRotatef(m_object[loop].spin,0.0f,0.0f,1.0f);// 旋转物体
                object(dimensionSize[m_object[loop].texid].w,
                       dimensionSize[m_object[loop].texid].h,
                       m_object[loop].texid);// 画物体
            }
        glPopMatrix();// 弹出矩阵
    }
}

//这课程是多次熬夜的成果, 许多的时间用来编码和写 HTML. 在这一课结束的时候你应你会学会怎样picking, sorting,
//alpha blending and alpha testing 工作. 制做点和软件. 每一个游戏, 到精选的GUI’们.最好的未来是制做时你不用记录物体.
//你给一个名字和碰撞 . 这很简单! 用alpha 通道和alpha 测试你能使物体完全显示, 或漏出一些. 结果是很好,
//你不用担心关于显示物体的材质, 除非你不显示他们! 同以往一样, 我希望你喜欢这个课程,愿看到一些好的游戏或好的项目从这个课程诞生.
//如果你有什么问题或找到错误，让我知道 ... 我仅是一个普通人 :)
//我将花大量的时间加入东西像物理系统, 更多图, 更多声音, 等. 虽然只是一个课程! 我不写不按车灯和车轮.
//我写这个用尽量不混乱的方法教你 OpenGL . 我希望看到一些严谨的修改. 若你找一些cool的课程发给我一份.
//若是好的修改我将放到下载页. 若有足够充分的修改我会专注修改这个课程的版本! 我在这里给你一个起点. 剩下的靠你了 :)
//要点: 这是很重要的，称为glTexImage2D 你设为两种格式国际 GL_RGBA. 否则 alpha blending 将不工作!
