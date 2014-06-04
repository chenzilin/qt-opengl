#include "myglwidget.h"

//线，反走样，计时，正投影和简单的声音:
//这是我第一个大的教程，它将包括线，反走样，计时，正投影和简单的声音。希望这一课中的东西能让每个人感到高兴。

//欢迎来到第21课，在这一课里，你将学会直线，反走样，正投影，计时，基本的音效和一个简单的游戏逻辑。希望这里的东西可以让你高兴。
//我花了两天的时间写代码，并用了两周的时间写这份HTML文件，希望你能享受我的劳动。
//在这课的结尾你将获得一个叫"amidar"的游戏，你的任务是走完所有的直线。这个程序有了一个基本游戏的一切要素，关卡，生命值，声音和一个游戏道具。
//我们从第一课的程序来逐步完整这个程序，按照惯例，我们只介绍改动的部分。

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_anti(true),
    m_lives(5), m_level(1), m_stage(1), m_freezeSound(":/voice/Freeze.wav"),
    m_filled(false), m_gameover(false)
{
    showNormal();
    resetObjects();
    resetLines();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(2, &m_texture[0]);
    glDeleteLists(m_base, 256);
}

//下面的代码基本没有变化，只是把透视投影变为了正投影
void MyGLWidget::resizeGL(int w, int h)
{
    if (h==0)
    {
        h=1;
    }
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f,w,h,0.0f,-1.0f,1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

//初始化的代码和前面的代码相比没有什么改变
void MyGLWidget::initializeGL()
{
    loadGLTexture();
    buildFont();
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepth(1.0f);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

//下面是我们的绘制代码。
//首先我们清空缓存，接着绑定字体的纹理，绘制游戏的提示字符串
void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);				// 选择字符纹理
    glColor3f(1.0f,0.5f,1.0f);
    glPrint(207,24,0,"GRID CRAZY");						// 绘制游戏名称"GRID CRAZY"
    glColor3f(1.0f,1.0f,0.0f);
    QString levelStr = QString("Level:%1").arg(m_level);
    QString stagesTR = QString("Stage:%1").arg(m_stage);
    glPrint(20,20,1,levelStr.toLatin1().data());					// 绘制当前的级别
    glPrint(20,40,1,stagesTR.toLatin1().data());					// 绘制当前级别的关卡
    //现在我们检测游戏是否结束，如果游戏结束绘制"Gmae over"并提示玩家按空格键重新开始
    if (m_gameover)								// 游戏是否结束?
    {
        glColor3ub(qrand()%255, qrand()%255, qrand()%255);			// 随机选择一种颜色
        glPrint(472,20,1,"GAME OVER");					// 绘制 GAME OVER 字符串到屏幕
        glPrint(456,40,1,"PRESS SPACE");				// 提示玩家按空格键重新开始
    }
    //在屏幕的右上角绘制玩家的剩余生命
    for (int i = 0, iend = m_lives - 1; i < iend; i++)					//循环绘制玩家的剩余生命
    {
        glLoadIdentity();
        glTranslatef(490+(i*40.0f),40.0f,0.0f);			// 移动到屏幕右上角
        glRotatef(-m_player.spin,0.0f,0.0f,1.0f);				// 旋转绘制的生命图标
        glColor3f(0.0f,1.0f,0.0f);					// 绘制玩家生命
        glBegin(GL_LINES);						// 绘制玩家图标
            glVertex2d(-5,-5);
            glVertex2d( 5, 5);
            glVertex2d( 5,-5);
            glVertex2d(-5, 5);
        glEnd();
        glRotatef(-m_player.spin*0.5f,0.0f,0.0f,1.0f);
        glColor3f(0.0f,0.75f,0.0f);
        glBegin(GL_LINES);
            glVertex2d(-7, 0);
            glVertex2d( 7, 0);
            glVertex2d( 0,-7);
            glVertex2d( 0, 7);
        glEnd();
    }
    //下面我们来绘制网格，我们设置变量filled为TRUE，这告诉程序填充网格。
    //接着我们把线的宽度设置为2，并把线的颜色设置为蓝色，接着我们检测线断是否被走过，如果走过我们设置颜色为白色。
    m_filled = true;								// 在测试前，把填充变量设置为TRUE
    glLineWidth(2.0f);							// 设置线宽为2.0f
    glDisable(GL_LINE_SMOOTH);						// 禁用反走样
    glLoadIdentity();
    for (int i = 0, iend = 11; i < iend; i++)					// 循环11根线
    {
        for (int j = 0, jend = 11; j < jend; j++)				// 循环每根线的线段
        {
            glColor3f(0.0f,0.5f,1.0f);				// 设置线为蓝色
            if (m_hline[i][j])				// 是否走过？
            {
                glColor3f(1.0f,1.0f,1.0f);			// 是，设线为白色
            }
            if (i<10)						// 绘制水平线
            {
                if (!m_hline[i][j])			// 如果当前线段没有走过，则不填充
                {
                    m_filled = false;
                }
                glBegin(GL_LINES);				// 绘制当前的线段
                    glVertex2d(20+(i*60),70+(j*40));
                    glVertex2d(80+(i*60),70+(j*40));
                glEnd();
            }
            //下面的代码绘制垂直的线段
            glColor3f(0.0f,0.5f,1.0f);				// 设置线为蓝色
            if (m_vline[i][j])				// 是否走过
            {
                glColor3f(1.0f,1.0f,1.0f);			// 是，设线为白色
            }
            if (j<10)						// 绘制垂直线
            {
                if (!m_vline[i][j])			// 如果当前线段没有走过，则不填充
                {
                    m_filled = false;
                }
                glBegin(GL_LINES);				// 绘制当前的线段
                    glVertex2d(20+(i*60),70+(j*40));
                    glVertex2d(20+(i*60),110+(j*40));
                glEnd();
            }
            //接下来我们检测长方形的四个边是否都被走过，如果被走过我们就绘制一个带纹理的四边形。
            //我们用下图来解释这个检测过程
            //如果对于垂直线vline的相邻两个边都被走过，并且水平线hline的相邻两个边也被走过，那么我们就可以绘制这个四边形了。
            //我们可以使用循环检测每一个四边形，代码如下：
            glEnable(GL_TEXTURE_2D);				// 使用纹理映射
            glColor3f(1.0f,1.0f,1.0f);				// 设置为白色
            glBindTexture(GL_TEXTURE_2D, m_texture[1]);		// 绑定纹理
            if ((i<10) && (j<10))				// 绘制走过的四边形
            {
                // 这个四边形是否被走过?
                if (m_hline[i][j] && m_hline[i][j+1] && m_vline[i][j] && m_vline[i+1][j])
                {
                    glBegin(GL_QUADS);			// 是，则绘制它
                        glTexCoord2f(float(i/10.0f)+0.1f,1.0f-(float(j/10.0f)));
                        glVertex2d(20+(i*60)+59,(70+j*40+1));
                        glTexCoord2f(float(i/10.0f),1.0f-(float(j/10.0f)));
                        glVertex2d(20+(i*60)+1,(70+j*40+1));
                        glTexCoord2f(float(i/10.0f),1.0f-(float(j/10.0f)+0.1f));
                        glVertex2d(20+(i*60)+1,(70+j*40)+39);
                        glTexCoord2f(float(i/10.0f)+0.1f,1.0f-(float(j/10.0f)+0.1f));
                        glVertex2d(20+(i*60)+59,(70+j*40)+39);
                    glEnd();
                }
            }
            glDisable(GL_TEXTURE_2D);
        }
    }
    glLineWidth(1.0f);
    //下面的代码用来设置是否启用直线反走样
    if (m_anti)								// 是否启用反走样?
    {
        glEnable(GL_LINE_SMOOTH);
    }
    //为了使游戏变得简单些，我添加了一个时间停止器，当你吃掉它时，可以让追击的你的敌人停下来。
    //下面的代码用来绘制一个时间停止器。
    if (m_hourglass.fx==1)
    {
        glLoadIdentity();
        glTranslatef(20.0f+(m_hourglass.x*60),70.0f+(m_hourglass.y*40),0.0f);
        glRotatef(m_hourglass.spin,0.0f,0.0f,1.0f);
        glColor3ub(qrand()%255,qrand()%255,qrand()%255);
        glBegin(GL_LINES);
            glVertex2d(-5,-5);
            glVertex2d( 5, 5);
            glVertex2d( 5,-5);
            glVertex2d(-5, 5);
            glVertex2d(-5, 5);
            glVertex2d( 5, 5);
            glVertex2d(-5,-5);
            glVertex2d( 5,-5);
        glEnd();
    }
    //接下来绘制我们玩家
    glLoadIdentity();
    glTranslatef(m_player.fx+20.0f, m_player.fy+70.0f, 0.0f);			// 设置玩家的位置
    glRotatef(m_player.spin, 0.0f, 0.0f, 1.0f);					// 旋转动画
    glColor3f(0.0f,1.0f,0.0f);
    glBegin(GL_LINES);
        glVertex2d(-5,-5);
        glVertex2d( 5, 5);
        glVertex2d( 5,-5);
        glVertex2d(-5, 5);
    glEnd();
    //绘制玩家的显示效果，让它看起来更好看些（其实没用）
    glRotatef(m_player.spin*0.5f, 0.0f, 0.0f, 1.0f);
    glColor3f(0.0f,0.75f,0.0f);
    glBegin(GL_LINES);
        glVertex2d(-7, 0);
        glVertex2d( 7, 0);
        glVertex2d( 0,-7);
        glVertex2d( 0, 7);
    glEnd();
    //接下来绘制追击玩家的敌人
    for (int i = 0, iend = m_stage*m_level; i < iend; i++)
    {
        glLoadIdentity();
        glTranslatef(m_enemy[i].fx+20.0f, m_enemy[i].fy+70.0f, 0.0f);
        glColor3f(1.0f,0.5f,0.5f);
        glBegin(GL_LINES);
            glVertex2d( 0,-7);
            glVertex2d(-7, 0);
            glVertex2d(-7, 0);
            glVertex2d( 0, 7);
            glVertex2d( 0, 7);
            glVertex2d( 7, 0);
            glVertex2d( 7, 0);
            glVertex2d( 0,-7);
        glEnd();
        //下面的代码绘制敌人的显示效果，让其更好看。
        glRotatef(m_enemy[i].spin,0.0f,0.0f,1.0f);
        glColor3f(1.0f,0.0f,0.0f);
        glBegin(GL_LINES);
            glVertex2d(-7,-7);
            glVertex2d( 7, 7);
            glVertex2d(-7, 7);
            glVertex2d( 7,-7);
        glEnd();
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
        case Qt::Key_A:
        {
            //按A键切换是否启用反走样
            m_anti = !m_anti;
            break;
        }
        //使用上，下，左，右控制玩家的位置
        case Qt::Key_Up:
        {
            if (m_player.y > 0 && m_player.fx==m_player.x*60 && m_player.fy==m_player.y*40)
            {
                m_player.y--;
                m_vline[m_player.x][m_player.y] = true;
            }
            break;
        }
        case Qt::Key_Down:
        {
            if (m_player.y<10 && m_player.fx==m_player.x*60 && m_player.fy==m_player.y*40)
            {
                m_vline[m_player.x][m_player.y] = true;
                m_player.y++;
            }
            break;
        }
        case Qt::Key_Left:
        {
            if (m_player.x>0 && m_player.fx==m_player.x*60 && m_player.fy==m_player.y*40)
            {
                m_player.x--;
                m_hline[m_player.x][m_player.y] = true;
            }
            break;
        }
        case Qt::Key_Right:
        {
            if (m_player.x<10 && m_player.fx==m_player.x*60 && m_player.fy==m_player.y*40)
            {
                m_hline[m_player.x][m_player.y] = true;
                m_player.x++;
            }
            break;
        }
        case Qt::Key_Space:
        {
            if(m_gameover)
            {
                //如果游戏结束，按空格开始新的一局游戏
                m_gameover = false;				// 开始新的一局
                m_filled = true;				// 重置所有的变量
                m_level=1;
                m_stage=0;
                m_lives=5;
            }
            break;
        }
    }
    QGLWidget::keyPressEvent(event);
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    if (!m_gameover)				// 如果游戏没有结束，则进行游戏循环
    {
        for(int i = 0, iend = m_stage*m_level; i < iend; i++)
        {
            //根据玩家的位置，让敌人追击玩家
            if ((m_enemy[i].x<m_player.x) && (m_enemy[i].fy==m_enemy[i].y*40))
            {
                m_enemy[i].x++;
            }
            if ((m_enemy[i].x>m_player.x) && (m_enemy[i].fy==m_enemy[i].y*40))
            {
                m_enemy[i].x--;
            }
            if ((m_enemy[i].y<m_player.y) && (m_enemy[i].fx==m_enemy[i].x*60))
            {
                m_enemy[i].y++;
            }
            if ((m_enemy[i].y>m_player.y) && (m_enemy[i].fx==m_enemy[i].x*60))
            {
                m_enemy[i].y--;
            }
            //如果时间停止器的显示时间结束，而玩家又没有吃到，那么重置计时计算器。
            if (m_delay > (3 - m_level) && (m_hourglass.fx!=2))		// 如果没有吃到时间停止器
            {
                m_delay=0;					// 重置时间停止器
                for (int j = 0, jend = m_stage*m_level; j < jend; j++)	// 循环设置每个敌人的位置
                {
                    //下面的代码调整每个敌人的位置，并绘制它们的显示效果
                    if (m_enemy[j].fx<m_enemy[j].x*60)
                    {
                        m_enemy[j].fx+=5;
                        m_enemy[j].spin+=5;
                    }
                    if (m_enemy[j].fx>m_enemy[j].x*60)
                    {
                        m_enemy[j].fx-=5;
                        m_enemy[j].spin-=5;
                    }
                    if (m_enemy[j].fy<m_enemy[j].y*40)
                    {
                        m_enemy[j].fy+=5;
                        m_enemy[j].spin+=5;
                    }
                    if (m_enemy[j].fy>m_enemy[j].y*40)
                    {
                        m_enemy[j].fy-=5;
                        m_enemy[j].spin-=5;
                    }
                }
            }
            //如果敌人的位置和玩家的位置相遇，这玩家死亡，开始新的一局
            if ((m_enemy[i].fx==m_player.fx) && (m_enemy[i].fy==m_player.fy))
            {
                m_lives--;			// 如果是，生命值减1
                if (m_lives==0)			// 如果生命值为0，则游戏结束
                {
                    m_gameover=TRUE;
                }
                resetObjects();			// 重置所有的游戏变量
                QSound::play(":/voice/Die.wav"); // 播放死亡的音乐
            }
        }
        //调整玩家的位置，让动画看起来跟自然
        if (m_player.fx<m_player.x*60)
        {
            m_player.fx+=5;
        }
        if (m_player.fx>m_player.x*60)
        {
            m_player.fx-=5;
        }
        if (m_player.fy<m_player.y*40)
        {
            m_player.fy+=5;
        }
        if (m_player.fy>m_player.y*40)
        {
            m_player.fy-=5;
        }
    }
    //如果顺利通过本关，播放通关音乐，并提高游戏难度，开始新的一局
    if (m_filled)						// 所有网格是否填满
    {
        QSound::play(":/voice/Complete.wav");
        m_stage++;					// 增加游戏难度
        if(m_stage>3)					// 如果当前的关卡大于3，则进入到下一个大的关卡?
        {
            m_stage=1;				// 重置当前的关卡
            m_level++;				// 增加大关卡的值
            if (m_level>3)
            {
                m_level=3;			// 如果大关卡大于3，则不再增加
                m_lives++;			// 完成一局给玩家奖励一条生命
                if (m_lives>5)			// 如果玩家有5条生命，则不再增加
                {
                    m_lives=5;
                }
            }
        }
        //进入到下一关卡，重置所有的游戏变量
        resetObjects();
        resetLines();
    }
    //如果玩家吃到时间停止器，记录这一信息
    if ((m_player.fx==m_hourglass.x*60) && (m_player.fy==m_hourglass.y*40) && (m_hourglass.fx==1))
    {
        // 播放一段声音
        m_freezeSound.setLoops(999);
        m_freezeSound.play();
        m_hourglass.fx=2;					// 设置fx为2，表示吃到时间停止器
        m_hourglass.fy=0;					// 设置fy为0
    }
    //显示玩家的动画效果
    m_player.spin+=0.5f*5;			// 旋转动画
    if(m_player.spin > 360.0f)
    {
        m_player.spin-=360;
    }
    //显示时间停止器的动画
    m_hourglass.spin-=0.25f*5;			// 旋转动画
    if (m_hourglass.spin<0.0f)
    {
        m_hourglass.spin+=360.0f;
    }
    //下面的代码计算何时出现一个时间停止计数器
    m_hourglass.fy+=5;				// 增加fy的值，当他大于一定的时候，产生时间停止计数器
    if ((m_hourglass.fx==0) && (m_hourglass.fy>6000/m_level))
    {
        QSound::play(":/voice/Hourglass.wav");
        m_hourglass.x=qrand()%10+1;
        m_hourglass.y=qrand()%11;
        m_hourglass.fx=1;					//fx=1表示时间停止器出现
        m_hourglass.fy=0;
    }
    //如果玩家没有拾取时间停止器，则过一段时间后，它自动消失
    if ((m_hourglass.fx==1) && (m_hourglass.fy>6000/m_level))
    {
        m_hourglass.fx=0;					// 消失后重置时间停止器
        m_hourglass.fy=0;
    }
    //如果玩家吃到时间停止器，在时间停止停止阶段播放一段音乐，过一段时间停止播放音乐
    if ((m_hourglass.fx==2) && (m_hourglass.fy>500+(500*m_level)))
    {
        m_freezeSound.stop();
        m_hourglass.fx=0;					// 重置变量
        m_hourglass.fy=0;
    }
    //增加敌人的延迟计数器的值，这个值用来更新敌人的运动
    m_delay++;						// 增加敌人的延迟计数器的值
    updateGL();
    QGLWidget::timerEvent(event);
}

