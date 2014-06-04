#include "myglwidget.h"

//轨迹球实现的鼠标旋转
//使用鼠标旋转物体,很简单也有很多实现方法,这里我们教会你模拟轨迹球来实现它.

//Arcball sphere constants:
//Diameter is       2.0f
//Radius is         1.0f
//Radius squared is 1.0f

const float PI2 = 2.0*3.1415926535f;								// PI Squared

void ArcBall_t::_mapToSphere(const Point2fT* NewPt, Vector3fT* NewVec) const
{
    Point2fT TempPt;
    GLfloat length;

    //Copy paramter into temp point
    TempPt = *NewPt;

    //Adjust point coords and scale down to range of [-1 ... 1]
    TempPt.s.X  =        (TempPt.s.X * this->AdjustWidth)  - 1.0f;
    TempPt.s.Y  = 1.0f - (TempPt.s.Y * this->AdjustHeight);

    //Compute the square of the length of the vector to the point from the center
    length      = (TempPt.s.X * TempPt.s.X) + (TempPt.s.Y * TempPt.s.Y);

    //If the point is mapped outside of the sphere... (length > radius squared)
    if (length > 1.0f)
    {
        GLfloat norm;

        //Compute a normalizing factor (radius / sqrt(length))
        norm    = 1.0f / FuncSqrt(length);

        //Return the "normalized" vector, a point on the sphere
        NewVec->s.X = TempPt.s.X * norm;
        NewVec->s.Y = TempPt.s.Y * norm;
        NewVec->s.Z = 0.0f;
    }
    else    //Else it's on the inside
    {
        //Return a vector to a point mapped inside the sphere sqrt(radius squared - length)
        NewVec->s.X = TempPt.s.X;
        NewVec->s.Y = TempPt.s.Y;
        NewVec->s.Z = FuncSqrt(1.0f - length);
    }
}

//Create/Destroy
ArcBall_t::ArcBall_t(GLfloat NewWidth, GLfloat NewHeight)
{
    //Clear initial values
    this->StVec.s.X     =
    this->StVec.s.Y     =
    this->StVec.s.Z     =

    this->EnVec.s.X     =
    this->EnVec.s.Y     =
    this->EnVec.s.Z     = 0.0f;

    //Set initial bounds
    this->setBounds(NewWidth, NewHeight);
}

//Mouse down
void ArcBall_t::click(const Point2fT* NewPt)
{
    //Map the point to the sphere
    this->_mapToSphere(NewPt, &this->StVec);
}

//Mouse drag, calculate rotation
void ArcBall_t::drag(const Point2fT* NewPt, Quat4fT* NewRot)
{
    //Map the point to the sphere
    this->_mapToSphere(NewPt, &this->EnVec);
    //Return the quaternion equivalent to the rotation
    if (NewRot)
    {
        Vector3fT  Perp;

        //Compute the vector perpendicular to the begin and end vectors
        Vector3fCross(&Perp, &this->StVec, &this->EnVec);

        //Compute the length of the perpendicular vector
        if (Vector3fLength(&Perp) > Epsilon)    //if its non-zero
        {
            //We're ok, so return the perpendicular vector as the transform after all
            NewRot->s.X = Perp.s.X;
            NewRot->s.Y = Perp.s.Y;
            NewRot->s.Z = Perp.s.Z;
            //In the quaternion values, w is cosine (theta / 2), where theta is rotation angle
            NewRot->s.W= Vector3fDot(&this->StVec, &this->EnVec);
        }
        else                                    //if its zero
        {
            //The begin and end vectors coincide, so return an identity transform
            NewRot->s.X =
            NewRot->s.Y =
            NewRot->s.Z =
            NewRot->s.W = 0.0f;
        }
    }
}

