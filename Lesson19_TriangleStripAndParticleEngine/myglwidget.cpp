#include "myglwidget.h"

//粒子系统:
//你是否希望创建爆炸，喷泉，流星之类的效果。这一课将告诉你如何创建一个简单的粒子系统，并用它来创建一种喷射的效果。
//欢迎来到第十九课.你已经学习了很多知识,并且现在想自己来实践.我将在这讲解一个新命令... 三角形带(我的理解就是画很多三角形来组合成我们要的形状),
//它非常容易使用,当画很多三角形的时候能加快你程序的运行速度.在本课中,我将会教你该如何做一个半复杂的微粒程序.一旦您了解微粒程序的原理后,
//在创建例如:火,烟,喷泉等效果将是很轻松的事情.我必须警告你!直到今天我从未写一个真正的粒子程序.我想写一个"出名"的复杂的粒子程序.我尝试过,
//但在我了解我不能控制所有点变疯狂之后我放弃了!!!你也许不相信我要告诉你的,但这个课程从头到尾都是我自己的想法.开始我没有一点想法,并且没有
//任何技术数据放在我的面前.我开始考虑粒子,突然我的脑袋装满想法(脑袋开启了??):给予每个粒子生命,任意变化颜色,速度,重力影响等等.来适应环境的
//变化,把每个粒子看成单一的从这个点运动到另一个点的颗粒.很快我完成了这个项目.我看看时钟然后有个想法突然出现.四个小时过去了!我偶尔记得已经
//停止喝咖啡,眨眼,但是4个小时...?尽管这个程序我觉得很棒,并象我想象的那么严密的运行,但它不可能是最好的粒子引擎,这个我不关心,只要他运行好
//就可以.并且我能把它运行在我的项目中.如果你是那种想了解透彻的人,那么你要花费很多时间在网络上查找资料并弄明白它.在程序中有很多小的代码会
//看起来很模糊:)本课教程所用的部分代码来自于Lesson1.但有很多新的代码,因此我将重写一些发生代码变化的部分(使它更容易了解).

