#include "myglwidget.h"

//Blitter 函数:
//类似于DirectDraw的blit函数，过时的技术，我们有实现了它。它非常的简单，就是把一块纹理贴到另一块纹理上。
//这篇文章是有Andreas Lffler所写的，它写了一份原始的教程。过了几天，Rob Fletcher发了封邮件给我，他重新改写了所有的代码，
//我在它的基础上把glut的框架变换为Win32的框架。
//现在让我们开始吧！

//下面的函数为w*h的图像分配内存
P_TEXTURE_IMAGE allocateTextureBuffer(GLint w, GLint h, GLint f)
{
    P_TEXTURE_IMAGE ti=NULL;
    unsigned char *c=NULL;
    ti = (P_TEXTURE_IMAGE)malloc(sizeof(TEXTURE_IMAGE));// 分配图像结构内存
    if( ti != NULL )
    {
        ti->width  = w;// 设置宽度
        ti->height = h;// 设置高度
        ti->format = f;// 设置格式
        // 分配w*h*f个字节
        c = (unsigned char *)malloc( w * h * f);
        if ( c != NULL )
        {
            ti->data = c;
        }
        else
        {
            qDebug() << "no enough memory.";
            return NULL;
        }
    }
    else
    {
        qDebug() << "no enough memory.";
        return NULL;
    }
    return ti;									// 返回指向图像数据的指针
}

//下面的函数释放分配的内存
// 释放图像内存
void deallocateTexture(P_TEXTURE_IMAGE t)
{
    if(t)
    {
        if(t->data)
        {
            free(t->data);// 释放图像内存
        }
        free(t);// 释放图像结构内存
    }
}

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_xrot(0.0f),
    m_yrot(0.0f), m_zrot(0.0f)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(1, &m_texture[0]);
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

//初始化代码基本不变，我们使用新的函数，加载*.raw纹理。并把纹理t2的一部分blit到t1中混合，接着按常规的方法设置2D纹理。
void MyGLWidget::initializeGL()
{
    m_t1 = allocateTextureBuffer(256, 256, 4);// 为图像t1分配内存
    if(readTextureData(":/data/Monitor.raw", m_t1) == 0)// 读取图像数据
    {
        QMessageBox::warning(this, tr("Warning"), tr("Read Monitor.raw error."));
    }
    m_t2 = allocateTextureBuffer(256, 256, 4);// 为图像t2分配内存
    if(readTextureData(":/data/GL.raw", m_t2) == 0)// 读取图像数据
    {
        QMessageBox::warning(this, tr("Warning"), tr("Read GL.raw error."));
    }
    //把图像t2的（127，127）-（256，256）部分和图像t1的（64，64，196，196）部分混合
    blit(m_t2, m_t1,127,127,128,128,64,64,1,127);
    //下面的代码和前面一样，释放分配的空间，创建纹理
    buildTexture(m_t1);//t1图像加载为纹理
    deallocateTexture(m_t1);// 释放图像数据
    deallocateTexture(m_t2);
    glEnable(GL_TEXTURE_2D);// 使用2D纹理
    glShadeModel(GL_SMOOTH);// 使用光滑着色
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);// 设置背景色为黑色
    glClearDepth(1.0);// 设置深度缓存清楚值为1
    glEnable(GL_DEPTH_TEST);// 使用深度缓存
    glDepthFunc(GL_LESS);// 设置深度测试函数
}