//轨迹球控制
//By Terence J. Grant (tjgrant@tatewake.com)
//如果只用鼠标来控制你的模型是不是很酷?轨迹球可以帮你做到这一点，我将告诉你我的实现，你可以把它应用在你的工程里。
//我的实现是基于Bretton Wade’s，它是基于Ken Shoemake’s 实现的，最初的版本，你可以从游戏编程指南这本图上找到。
//但我还是修正了一些错误，并优化了它。
//轨迹球实现的内容就是把二维的鼠标点映射到三维的轨迹球，并基于它完成旋转变化。
//为了完成这个设想，首先我们把鼠标坐标映射到[-1，1]之间，它很简单：
//MousePt.X  =  ((MousePt.X / ((Width  -1) / 2)) -1);
//MousePt.Y  = -((MousePt.Y / ((Height -1) / 2))-1);

Matrix4fT Transform = {  1.0f,  0.0f,  0.0f,  0.0f,
                   0.0f,  1.0f,  0.0f,  0.0f,
                   0.0f,  0.0f,  1.0f,  0.0f,
                   0.0f,  0.0f,  0.0f,  1.0f };
Matrix3fT LastRot = { 1.0f, 0.0f, 0.0f,
0.0f, 1.0f, 0.0f,
0.0f, 0.0f, 1.0f };

Matrix3fT ThisRot = { 1.0f, 0.0f, 0.0f,
0.0f, 1.0f, 0.0f,
0.0f, 0.0f, 1.0f };

//在上面定义的变量中，transform是我们获得的最终的变换矩阵，lastRot是上一次鼠标拖动得到的旋转矩阵，thisRot为这次鼠标拖动得到的旋转矩阵。
//当我们点击鼠标时，创建一个单位旋转矩阵，当我们拖动鼠标时，这个矩阵跟踪鼠标的变化。
//为了更新鼠标的移动范围，我们在函数ReshapeGL中加入下面一行：

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_arcBall(640.0f, 480.0f),
    m_leftButtonClicked(false)
{
    showNormal();
}

MyGLWidget::~MyGLWidget()
{
    gluDeleteQuadric(m_quadratic);
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

    m_arcBall.setBounds((GLfloat)w, (GLfloat)h);                 //*NEW* Update mouse bounds for arcball
}

//接下的代码段中，我们将对OpenGL进行所有的设置。我们将设置清除屏幕所用的颜色，打开深度缓存，启用smooth shading(阴影平滑)，等等。
//这个例程直到OpenGL窗口创建之后才会被调用。此过程将有返回值。但我们此处的初始化没那么复杂，现在还用不着担心这个返回值。
void MyGLWidget::initializeGL()
{
    glClearColor (0.0f, 0.0f, 0.0f, 0.5f);							// Black Background
    glClearDepth (1.0f);											// Depth Buffer Setup
    glDepthFunc (GL_LEQUAL);										// The Type Of Depth Testing (Less Or Equal)
    glEnable (GL_DEPTH_TEST);										// Enable Depth Testing
    glShadeModel (GL_FLAT);											// Select Flat Shading (Nice Definition Of Objects)
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);				// Set Perspective Calculations To Most Accurate

    m_quadratic=gluNewQuadric();										// Create A Pointer To The Quadric Object
    gluQuadricNormals(m_quadratic, GLU_SMOOTH);						// Create Smooth Normals
    gluQuadricTexture(m_quadratic, GL_TRUE);							// Create Texture Coords

    glEnable(GL_LIGHT0);											// Enable Default Light
    glEnable(GL_LIGHTING);											// Enable Lighting

    glEnable(GL_COLOR_MATERIAL);									// Enable Color Material
}

void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				// Clear Screen And Depth Buffer
    glLoadIdentity();												// Reset The Current Modelview Matrix
    glTranslatef(-1.5f,0.0f,-6.0f);									// Move Left 1.5 Units And Into The Screen 6.0

    glPushMatrix();													// NEW: Prepare Dynamic Transform
    glMultMatrixf(Transform.M);										// NEW: Apply Dynamic Transform
    glColor3f(0.75f,0.75f,1.0f);
    torus(0.30f,1.00f);
    glPopMatrix();													// NEW: Unapply Dynamic Transform

    glLoadIdentity();												// Reset The Current Modelview Matrix
    glTranslatef(1.5f,0.0f,-6.0f);									// Move Right 1.5 Units And Into The Screen 7.0

    glPushMatrix();													// NEW: Prepare Dynamic Transform
    glMultMatrixf(Transform.M);										// NEW: Apply Dynamic Transform
    glColor3f(1.0f,0.75f,0.75f);
    gluSphere(m_quadratic,1.3f,20,20);
    glPopMatrix();													// NEW: Unapply Dynamic Transform
    glFlush ();														// Flush The GL Rendering Pipeline
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

void MyGLWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        Matrix3fSetIdentity(&LastRot);								// Reset Rotation
        Matrix3fSetIdentity(&ThisRot);								// Reset Rotation
        Matrix4fSetRotationFromMatrix3f(&Transform, &ThisRot);		// Reset Rotation
        updateGL();
    }
    else if(event->button() == Qt::LeftButton)
    {
        m_mousePt.s.X = (GLfloat)(event->pos().x());
        m_mousePt.s.Y = (GLfloat)(event->pos().y());
        LastRot = ThisRot;// Set Last Static Rotation To Last Dynamic One
        m_arcBall.click(&m_mousePt);// Update Start Vector And Prepare For Dragging
        m_leftButtonClicked = true;
    }
    QGLWidget::mousePressEvent(event);
}

void MyGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(m_leftButtonClicked)
    {
        m_mousePt.s.X = (GLfloat)(event->pos().x());
        m_mousePt.s.Y = (GLfloat)(event->pos().y());

        Quat4fT ThisQuat;
        m_arcBall.drag(&m_mousePt, &ThisQuat);// Update End Vector And Get Rotation As Quaternion
        Matrix3fSetRotationFromQuat4f(&ThisRot, &ThisQuat);// Convert Quaternion Into Matrix3fT
        Matrix3fMulMatrix3f(&ThisRot, &LastRot);// Accumulate Last Rotation Into This One
        Matrix4fSetRotationFromMatrix3f(&Transform, &ThisRot);// Set Our Final Transform's Rotation From This One
        updateGL();
    }
    QGLWidget::mouseMoveEvent(event);
}

void MyGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_leftButtonClicked = false;
    updateGL();
    QGLWidget::mouseReleaseEvent(event);
}

void MyGLWidget::torus(float MinorRadius, float MajorRadius)					// Draw A Torus With Normals
{
    int i, j;
    glBegin( GL_TRIANGLE_STRIP );									// Start A Triangle Strip
        for (i=0; i<20; i++ )										// Stacks
        {
            for (j=-1; j<20; j++)									// Slices
            {
                float wrapFrac = (j%20)/(float)20;
                float phi = PI2*wrapFrac;
                float sinphi = float(sin(phi));
                float cosphi = float(cos(phi));

                float r = MajorRadius + MinorRadius*cosphi;

                glNormal3f(float(sin(PI2*(i%20+wrapFrac)/(float)20))*cosphi, sinphi, float(cos(PI2*(i%20+wrapFrac)/(float)20))*cosphi);
                glVertex3f(float(sin(PI2*(i%20+wrapFrac)/(float)20))*r,MinorRadius*sinphi,float(cos(PI2*(i%20+wrapFrac)/(float)20))*r);

                glNormal3f(float(sin(PI2*(i+1%20+wrapFrac)/(float)20))*cosphi, sinphi, float(cos(PI2*(i+1%20+wrapFrac)/(float)20))*cosphi);
                glVertex3f(float(sin(PI2*(i+1%20+wrapFrac)/(float)20))*r,MinorRadius*sinphi,float(cos(PI2*(i+1%20+wrapFrac)/(float)20))*r);
            }
        }
    glEnd();														// Done Torus
}

//在这一课中，我已试着尽量详细解释一切。每一步都与设置有关，并创建了一个全屏OpenGL程序。
//当您按下ESC键程序就会退出，并监视窗口是否激活。
//如果您有什么意见或建议请给我EMAIL。如果您认为有什么不对或可以改进，请告诉我。
//我想做最好的OpenGL教程并对您的反馈感兴趣。