//在颜色数组上我们减少一些代码来存储12中不同的颜色.对每一个颜色从0到11我们存储亮红,亮绿,和亮蓝.
//下面的颜色表里包含12个渐变颜色从红色到紫罗兰色
static GLfloat colors[12][3]=				// 彩虹颜色
{
    {1.0f,0.5f,0.5f},{1.0f,0.75f,0.5f},{1.0f,1.0f,0.5f},{0.75f,1.0f,0.5f},
    {0.5f,1.0f,0.5f},{0.5f,1.0f,0.75f},{0.5f,1.0f,1.0f},{0.5f,0.75f,1.0f},
    {0.5f,0.5f,1.0f},{0.75f,0.5f,1.0f},{1.0f,0.5f,1.0f},{1.0f,0.5f,0.75f}
};

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_rainbow(true), m_slowdown(2.0f), m_zoom(-40.0f),
    m_xspeed(0.0f), m_yspeed(0.0f), m_delay(0), m_col(0)
{
    showNormal();
    startTimer(10);
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(1, &m_texture[0]);
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

//我们使用光滑的阴影,清除背景为黑色,关闭深度测试,绑定并映射纹理.启用映射位图后我们选择粒子纹理。唯一的改变就是禁用深度测试和初始化粒子
void MyGLWidget::initializeGL()
{
    loadGLTexture();

    glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
    glClearColor(0.0f,0.0f,0.0f,0.0f);					// Black Background
    glClearDepth(1.0f);									// Depth Buffer Setup
    glDisable(GL_DEPTH_TEST);							// Disable Depth Testing
    glEnable(GL_BLEND);									// Enable Blending
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);					// Type Of Blending To Perform
    glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);	// Really Nice Perspective Calculations
    glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);				// Really Nice Point Smoothing
    glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
    glBindTexture(GL_TEXTURE_2D,m_texture[0]);

    //下面代码初始化每个粒子.我们从活跃的粒子开始.如果粒子不活跃,它在荧屏上将不出现,无论它有多少life.
    //当我们使粒子活跃之後,我们给它life.我怀疑给粒子生命和颜色渐变的是否是最好的方法,但当它运行一次后,效果很好!
    //life满值是1.0f.这也给粒子完整的光亮.
    for (int i = 0, iend = MAX_PARTICLES; i < iend;i++)				//初始化所有的粒子
    {
        particle[i].active=true;					// 使所有的粒子为激活状态
        particle[i].life=1.0f;					// 所有的粒子生命值为最大
        //我们通过给定的值来设定粒子退色快慢.每次粒子被拉的时候life随着fade而减小.
        //结束的数值将是0~99中的任意一个,然后平分1000份来得到一个很小的浮点数.最后我们把结果加上0.003f来使fade速度值不为0
        particle[i].fade=float(qrand()%100)/1000.0f+0.003f;		// 随机生成衰减速率
        //既然粒子是活跃的,而且我们又给它生命,下面将给它颜色数值.一开始,我们就想每个粒子有不同的颜色.
        //我怎么做才能使每个粒子与前面颜色箱里的颜色一一对应那?数学很简单,我们用i变量乘以箱子中颜色的数目与粒子
        //最大值(MAX_PARTICLES)的余数.这样防止最后的颜色数值大于最大的颜色数值(12).举例:900*(12/900)=12.1000*(12/1000)=12,等等
        particle[i].r=colors[i*(12/MAX_PARTICLES)][0];		// 粒子的红色颜色
        particle[i].g=colors[i*(12/MAX_PARTICLES)][1];		// 粒子的绿色颜色
        particle[i].b=colors[i*(12/MAX_PARTICLES)][2];		// 粒子的蓝色颜色
        //现在设定每个粒子移动的方向和速度.我们通过将结果乘于10.0f来创造开始时的爆炸效果.我们将会以任意一个正或负值结束.
        //这个数值将以任意速度,任意方向移动粒子.
        particle[i].xi=float((qrand()%50)-26.0f)*10.0f;		// 随机生成X轴方向速度
        particle[i].yi=float((qrand()%50)-25.0f)*10.0f;		// 随机生成Y轴方向速度
        particle[i].zi=float((qrand()%50)-25.0f)*10.0f;		// 随机生成Z轴方向速度
        //最后,我们设定加速度的数值.不像一般的加速度仅仅把事物拉下,我们的加速度能拉出,拉下,拉左,拉右,拉前和拉后粒子.
        //开始我们需要强大的向下加速度.为了达到这样的效果我们将xg设为0.0f.在x方向没有拉力.我们设yg为-0.8f来产生一个向下的拉力.
        //如果值为正则拉向上.我们不希望粒子拉近或远离我们,所以将zg设为0.0f
        particle[i].xg=0.0f;						// 设置X轴方向加速度为0
        particle[i].yg=-0.8f;						//  设置Y轴方向加速度为-0.8
        particle[i].zg=0.0f;						//  设置Z轴方向加速度为0
    }

}