//下面的代码绘制一个盒子
void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// 清楚颜色缓存和深度缓存
    glLoadIdentity();
    glTranslatef(0.0f,0.0f,-5.0f);
    glRotatef(m_xrot,1.0f,0.0f,0.0f);
    glRotatef(m_yrot,0.0f,1.0f,0.0f);
    glRotatef(m_zrot,0.0f,0.0f,1.0f);
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);
    glBegin(GL_QUADS);
        // 前面
        glNormal3f( 0.0f, 0.0f, 1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
        // 后面
        glNormal3f( 0.0f, 0.0f,-1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
        // 上面
        glNormal3f( 0.0f, 1.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
        // 下面
        glNormal3f( 0.0f,-1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
        // 右面
        glNormal3f( 1.0f, 0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
        // 左面
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
    glEnd();
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

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    m_xrot+=0.3f;
    m_yrot+=0.2f;
    m_zrot+=0.4f;
    updateGL();
    QGLWidget::timerEvent(event);
}

int MyGLWidget::readTextureData(QString filename, P_TEXTURE_IMAGE buffer)
{
    QFile file(filename);
    int done=0;
    int stride = buffer->width * buffer->format;// 记录每一行的宽度，以字节为单位
    char *p = NULL;
    file.open(QIODevice::ReadOnly);// 打开文件
    //如果文件存在，我们通过一个循环读取我们的纹理，我们从图像的最下面一行，一行一行的读取图像。
    for(int i = buffer->height-1; i >= 0 ; i-- )// 循环所有的行，从最下面以行开始，一行一行的读取
    {
        p = (char*)buffer->data + (i * stride );
        for (int j = 0; j < buffer->width ; j++ )// 读取每一行的数据
        {
            //下面的循环读取每一像素的数据，并把alpha设为255
            for (int k = 0 ; k < buffer->format-1 ; k++, p++, done++ )
            {
                file.read(p, 1);// 读取一个字节
            }
            *p = 255;
            p++;// 255存储在alpha通道中
        }
    }
    file.close();// 关闭文件
    return done;// 返回读取的字节数
}

//下面的代码创建一个2D纹理，和前面课程介绍的方法相同
void MyGLWidget::buildTexture (P_TEXTURE_IMAGE tex)
{
    glGenTextures(1, &m_texture[0]);
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, tex->width, tex->height,
                      GL_RGBA, GL_UNSIGNED_BYTE, tex->data);
}

//现在到了blitter函数的地方了，他运行你把一个图像的任意部分复制到另一个图像的任意部分，并混合。
//src为原图像
//dst为目标图像
//src_xstart,src_ystart为要复制的部分在原图像中的位置
//src_width,src_height为要复制的部分的宽度和高度
//dst_xstart,dst_ystart为复制到目标图像时的起始位置
//上面的意思是把原图像中的(src_xstart,src_ystart)-(src_width,src_height)复制到目标图像中(dst_xstart,dst_ystart)-(src_width,src_height)
//blend设置是否启用混合，0为不启用，1为启用
//alpha设置源图像中颜色在混合时所占的百分比
void MyGLWidget::blit(P_TEXTURE_IMAGE src, P_TEXTURE_IMAGE dst, int src_xstart,
                      int src_ystart, int src_width, int src_height,int dst_xstart,
                      int dst_ystart, int blend, int alpha)
{
    unsigned char *s, *d;
    // 掐断alpha的值
    if( alpha > 255 )
    {
        alpha = 255;
    }
    if( alpha < 0 )
    {
        alpha = 0;
    }
    // 判断是否启用混合
    if( blend < 0 )
    {
        blend = 0;
    }
    if( blend > 1 )
    {
        blend = 1;
    }
    d = dst->data + (dst_ystart * dst->width * dst->format);// 要复制的像素在目标图像数据中的开始位置
    s = src->data + (src_ystart * src->width * src->format);// 要复制的像素在源图像数据中的开始位置
    for (int i = 0 ; i < src_height ; i++ )						// 循环每一行
    {
        s = s + (src_xstart * src->format);// 移动到下一个像素
        d = d + (dst_xstart * dst->format);
        for (int j = 0 ; j < src_width ; j++ )// 循环复制一行
        {
            for(int k = 0 ; k < src->format ; k++, d++, s++)// 复制每一个字节
            {
                if (blend)// 如果启用了混合
                {
                    *d = ( (*s * alpha) + (*d * (255-alpha)) ) >> 8;// 根据混合复制颜色
                }
                else
                {
                    *d = *s;						// 否则直接复制
                }
            }
        }
        d = d + (dst->width - (src_width + dst_xstart))*dst->format;// 移动到下一行
        s = s + (src->width - (src_width + src_xstart))*src->format;
    }
}

//好了，现你可以很轻松的绘制很多混合效果。如果你有什么好的建议，请告诉我。