//在下面的代码里，我们把玩家重置在屏幕的左上角，而给敌人设置一个随机的位置。
void MyGLWidget::resetObjects()
{
    m_player.x=0;								// 把玩家置于左上角
    m_player.y=0;
    m_player.fx=0;
    m_player.fy=0;
    //接着我们给敌人一个随机的开始位置，敌人的数量等于难度乘上当前关卡号。记着，难度最大是3，而最多有3关。因此敌人最多有9个。
    for (int i = 0, iend = m_stage*m_level; i < iend; i++)				// 循环随即放置所有的敌人
    {
        m_enemy[i].x = 5 + qrand()%6;
        m_enemy[i].y = qrand()%11;
        m_enemy[i].fx = m_enemy[i].x*60;
        m_enemy[i].fy = m_enemy[i].y*40;
    }
}

void MyGLWidget::loadGLTexture()
{
    QVector<QImage> images;
    images.push_back(QImage(":/image/Font.bmp"));
    images.push_back(QImage(":/image/Image.bmp"));
    for(int i = 0, iend = images.count(); i < iend; i++)
    {
        images[i] = images[i].convertToFormat(QImage::Format_RGB888);
        images[i] = images[i].mirrored();
    }

    glGenTextures(2, &m_texture[0]);
    for(int i = 0, iend = images.count(); i < iend ;i++)
    {
        glBindTexture(GL_TEXTURE_2D, m_texture[i]);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, images[i].width(), images[i].height(),
                     0, GL_RGB, GL_UNSIGNED_BYTE, images[i].bits());
    }
}

