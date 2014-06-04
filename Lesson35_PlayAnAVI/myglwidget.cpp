#include "myglwidget.h"

//在OpenGL中播放AVI:
//在OpenGL中如何播放AVI呢？利用Windows的API把每一帧作为纹理绑定到OpenGL中，虽然很慢，但它的效果不错。你可以试试。

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_angle(0),
    m_effect(0), m_env(true), m_bg(true), m_quadratic(NULL),
    m_player(NULL), m_surface(NULL)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
}

void MyGLWidget::resizeGL(int w, int h)
{
    glViewport (0, 0, (GLsizei)(w), (GLsizei)(h));				// Reset The Current Viewport
    glMatrixMode (GL_PROJECTION);								// Select The Projection Matrix
    glLoadIdentity ();											// Reset The Projection Matrix
    gluPerspective (45.0f, (GLfloat)(w)/(GLfloat)(h),1.0f, 100.0f);// Calculate The Aspect Ratio Of The Window
    glMatrixMode (GL_MODELVIEW);// Select The Modelview Matrix
    glLoadIdentity ();// Reset The Modelview Matrix
}

void MyGLWidget::initializeGL()
{
    // 开始用户的初始
    m_angle = 0.0f;// angle为0先
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);// 黑色背景
    glClearDepth(1.0f);// 深度缓冲初始
    glDepthFunc(GL_LEQUAL);// 深度测试的类型(小于或等于)
    glEnable(GL_DEPTH_TEST);// 开启深度测试
    glShadeModel (GL_SMOOTH);// 平滑效果
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);// 透视图计算设为最高精度
    m_quadratic=gluNewQuadric();// 建二次曲面的指针
    gluQuadricNormals(m_quadratic, GLU_SMOOTH);// 设置光滑的法线
    gluQuadricTexture(m_quadratic, GL_TRUE);// 创建纹理坐标
    //下面的代码中,我们开启2D纹理映射,纹理滤镜设为GLNEAREST(最快,但看起来很糙),建立球面映射(为了实现环境映射效果).
    //试试其它滤镜,如果你有条件,可以试试GLLINEAR得到一个平滑的动画效果.
    glEnable(GL_TEXTURE_2D);// 开启2D纹理映射
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);// 设置纹理滤镜
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);		// 设纹理坐标生成模式为s
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);		// 设纹理坐标生成模式为t

    m_player = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
    QString filePath = QApplication::applicationDirPath() + "/Face3.avi";
    m_surface = new VideoWidgetSurface(this);
    connect(m_surface, SIGNAL(aviImage(QImage)), this, SLOT(newAVIImage(QImage)));
    m_player->setVideoOutput(m_surface);
    m_player->setMedia(QUrl::fromLocalFile(filePath));
    m_player->play();
}

