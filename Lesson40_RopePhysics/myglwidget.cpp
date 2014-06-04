#include "myglwidget.h"

//绳子的模拟:
//怎样模拟一根绳子呢，把它想象成一个个紧密排列的点，怎么样有了思路了吧，在这一课你你将学会怎样建模，简单吧，你能模拟更多。

//绳索模拟
//在这个教程里我们将模拟一段绳索，我们是在39课的基础上进行的。
//在物理模拟中，我们必须设置各个物理量，就像它们在自然界中的行为一样。模拟中的运动并不一定和自然界相同，我们使用的运动模型，必须和我们需要模拟的目的有关，
//目的决定了它的精确度。要知道我们的目标不是模拟原子和分子，也不是模拟成千上万的粒子系。首先我们需要确定我们模拟的目标，才能创建我们的物理模型。它和下面内容相关：
//1. 运动的数学表示
//2. 执行模拟的计算机的速度
//1. 运动的数学表示:
//这个问题决定了我们使用何种数学方程来模拟运动，使用经典力学还是量子力学。
//2. 执行模拟的计算机的速度:
//计算机的速度决定了我们可以模拟的精度。

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_ropeSimulation(NULL)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
    m_ropeSimulation->release();												// Release The ropeSimulation
    delete m_ropeSimulation;													// Delete The ropeSimulation
    m_ropeSimulation = NULL;
}

void MyGLWidget::resizeGL(int w, int h)
{
    glViewport (0, 0, (GLsizei)(w), (GLsizei)(h));				// Reset The Current Viewport
    glMatrixMode (GL_PROJECTION);										// Select The Projection Matrix
    glLoadIdentity ();													// Reset The Projection Matrix
    gluPerspective (45.0f, (GLfloat)(w)/(GLfloat)(h),			// Calculate The Aspect Ratio Of The Window
                    1.0f, 100.0f);
    glMatrixMode (GL_MODELVIEW);										// Select The Modelview Matrix
    glLoadIdentity ();													// Reset The Modelview Matrix
}

void MyGLWidget::initializeGL()
{
    m_ropeSimulation = new RopeSimulation(80,// 80 质点
                                          0.05f,// 每个质点50g
                                          10000.0f,// 弹性系数
                                          0.05f,// 质点之间的距离
                                          0.2f,// 弹簧的内摩擦力
                                          Vector3D(0, -9.81f, 0),// 万有引力
                                          0.02f,// 空气摩擦力
                                          100.0f,// 地面反作用系数
                                          0.2f,// 地面摩擦系数
                                          2.0f,// 地面缓冲系数
                                          -1.5f);// 地面高度
    m_ropeSimulation->getMass(m_ropeSimulation->numOfMasses - 1)->vel.z = 10.0f;
    glClearColor (0.0f, 0.0f, 0.0f, 0.5f);									// Black Background
    glClearDepth (1.0f);													// Depth Buffer Setup
    glShadeModel (GL_SMOOTH);												// Select Smooth Shading
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);						// Set Perspective Calculations To Most Accurate
}

void MyGLWidget::paintGL()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity ();														// Reset The Modelview Matrix

    // Position Camera 40 Meters Up In Z-Direction.
    // Set The Up Vector In Y-Direction So That +X Directs To Right And +Y Directs To Up On The Window.
    gluLookAt(0, 0, 4, 0, 0, 0, 0, 1, 0);

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);					// Clear Screen And Depth Buffer

    // Draw A Plane To Represent The Ground (Different Colors To Create A Fade)
    glBegin(GL_QUADS);
        glColor3ub(0, 0, 255);												// Set Color To Light Blue
        glVertex3f(20, m_ropeSimulation->groundHeight, 20);
        glVertex3f(-20, m_ropeSimulation->groundHeight, 20);
        glColor3ub(0, 0, 0);												// Set Color To Black
        glVertex3f(-20, m_ropeSimulation->groundHeight, -20);
        glVertex3f(20, m_ropeSimulation->groundHeight, -20);
    glEnd();

    // Start Drawing Shadow Of The Rope
    glColor3ub(0, 0, 0);													// Set Color To Black
    for (int a = 0; a < m_ropeSimulation->numOfMasses - 1; ++a)
    {
        Mass* mass1 = m_ropeSimulation->getMass(a);
        Vector3D* pos1 = &mass1->pos;

        Mass* mass2 = m_ropeSimulation->getMass(a + 1);
        Vector3D* pos2 = &mass2->pos;

        glLineWidth(2);
        glBegin(GL_LINES);
            glVertex3f(pos1->x, m_ropeSimulation->groundHeight, pos1->z);		// Draw Shadow At groundHeight
            glVertex3f(pos2->x, m_ropeSimulation->groundHeight, pos2->z);		// Draw Shadow At groundHeight
        glEnd();
    }
    // Drawing Shadow Ends Here.

    // Start Drawing The Rope.
    glColor3ub(255, 255, 0);												// Set Color To Yellow
    for (int a = 0; a < m_ropeSimulation->numOfMasses - 1; ++a)
    {
        Mass* mass1 = m_ropeSimulation->getMass(a);
        Vector3D* pos1 = &mass1->pos;

        Mass* mass2 = m_ropeSimulation->getMass(a + 1);
        Vector3D* pos2 = &mass2->pos;

        glLineWidth(4);
        glBegin(GL_LINES);
            glVertex3f(pos1->x, pos1->y, pos1->z);
            glVertex3f(pos2->x, pos2->y, pos2->z);
        glEnd();
    }
}

void MyGLWidget::keyPressEvent(QKeyEvent *event)
{
    Vector3D ropeConnectionVel;												// Create A Temporary Vector3D
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
        case Qt::Key_Right:
        {
            ropeConnectionVel.x += 3.0f;
            break;
        }
        case Qt::Key_Left:
        {
            ropeConnectionVel.x -= 3.0f;
            break;
        }
        case Qt::Key_Up:
        {
            ropeConnectionVel.z -= 3.0f;
            break;
        }
        case Qt::Key_Down:
        {
            ropeConnectionVel.z += 3.0f;
            break;
        }
        case Qt::Key_Home:
        {
            ropeConnectionVel.y += 3.0f;
            break;
        }
        case Qt::Key_End:
        {
            ropeConnectionVel.y -= 3.0f;
            break;
        }
        case Qt::Key_Escape:
        {
            qApp->exit();
            break;
        }
    }
    m_ropeSimulation->setRopeConnectionVel(ropeConnectionVel);
    QGLWidget::keyPressEvent(event);
}

void MyGLWidget::keyReleaseEvent(QKeyEvent *event)
{
    Vector3D ropeConnectionVel;
    m_ropeSimulation->setRopeConnectionVel(ropeConnectionVel);
    QGLWidget::keyReleaseEvent(event);
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    update(15);
    updateGL();
    QGLWidget::timerEvent(event);
}

void MyGLWidget::update(quint32 milliseconds)// 执行模拟
{
    float dt = milliseconds / 1000.0f;// 经过的秒数
    float maxPossible_dt = 0.002f;// 模拟间隔
    int numOfIterations = (int)(dt / maxPossible_dt) + 1;// 模拟次数
    if (numOfIterations != 0)												// Avoid Division By Zero
        dt = dt / numOfIterations;											// dt Should Be Updated According To numOfIterations
    for (int a = 0; a < numOfIterations; ++a)// 执行模拟
        m_ropeSimulation->operate(dt);
}

//我相信这一个教会了你很多，从最开始的模型的建立，到完成最后的代码。有了这个基础，相信你会创造出很多更有意思的代码！
