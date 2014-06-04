#include "myglwidget.h"

//从资源文件中载入图像:
//如何把图像数据保存到*.exe程序中，使用Windows的资源文件吧，它既简单又实用。

//欢迎来到NeHe教程第38课。离上节课的写作已经有些时日了，加上写了一整天的code，也许笔头已经开始生锈了 :)
//现在你已经学会了如何做方格贴图，如何读入bitmap及各种光栅图像...那么如何做三角形贴图，又如何在.exe文件中体现你的纹理呢？
//我每每被问及这两个问题，可是一旦你看到他们是多么简单，你就会大骂自己居然没有想到过 :)
//我不会事无巨细地解释每一个细节，只需给你一些抓图，就明白了。我将基于最新的code，请在主页"NeHeGL I Basecode"下或者
//这张网页最下面下载。
//首先，我们把图像加载入资源文件。我向大家已经知道怎么做了，只是，你忽略了几步，于是值得到一些无用的资源文件。
//里面有bitmap文件，却无法使用。
//还记得吧？我们使用Visual C++ 6.0 做的。如果你使用其它工具，这页教材关于资源的部分（尤其是那些图）完全不适用。
//* 暂时你只能用24bit BMP 图像。如果读8bit BMP文件要写很多额外的code。我很希望听到你们谁有更小的/更好的loader。
//我这里的读入8bit 和 24bit BMP 的code实在臃肿。用LoadImage就可以。

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(3, &m_textures[0]);
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

//nit code没有什么新鲜的，只是增加了LoadGLTextures()调用上面的code。清屏的颜色是黑色，不进行深度检测，这样比较快。启用纹理映
//射和混色效果。
void MyGLWidget::initializeGL()
{
    loadGLTexture();//载入纹理
    glClearColor (0.0f, 0.0f, 0.0f, 0.5f);// 设置背景
    glClearDepth (1.0f);
    glDepthFunc (GL_LEQUAL);
    glDisable(GL_DEPTH_TEST);// 启用深度测试
    glShadeModel (GL_SMOOTH);
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_TEXTURE_2D);// 启用2D纹理
    glBlendFunc(GL_ONE,GL_SRC_ALPHA);// 使用混合
    glEnable(GL_BLEND);
}

//接下来看看绘制代码。在这部分我将讲解如何用尽可能简单的方式将一个图像映到两个三角形上。有些人认为有理由相信，一个图像到三角形
//上的单一映射是不可能的。
//实际上，你可以轻而易举地将图像映到任何形状的区域内。使得图像与边界匹配或者完全不考虑形式。根本没关系的。（译者：我想作者的意
//思是，从长方形到三角形的解析影射是不存在的，但不考虑那么多的话，任意形状之间的连续影射总是可以存在的。他说的使纹理与边界匹配
//，大概是指某一种参数化的方法，简单地说使得扭曲最小。）
//首先清屏，循环润色50个蝴蝶对象。
void MyGLWidget::paintGL()
{
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (int loop=0; loop<50; loop++)
    {
        //调用glLoadIdentify()重置投影矩阵，然后选择对象的纹理。用glTranslatef()为蝴蝶定位，然后沿x轴旋转45度。使之向观众微略倾斜，这
        //样比较有立体感。最后沿z轴旋转，蝴蝶就旋转下落了。
        glLoadIdentity ();// 重置矩阵
        glBindTexture(GL_TEXTURE_2D, m_textures[m_obj[loop].tex]);// 绑定纹理
        glTranslatef(m_obj[loop].x,m_obj[loop].y,m_obj[loop].z);// 绘制物体
        glRotatef(45.0f,1.0f,0.0f,0.0f);
        glRotatef((m_obj[loop].spinz),0.0f,0.0f,1.0f);
        //其实到三角形上的映射和到方形上并没有很大区别。只是你只有三个定点，要小心一点。
        //下面的code中，我们将会值第一个三角形。从一个设想的方形的右上角开始，到左上角，再到左下角。润色的结果像下面这样：
        //注意半个蝴蝶出现了。另外半个出现在第二个三角形里。同样地将三个纹理坐标与顶点坐标非别对应，
        //这给出充分的信息定义一个三角形上的映射。
        glBegin(GL_TRIANGLES);
            glTexCoord2f(1.0f,1.0f); glVertex3f( 1.0f, 1.0f, 0.0f);
            glTexCoord2f(0.0f,1.0f); glVertex3f(-1.0f, 1.0f, m_obj[loop].flap);
            glTexCoord2f(0.0f,0.0f); glVertex3f(-1.0f,-1.0f, 0.0f);
            //下面的code润色另一半。同上，只是我们的三角变成了从右上到左下，再到右下。
            //第一个三角形的第二点和第二个三角形的第三点（也就是翅膀的尖端）在z方向往复运动（即z=-1.0f和1.0f之间），
            //两个三角形沿着蝴蝶的身体折叠起来，产生拍打的效果，简易可行。
            glTexCoord2f(1.0f,1.0f); glVertex3f( 1.0f, 1.0f, 0.0f);
            glTexCoord2f(0.0f,0.0f); glVertex3f(-1.0f,-1.0f, 0.0f);
            glTexCoord2f(1.0f,0.0f); glVertex3f( 1.0f,-1.0f, m_obj[loop].flap);
        glEnd();
        //下面一段通过从obj[loop].y中递减obj[loop].yi使蝴蝶自上而下运动。
        //spinz值递增spinzi（可正可负）flap递增fi.fi的正负取决于翅膀向上还是向下运动。
        //移动，选择图像
        m_obj[loop].y-=m_obj[loop].yi;
        m_obj[loop].spinz+=m_obj[loop].spinzi;
        m_obj[loop].flap+=m_obj[loop].fi;
        //当蝴蝶向下运行时，需要检查是否越出屏幕，如果是，就调用SetObject(loop)来给蝴蝶赋新的纹理，新下落速度等。
        if (m_obj[loop].y<-18.0f)								//判断是否超出了屏幕，如果是重置它
        {
            m_obj[loop] = object();
        }
        //翅膀拍打的时候，还要检查flap是否小于-1.0f或大于1.0f，如果是，令fi=-fi，以改变运动方向。Sleep(15)是用来减缓运行速度，
        //每帧15毫秒。在我朋友的机器上，这让蝴蝶疯狂的飞舞。不过我懒得改了:)
        if ((m_obj[loop].flap>1.0f) || (m_obj[loop].flap<-1.0f))
        {
            m_obj[loop].fi=-m_obj[loop].fi;
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
    QVector<QImage> images;
    images.push_back(QImage(":/Data/Butterfly1.bmp"));
    images.push_back(QImage(":/Data/Butterfly2.bmp"));
    images.push_back(QImage(":/Data/Butterfly3.bmp"));
    glGenTextures(3, &m_textures[0]);// 创建三个纹理
    for(int i = 0, iend = 3; i < iend; i++)
    {
        QImage image = images[i].convertToFormat(QImage::Format_RGB888);
        image = image.mirrored();

        glBindTexture(GL_TEXTURE_2D, m_textures[i]);// 绑定位图
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);// 设置纹理过滤器
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        // 创建纹理
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image.width(), image.height(),
                          GL_BGR_EXT, GL_UNSIGNED_BYTE, image.bits());
    }
}

//希望你在这一课学的开心。也希望通过这一课，从资源文件里读取纹理，和三角形映射的过程变得比较容易理解。我花五分钟又冲读了一遍，
//感觉还好。如果你还有什么问题，尽管问。我希望我的讲义尽可能好，因此期待您的任何回应。