void MyGLWidget::paintGL()
{
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// 清屏和深度缓冲
    //下面检查我们是否想画一个背景图.若bg是TRUE,重设模型视角矩阵,
    //画一个单纹理映射的能盖住整个屏幕的矩形(纹理是从AVI从得到的一帧).矩形距离屏面向里20个单位,
    //这样它看起来在对象之后(距离更远).
    if (m_bg)							// 背景可见?
    {
        glLoadIdentity();					// 重设模型视角矩阵
        glBegin(GL_QUADS);					// 开始画背景(一个矩形)
            // 正面
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 11.0f,  8.3f, -20.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-11.0f,  8.3f, -20.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-11.0f, -8.3f, -20.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 11.0f, -8.3f, -20.0f);
        glEnd();
    }
    //画完背景(或没有),重设模型视角矩阵(使视角中心回到屏幕中央).视角中心再向屏内移进10个单位.然后检查env是否为TRUE.
    //若是,开启球面映射来实现环境映射效果.
    glLoadIdentity();// 重设模型视角矩阵
    glTranslatef(0.0f, 0.0f, -10.0f);// 视角中心再向屏内移进10个单位
    if (m_env)							// 环境映射开启?
    {
        glEnable(GL_TEXTURE_GEN_S);				// 开启纹理坐标生成S坐标
        glEnable(GL_TEXTURE_GEN_T);				// 开启纹理坐标生成T坐标
    }
    //在最后关头我加了以下代码.他绕X轴和Y轴旋转(根据angle的值)然后在Z轴方向移动2单位.这使我们离开了屏幕中心.
    //如果删掉下面三行,对象会在屏幕中心打转.有了下面三行,对象旋转时看起来离我们远一些:)
    //如果你不懂旋转和平移...你就不该读这一章:)
    glRotatef(m_angle*2.3f,1.0f,0.0f,0.0f);// 加旋转让东西动起来
    glRotatef(m_angle*1.8f,0.0f,1.0f,0.0f);// 加旋转让东西动起来
    glTranslatef(0.0f,0.0f,2.0f);// 旋转后平移到新位置
    //下面的代码检查我们要画哪一个对象.若effect为0,我们做一些旋转在画一个立方体.
    //这个旋转使立方体绕X,Y,Z轴旋转.现在你脑中该烙下建一个立方体的方法了吧:)
    switch (m_effect)							// 哪个效果?
    {
    case 0:								// 效果 0 - 立方体
        glRotatef(m_angle*1.3f, 1.0f, 0.0f, 0.0f);
        glRotatef(m_angle*1.1f, 0.0f, 1.0f, 0.0f);
        glRotatef(m_angle*1.2f, 0.0f, 0.0f, 1.0f);
        glBegin(GL_QUADS);
            glNormal3f( 0.0f, 0.0f, 0.5f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
            glNormal3f( 0.0f, 0.0f,-0.5f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
            glNormal3f( 0.0f, 0.5f, 0.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
            glNormal3f( 0.0f,-0.5f, 0.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
            glNormal3f( 0.5f, 0.0f, 0.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
            glNormal3f(-0.5f, 0.0f, 0.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
        glEnd();
        break;
    case 1:
        //下面是画球体的地方.开始先绕X,Y,Z轴旋转,再画球体.球体半径为1.3f,20经线,20纬线.
        //我用20是因为我没打算让球体非常光滑.少用些经纬数,使球看起来不那么光滑,
        //这样球转起来时就能看到球面映射的效果(当然球面映射必须开启).试着尝试其它值!
        //要知道,使用更多的经纬数需要更强的计算能力!
        glRotatef(m_angle*1.3f, 1.0f, 0.0f, 0.0f);
        glRotatef(m_angle*1.1f, 0.0f, 1.0f, 0.0f);
        glRotatef(m_angle*1.2f, 0.0f, 0.0f, 1.0f);
        gluSphere(m_quadratic,1.3f,20,20);
        break;
    case 2:
        // 效果2，圆柱
        glRotatef(m_angle*1.3f, 1.0f, 0.0f, 0.0f);
        glRotatef(m_angle*1.1f, 0.0f, 1.0f, 0.0f);
        glRotatef(m_angle*1.2f, 0.0f, 0.0f, 1.0f);
        glTranslatef(0.0f,0.0f,-1.5f);
        gluCylinder(m_quadratic,1.0f,1.0f,3.0f,32,32);
        break;
    }
    //下面检查env是否为TRUE,若是,关闭球面映射.调用glFlush()清空渲染流水线(使在下一帧开始前一切都渲染了).
    if(m_env)                                   // 是否开启了环境渲染
    {
        glDisable(GL_TEXTURE_GEN_S);				// 关闭纹理坐标S
        glDisable(GL_TEXTURE_GEN_T);				// 关闭纹理坐标T
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
        case Qt::Key_Space:
        {
            m_effect++;						// 增加effect
            if (m_effect>3)					// 超出界限
            {
                m_effect=0;					// 重设为0
            }
            break;
        }
        case Qt::Key_B:
        {
            m_bg=!m_bg;						// 改变背景 Off/On
            break;
        }
        case Qt::Key_E:
        {
            m_env=!m_env;// 改变环境映射 Off/On
            break;
        }
    }
    QGLWidget::keyPressEvent(event);
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    m_angle += (float)(15.0)/60.0f;// 根据时间更新angle
    updateGL();
    QWidget::timerEvent(event);
}

void MyGLWidget::newAVIImage(QImage image)
{
    image = image.mirrored();
    bindTexture(image);
    updateGL();
}

//希望你们喜欢这一章.现在已经凌晨两点了(译者oak:译到这时刚好也是2:00am!)...写这章花了我6小时了.听起来不可思议,
//可要把东西写通不是件容易的事.本文我读了三边,我力图使文章好懂.不管你信还是不信,对我最重要的是你们能明白代码是
//怎样运作的,它为什么能行.那就是我喋喋不休并且加了过量注解的原因.
//无论如何,我都想听到本文的反馈.如果你找到文章的错误,并想帮我做一些改进,请联系我.就像我说的那样,
//这是我第一次写和AVI有关的代码.通常我不会写一个我才接触到的主题,但我太兴奋了,并且考虑到关于这方面的文章太少了.
//我所希望的是,我打开了编写高质量AVI demo和代码的一扇门!也许成功,也许没有.不管怎样,你可以任意处理我的代码.
//非常感谢 Fredster提供face AVI文件.Face是他发来的六个AVI动画中的一个.他没提出任何问题和条件.他以他的方式帮助了我,
//谢谢他!
//更要感谢Jonathan de Blok,要没要她,本文就不会有.他给我发来他的AVI播放器的代码,这使我对AVI格式产生了兴趣.
//他也回答了我问的关于他的代码的问题.但重要的是我并没有借鉴或抄袭他的代码,他的代码只是帮助我理解AVI播放器的运行机制.
//我的播放器的打开,解帧和播放AVI文件用的是不同的代码!
//感谢给予帮助的所有人,包括所有参观者!若没有你们,我的网站不值一文!!!
