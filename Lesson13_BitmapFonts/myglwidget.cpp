#include "myglwidget.h"

//图像字体:
//这一课我们将创建一些基于2D图像的字体，它们可以缩放，但不能旋转，并且总是面向前方，但作为基本的显示来说，我想已经够了。

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_cnt1(0.0f)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
    m_our_font.clean();
}

void MyGLWidget::resizeGL(int w, int h)
{
    if(h == 0)// 防止被零除
    {
        h = 1;// 将高设为1
    }
    glViewport(0, 0, w, h); //重置当前的视口
    glMatrixMode(GL_PROJECTION);// 选择投影矩阵
    glLoadIdentity();// 重置投影矩阵
    //设置视口的大小
    gluPerspective(45.0f,(GLfloat)w/(GLfloat)h,0.1f,100.0f);

    glMatrixMode(GL_MODELVIEW);	//选择模型观察矩阵
    glLoadIdentity(); // 重置模型观察矩阵
}

void MyGLWidget::initializeGL()
{
    glShadeModel(GL_SMOOTH); // 启用阴影平滑
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // 黑色背景
    glClearDepth(1.0f);	// 设置深度缓存
    glEnable(GL_DEPTH_TEST); // 启用深度测试
    glDepthFunc(GL_LEQUAL);	// 所作深度测试的类型
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// 告诉系统对透视进行修正
    QString fontPath = QApplication::applicationDirPath() + "/Test.ttf";
    m_our_font.init(fontPath.toLatin1().data(), 24);
}

void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// 清除屏幕及深度缓存
    glLoadIdentity();// 重置当前的模型观察矩阵// 重置当前的模型观察矩阵
    glTranslatef(0.0f,0.0f,-1.0f);// 移入屏幕一个单位

    // 红色文字
    //下面我们使用一些奇妙的数学方法来产生颜色变化脉冲。如果你不懂我在做什么你也不必担心。我喜欢利用教多的变量和教简单的方法来达到我的目的。
    //这样，我使用那两个用来改变文字在屏幕上位置的计数器来改变红、绿、蓝这些颜色。红色值使用COS和计数器1在-1.0到1.0之间变化。
    //绿色值使用SIN和计数器2也在-1.0到1.0之间变化。蓝色值使用COS和计数器1和2在0.5到1.5之间变化。于是，蓝色值就永远不会等于0，
    //文字的颜色也永远不会消失。笨办法，但很管用。
    glColor3f(1.0f*float(cos(m_cnt1)),1.0f*float(sin(m_cnt2)),1.0f-0.5f*float(cos(m_cnt1+m_cnt2)));
    glPushMatrix();
    glLoadIdentity();
    freetype::print(m_our_font, 320*float(cos(m_cnt1)),
                    200 + 200*float(sin(m_cnt2)), "Active OpenGL Text With NeHe - %7.2f", m_cnt1);
    glPopMatrix();
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
    m_cnt1+=0.051f;										// Increase The First Counter
    m_cnt2+=0.005f;										// Increase The First Counter
    updateGL();
    QGLWidget::timerEvent(event);
}

//好了，用于使用位图字体的所有一切都在你的OpenGL程序中了。我在网上寻找过与这篇教程相似的文章，但没有找到。
//或许我的网站是第一个涉及这个主题的C代码的网站吧。不管怎样，享用这篇教程，快乐编码！