//现在为有趣的部分.下面的部分是我们从哪里拉粒子,检查加速度等等.你要明白它是怎么实现的,因此仔细的看:)我们重置Modelview巨阵.
//在画粒子位置的时候用glVertex3f()命令来代替tranlations,这样在我们画粒子的时候不会改变modelview巨阵
void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				// 以黑色背景清楚
    glLoadIdentity();								// 重置模型变换矩阵
    //我们开始创建一个循环.这个环将会更新每一个粒子.
    for (int i = 0, iend = MAX_PARTICLES;i<iend;i++)					// 循环所有的粒子
    {
        //首先我们做的事物是检查粒子是否活跃.如果不活跃,则不被更新.在这个程序中,它们始终活跃.但是在你自己的程序中,你可能想要使某粒子不活跃
        {
            if(particle[i].active)					// 如果粒子为激活的
            {
                //下面三个变量是我们确定x,y和z位置的暂时变量.注意:在z的位置上我们加上zoom以便我们的场景在以前的基础上再移入zoom个位置.
                //particle[i].x告诉我们要画的x的位置.particle[i].y告诉我们要画的y的位置.particle[i].
                //z告诉我们要画的z的位置
                float x = particle[i].x;				// 返回X轴的位置
                float y = particle[i].y;				// 返回Y轴的位置
                float z = particle[i].z+m_zoom;			// 返回Z轴的位置
                //既然知道粒子位置,就能给粒子上色.particle[i].r保存粒子的亮红,particle[i].
                //g保存粒子的亮绿,particle[i].b保存粒子的亮蓝.注意我用alpha为粒子生命.当粒子要燃尽时,
                //它会越来越透明直到它最后消失.这就是为什么粒子的生命不应该超过1.0f.如果你想粒子燃烧时间长,可降低fade减小的速度
                // 设置粒子颜色
                glColor4f(particle[i].r,particle[i].g,particle[i].b,particle[i].life);
                //我们有粒子的位置,并设置颜色了.所以现在我们来画我们的粒子.我们用一个三角形带来代替一个四边形这样使程序运行快一点.
                //很多3D card画三角形带比画四边形要快的多.有些3D card将四边形分成两个三角形,而有些不.
                //所以我们按照我们自己的想法来,所以我们来画一个生动的三角形带
                glBegin(GL_TRIANGLE_STRIP);				// 绘制三角形带
                //从红宝书引述:三角形带就是画一连续的三角形(三个边的多角形)使用vertices V0,V1,V2,然后V2,V1,V3(注意顺序),
                //然后V2,V3,V4等等.画三角形的顺序一样才能保证三角形带为相同的表面.要求方向是很重要的,
                //例如:剔除,最少用三点来画当第一个三角形使用vertices0,1和2被画.
                //如果你看图片你将会理解用顶点0,1和2构造第一个三角形(顶端右边,顶端左边,底部的右边).
                //第二个三角形用点vertices2,1和3构造.再一次,如果你注意图片,点vertices2,
                //1和3构造第二个三角形(底部右边,顶端左边,底部左边).注意:两个三角形画点顺序相同.
                //我看到很多的网站要求第二个三角形反方向画.这是不对的.Opengl从新整理顶点来保证所有的三角形为同一方向!
                //注意:你在屏幕上看见的三角形个数是你叙述的顶点的个数减2.在程序中在我们有4个顶点,所以我们看见二个三角形
                    glTexCoord2d(1,1); glVertex3f(x+0.5f,y+0.5f,z);
                    glTexCoord2d(0,1); glVertex3f(x-0.5f,y+0.5f,z);
                    glTexCoord2d(1,0); glVertex3f(x+0.5f,y-0.5f,z);
                    glTexCoord2d(0,0); glVertex3f(x-0.5f,y-0.5f,z);
                //最后我们告诉Opengl我们画完三角形带
                glEnd();
            }
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
        case Qt::Key_Escape:
        {
            qApp->exit();
            break;
        }
        case Qt::Key_Up:
        {
            //下行描述加速度的数值是多少.通过上箭头键,我们增加yg(y 地心引力)值.这引起向上的力.
            //如果这个程序在循环外面,那么我们必须生成另一个循环做相同的工作,因此我们最好放在这里
            for (int i = 0, iend = MAX_PARTICLES; i < iend;i++)					// 循环所有的粒子
            {
                if(particle[i].active)
                {
                    if(particle[i].yg < 1.5f)
                    {
                        particle[i].yg+=0.01f;
                    }
                }
            }
            break;
        }
        case Qt::Key_Down:
        {
            //这行是产生相反的效果.通过下箭头号键,减小yg值,引起向下的力
            //下行描述加速度的数值是多少.通过上箭头键,我们增加yg(y 地心引力)值.这引起向上的力.
            //如果这个程序在循环外面,那么我们必须生成另一个循环做相同的工作,因此我们最好放在这里
            for (int i = 0, iend = MAX_PARTICLES; i < iend;i++)					// 循环所有的粒子
            {
                if(particle[i].active)
                {
                    if(particle[i].yg > -1.5f)
                    {
                        particle[i].yg-=0.01f;
                    }
                }
            }
            break;
        }
        case Qt::Key_Right:
        {
            //现在更改向右的拉力.如果按下右箭头键时增加向右的拉力.
            for (int i = 0, iend = MAX_PARTICLES; i < iend;i++)					// 循环所有的粒子
            {
                if(particle[i].active)
                {
                    if(particle[i].xg < 1.5f)
                    {
                        particle[i].xg+=0.01f;
                    }
                }
            }
            break;
        }
        case Qt::Key_Left:
        {
            //最后如果左箭头键被按下则增加向左的拉力.这些按键给了我们很酷的结果.
            //举例来说:你可以用粒子造一条向上设的水流.通过增加向下的引力可以形成泉水
            for (int i = 0, iend = MAX_PARTICLES; i < iend;i++)					// 循环所有的粒子
            {
                if(particle[i].active)
                {
                    if(particle[i].xg > -1.5f)
                    {
                        particle[i].xg-=0.01f;
                    }
                }
            }
            break;
        }
        case Qt::Key_Tab:
        {
            //我仅仅为乐趣增加了一些代码.我的兄弟产生很棒的效果:)通过按住tab键所有粒子都回到屏幕中心.
            //所有的粒子在从新开始运动,再产生一个大的爆发.在粒子变弱之后,你最初的效果会再一次出现
            for (int i = 0, iend = MAX_PARTICLES; i < iend;i++)					// 循环所有的粒子
            {
                particle[i].x=0.0f;
                particle[i].y=0.0f;
                particle[i].z=0.0f;
                particle[i].xi=float((qrand()%50)-26.0f)*10.0f;	// 随机生成速度
                particle[i].yi=float((qrand()%50)-25.0f)*10.0f;
                particle[i].zi=float((qrand()%50)-25.0f)*10.0f;
            }
            break;
        }
        case Qt::Key_BracketRight:
        {
            //下面的代码检查"]"是否被按下.如果它和slowdown一起实现则slowdown减少0.01f.粒子就可以较快速地移动.
            if(m_slowdown > 1.0f)
            {
                m_slowdown-=0.1f;
            }
            break;
        }
        case Qt::Key_BracketLeft:
        {
            //下面的代码检查"["是否被按下.如果它和slowdown一起实现则slowdown增加0.01f.
            //粒子就可以较慢速地移动.我实质的极限是4.0f,我不想它太慢的运动,你可以随你的要求改变最大最小值
            if(m_slowdown < 4.0f)
            {
                m_slowdown+=0.1f;
            }
            break;
        }
        case Qt::Key_PageUp:
        {
            //下面的代码检测Page Up是否被按下.如果是,则zoom增加.从而导致粒子靠近我们
            m_zoom+=1.0f;
            break;
        }
        case Qt::Key_PageDown:
        {
            //下行代码检测Page Down是否别按下,如果是,则zoom减小.从而导致粒子离开我们
            m_zoom-=1.0f;
            break;
        }
        case Qt::Key_Return:
        {
            //下面的代码检验enter键是否被按下.如果是,并且没有被一直按着,我们将让计算机把rp变为true,
            //然后我们固定彩虹模式.如果彩虹模式为true,将其变成false.如果为false,将其变成true.
            m_rainbow = !m_rainbow;
            break;
        }
        case Qt::Key_Space:
        {
            //下面行是为了当space按下则彩虹关掉而设置的.如果我们不关掉彩虹模式,颜色会继续变化直到enter再被按下.
            //也就是说人们按下space来代替enter是想叫粒子颜色自己变化
            m_rainbow = false;
            m_delay = 0;
            //如果颜色值大于11,我们把它重新设为零.如果我们不重新设定为零,程序将去找第13颜色.
            //而我们只有12种颜色!寻找不存在的颜色将会导致程序瘫痪
            m_col++;
            if(m_col > 11)
            {
                m_col = 0;
            }
            break;
        }
        case Qt::Key_W:
        {
            //现在对粒子增加一些控制.还记得我们从开始定义的2变量么?一个xspeed,一个yspeed.在粒子燃尽之后,
            //我们给它新的移动速度且把新的速度加入到xspeed和yspeed中.这样当粒子被创建时将影响粒子的速度.
            //举例来说:粒子在x轴上的速度为5在y轴上的速度为0.当我们减少xspeed到-10,我们将以-10(xspeed)+5(最初的移动速度)的速度移动.
            //这样我们将以5的速度向左移动.明白了么??无论如何,下面的代码检测W是否被按下.
            //如果它,yspeed将增加这将引起粒子向上运动.最大速度不超过200.速度在快就不好看了
            if(m_yspeed < 200)
            {
                m_yspeed+=10.0f;
            }
            break;
        }
        case Qt::Key_S:
        {
            //这行检查S键是否被按下,如果它是,yspeed将减少.这将引起粒子向下运动.再一次,最大速度为200
            if(m_yspeed > -200)
            {
                m_yspeed-=10.0f;
            }
            break;
        }
        case Qt::Key_D:
        {
            //现在我们检查A键是否被按下.如果它是..xspeed将被增加.粒子将移到右边.最大速度为200
            if(m_xspeed < 200)
            {
                m_xspeed+=10.0f;
            }
            break;
        }
        case Qt::Key_A:
        {
            if(m_xspeed > -200)
            {
                m_xspeed-=10.0f;
            }
            break;
        }
    }
    QGLWidget::keyPressEvent(event);
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    for (int i = 0, iend = MAX_PARTICLES;i<iend;i++)					// 循环所有的粒子
    {
        //首先我们做的事物是检查粒子是否活跃.如果不活跃,则不被更新.在这个程序中,它们始终活跃.但是在你自己的程序中,你可能想要使某粒子不活跃
        {
            if(particle[i].active)					// 如果粒子为激活的
            {
                //现在我们能移动粒子.下面公式可能看起来很奇怪,其实很简单.首先我们取得当前粒子的x位置.
                //然后把x运动速度加上粒子被减速1000倍后的值.所以如果粒子在x轴(0)上屏幕中心的位置,
                //运动值(xi)为x轴方向+10(移动我们为右),而slowdown等于1,我们移向右边以10/(1*1000)或 0.01f速度.
                //如果增加slowdown值到2我们只移动0.005f.希望能帮助你了解slowdown如何工作.
                //那也是为什么用10.0f乘开始值来叫象素移动快速,创造一个爆发效果.y和z轴用相同的公式来计算附近移动粒子
                particle[i].x+=particle[i].xi/(m_slowdown*1000);	// 更新X坐标的位置
                particle[i].y+=particle[i].yi/(m_slowdown*1000);	// 更新Y坐标的位置
                particle[i].z+=particle[i].zi/(m_slowdown*1000);	// 更新Z坐标的位置
                //在计算出下一步粒子移到那里,开始考虑重力和阻力.在下面的第一行,将阻力(xg)和移动速度(xi)相加.
                //我们的移动速度是10和阻力是1.每时每刻粒子都在抵抗阻力.第二次画粒子时,阻力开始作用,移动速度将会从10掉到9.
                //第三次画粒子时,阻力再一次作用,移动速度降低到8.如果粒子燃烧为超过10次重画,它将会最后结束,并向相反方向移动.
                //因为移动速度会变成负值.阻力同样使用于y和z移动速度
                particle[i].xi+=particle[i].xg;			// 更新X轴方向速度大小
                particle[i].yi+=particle[i].yg;			// 更新Y轴方向速度大小
                particle[i].zi+=particle[i].zg;			// 更新Z轴方向速度大小
                //下行将粒子的生命减少.如果我们不这么做,粒子无法烧尽.我们用粒子当前的life减去当前的fade值.
                //每粒子都有不同的fade值,因此他们全部将会以不同的速度烧尽
                particle[i].life-=particle[i].fade;		// 减少粒子的生命值
                //现在我们检查当生命为零的话粒子是否活着
                if(particle[i].life<0.0f)					// 如果粒子生命值小于0
                {
                    //如果粒子是小时(烧尽),我们将会使它复原.我们给它全值生命和新的衰弱速度.
                    particle[i].life=1.0f;				// 产生一个新的粒子
                    particle[i].fade=float(qrand()%100)/1000.0f+0.003f;	// 随机生成衰减速率
                    //我们也重新设定粒子在屏幕中心放置.我们重新设定粒子的x,y和z位置为零
                    particle[i].x=0.0f;					// 新粒子出现在屏幕的中央
                    particle[i].y=0.0f;
                    particle[i].z=0.0f;
                    //在粒子从新设置之后,将给它新的移动速度/方向.注意:我增加最大和最小值,粒子移动速度为从50到60的任意值,
                    //但是这次我们没将移动速度乘10.我们这次不想要一个爆发的效果,而要比较慢地移动粒子.也注意我把xspeed和x轴移动速度相加,
                    //y轴移动速度和yspeed相加.这个控制粒子的移动方向.
                    particle[i].xi=m_xspeed+float((qrand()%60)-32.0f);	// 随机生成粒子速度
                    particle[i].yi=m_yspeed+float((qrand()%60)-30.0f);
                    particle[i].zi=float((qrand()%60)-30.0f);
                    //最后我们分配粒子一种新的颜色.变量col保存一个数字从1到11(12种颜色),我们用这个变量去找红,绿,蓝亮度在颜色箱里面.
                    //下面第一行表示红色的强度,数值保存在colors[col][0].所以如果col是0,红色的亮度就是1.0f.
                    //绿色的和蓝色的值用相同的方法读取.如果你不了解为什么红色亮度为1.0f那col就为0.我将一点点的解释.
                    //看着程序的最前面.找到那行:static GLfloat colors[12][3].
                    //注意:12行3列.三个数字的第一行是红色强度.第二行是绿色强度而且第三行是蓝色强度.[0],[1]和[2]下面描述的1st,2nd和3rd
                    //就是我刚提及的.如果col等于0,我们要看第一个组.11 是最後一个组(第12种颜色).
                    particle[i].r=colors[m_col][0];			// 设置粒子颜色
                    particle[i].g=colors[m_col][1];
                    particle[i].b=colors[m_col][2];
                }
            }
        }
    }
    if(m_rainbow && m_delay > 25)
    {
        //如果颜色值大于11,我们把它重新设为零.如果我们不重新设定为零,程序将去找第13颜色.
        //而我们只有12种颜色!寻找不存在的颜色将会导致程序瘫痪
        m_col++;
        if(m_col > 11)
        {
            m_col = 0;
        }
    }
    m_delay++;
    updateGL();
    QGLWidget::timerEvent(event);
}

