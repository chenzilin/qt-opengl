#include "myglwidget.h"
#include "milkshapemodel.h"

//模型加载:
//你知道大名鼎鼎的Milkshape3D建模软件么，我们将加载它的模型，当然你可以加载任何你认为不错的模型。
//这篇渲染模型的文章是由Brett Porter所写的。
//这篇教程的代码是从PortaLib3D中提取出来的，PortaLib3D是一个可以读取3D文件实用库。
//这篇教程的代码是以第六课为基础的，我们只讨论改变的部分。
//这课中使用的模型是从Milkshape3D中提取出来的，Milkshape3D是一个非常好的建模软件，
//它包含了自己的文件格式，所以你能很容易去分析和理解。
//但是文件格式并不能使你加载一个模型，你必须自己定义一个结构去保存数据，
//接着把数据读入那个结构，我们将告诉你如何定义这样一个结构。
//模型的定义在model.h中，好吧我们开始吧：

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_pModel(NULL), m_yrot(0.0f)
{
    showNormal();
    startTimer(15);
    m_pModel = new MilkshapeModel();
    QString filePath = QApplication::applicationDirPath() + "/Data/Model.ms3d";
    m_pModel->loadModelData(filePath.toLatin1().data());
    m_pModel->reloadTextures();
}

MyGLWidget::~MyGLWidget()
{
    delete m_pModel;
}

//下面的代码的作用是重新设置OpenGL场景的大小，而不管窗口的大小是否已经改变(假定您没有使用全屏模式)。
//甚至您无法改变窗口的大小时(例如您在全屏模式下)，它至少仍将运行一次--在程序开始时设置我们的透视图。
//OpenGL场景的尺寸将被设置成它显示时所在窗口的大小。
void MyGLWidget::resizeGL(int w, int h)
{
    if (h==0)													// Prevent A Divide By Zero By
	{
		h=1;													// Making Height Equal One
	}

	glViewport(0,0,w,h);									// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);									// Select The Projection Matrix
	glLoadIdentity();												// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)w/(GLfloat)h,1.0f,1000.0f);	// View Depth of 1000

	glMatrixMode(GL_MODELVIEW);										// Select The Modelview Matrix
	glLoadIdentity();												// Reset The Modelview Matrix
}

//接下的代码段中，我们将对OpenGL进行所有的设置。我们将设置清除屏幕所用的颜色，打开深度缓存，启用smooth shading(阴影平滑)，等等。
//这个例程直到OpenGL窗口创建之后才会被调用。此过程将有返回值。但我们此处的初始化没那么复杂，现在还用不着担心这个返回值。
void MyGLWidget::initializeGL()
{
    if(m_pModel)
    {
        m_pModel->reloadTextures();// Loads Model Textures
    }
    glEnable(GL_TEXTURE_2D);// Enable Texture Mapping ( NEW )
    glShadeModel(GL_SMOOTH);// Enable Smooth Shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);// Black Background
    glClearDepth(1.0f);// Depth Buffer Setup
    glEnable(GL_DEPTH_TEST);// Enables Depth Testing
    glDepthFunc(GL_LEQUAL);// The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);// Really Nice Perspective Calculations
}

//完成了初始化操作，我们来实际绘制我们的模型
void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
    glLoadIdentity();// Reset The Modelview Matrix
    gluLookAt( 75, 75, 75, 0, 0, 0, 0, 1, 0 );// (3) Eye Postion (3) Center Point (3) Y-Axis Up Vector
    glRotatef(m_yrot,0.0f,1.0f,0.0f);// Rotate On The Y-Axis By yrot
    m_pModel->draw();// Draw The Model
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
    m_yrot+=1.0f;// Increase yrot By One
    updateGL();
    QGLWidget::timerEvent(event);
}

//简单吧？下一步我们该做什么？在以后的教程中，我将会加入骨骼动画的知识，到时候见吧