//下面的代码建立了显示列表。对于字体的显示，我已经写过教程。在这里我把字体图象分成16×16个单元共256个字符。
//如果你有什么不明白，请参阅前面的教程
void MyGLWidget::buildFont()
{
    m_base = glGenLists(256);
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);
    for (int i = 0, iend = 256; i < iend; i++)
    {
        float cx=float(i%16)/16.0f;
        float cy=float(i/16)/16.0f;
        glNewList(m_base+i,GL_COMPILE);
            glBegin(GL_QUADS);
                glTexCoord2f(cx,1.0f-cy-0.0625f);
                glVertex2d(0,16);
                glTexCoord2f(cx+0.0625f,1.0f-cy-0.0625f);
                glVertex2i(16,16);
                glTexCoord2f(cx+0.0625f,1.0f-cy);
                glVertex2i(16,0);
                glTexCoord2f(cx,1.0f-cy);
                glVertex2i(0,0);
            glEnd();
            glTranslated(15,0,0);
        glEndList();
    }
}

//函数没有做太多改变。唯一的改动是它可以打印变量了。我把代码列出这样你可以容易看到改动的地方。
//请注意，在这里我激活了纹理并且重置了视图矩阵。如果set被置1的话，字体将被放大。我这样做是希望可以在屏幕上显示大一点的字符。
//在一切结束后，我会禁用纹理。
void MyGLWidget::glPrint(GLint x, GLint y, int set, char *fmt)
{
    if (set>1)
    {
        set = 1;
    }
    glEnable(GL_TEXTURE_2D);
    glLoadIdentity();
    glTranslated(x,y,0);
    glListBase(m_base-32+(128*set));
    if(set==0)
    {
        glScalef(1.5f,2.0f,1.0f);
    }
    glCallLists(strlen(fmt),GL_UNSIGNED_BYTE, fmt);
    glDisable(GL_TEXTURE_2D);
}

void MyGLWidget::resetLines()
{
    for (int i = 0, iend = 11; i<iend; i++)
    {
        for (int j = 0, jend = 11; j<jend; j++)
        {
            if (i<10)
            {
                m_hline[i][j] = false;
            }
            if (j<10)
            {
                m_vline[i][j] = false;
            }
        }
    }
}

//我花了很长时间写这份教程，它开始于一个简单的直线教程，结束与一个小型的游戏。希望它能给你一些有用的信息，
//我知道你们中大部分喜欢那些基于“贴图”的游戏，但我觉得这些将教会你关于游戏更多的东西。如果你不同意我的看法，请让我知道，
//因为我想写最好的OpenGL教程。
//请注意，这是一个很大的程序了。我尽量去注释每一行代码，我知道程序运行的一切细节，但把它表达出来又是另一回事。如果你有更好的表达能力，
//请告诉我如何更好的表达。我希望通过我们的努力，这份教程越来越好。谢谢