void MyGLWidget::loadGLTexture()
{
    //现在载入图像，并将其转换为纹理。
    QImage image(":/image/Particle.bmp");
    image = image.convertToFormat(QImage::Format_RGB888);
    image = image.mirrored();
    glGenTextures(1, &m_texture[0]);// 创建纹理
    // 使用来自位图数据生成 的典型纹理
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);
    //下来我们创建真正的纹理。下面一行告诉OpenGL此纹理是一个2D纹理 ( GL_TEXTURE_2D )。
    //参数“0”代表图像的详细程度，通常就由它为零去了。参数三是数据的成分数。
    //因为图像是由红色数据，绿色数据，蓝色数据三种组分组成。 TextureImage[0]->sizeX 是纹理的宽度。
    //如果您知道宽度，您可以在这里填入，但计算机可以很容易的为您指出此值。
    //TextureImage[0]->sizey 是纹理的高度。参数零是边框的值，一般就是“0”。
    //GL_RGB 告诉OpenGL图像数据由红、绿、蓝三色数据组成。
    //GL_UNSIGNED_BYTE 意味着组成图像的数据是无符号字节类型的。
    //最后... TextureImage[0]->data 告诉OpenGL纹理数据的来源。此例中指向存放在 TextureImage[0] 记录中的数据。
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 image.width(), image.height(), 0, GL_RGB, GL_UNSIGNED_BYTE,
                 image.bits());
    //下面的两行告诉OpenGL在显示图像时，
    //当它比放大得原始的纹理大 ( GL_TEXTURE_MAG_FILTER )或缩小得比原始得纹理小( GL_TEXTURE_MIN_FILTER )时
    //OpenGL采用的滤波方式。通常这两种情况下我都采用 GL_LINEAR 。
    //这使得纹理从很远处到离屏幕很近时都平滑显示。使用 GL_LINEAR 需要CPU和显卡做更多的运算。
    //如果您的机器很慢，您也许应该采用 GL_NEAREST 。过滤的纹理在放大的时候，看起来斑驳的很『译者注：马赛克啦』。
    //您也可以结合这两种滤波方式。在近处时使用 GL_LINEAR ，远处时 GL_NEAREST 。
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	// 线形滤波
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	// 线形滤波
}

//在课程中,我试着把所有细节都讲清楚,并且简单的了解粒子系统.这个粒子系统能在游戏产生例如火,水,雪,爆炸,流行等效果.
//程序能简单的修改参数来实现新的效果(例:烟花效果)
