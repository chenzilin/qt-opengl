#include "myglwidget.h"

//影子:
//这是一个高级的主题，请确信你已经熟练的掌握了基本的OpenGL，并熟悉蒙板缓存。当然它会给你留下深刻的印象的。
//欢迎来到另一个有些复杂的课程，阴影。这一课的效果好的有些让人不可思议，阴影可以变形，混合在其他的物体上。
//这一课要求你必须对OpenGL比较了解，它假设你知道许多OpenGL的知识，你必须知道蒙板缓存，基本的OpenGL步骤。
//如果你对这些不太熟悉，我建议你可以看看前面的教程。当然，在这一课里，我们用到了很多数学知识，请准备好一本数学手册在你的身边。
//首先我们定义阴影体可以延伸的距离。

// 定义阴影体可以延伸的距离
#define INFINITY	100
typedef float GLvector4f[4];// Typedef's For VMatMult Procedure
typedef float GLmatrix16f[16];// Typedef's For VMatMult Procedure

//下面的函数完成矩阵M与向量V的乘法M=M*V
void VMatMult(GLmatrix16f M, GLvector4f v)
{
    GLfloat res[4];// 保存中间计算结果
    res[0]=M[0]*v[0]+M[ 4]*v[1]+M[ 8]*v[2]+M[12]*v[3];
    res[1]=M[1]*v[0]+M[ 5]*v[1]+M[ 9]*v[2]+M[13]*v[3];
    res[2]=M[2]*v[0]+M[ 6]*v[1]+M[10]*v[2]+M[14]*v[3];
    res[3]=M[3]*v[0]+M[ 7]*v[1]+M[11]*v[2]+M[15]*v[3];
    v[0]=res[0];// 把结果保存在V中
    v[1]=res[1];
    v[2]=res[2];
    v[3]=res[3];
}


MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_lightPos({ -4.0f, 0.0f, -9.0f, 1.0f}),
    m_lightAmb({ 0.2f, 0.2f, 0.2f, 1.0f}), m_lightDif({ 0.6f, 0.6f, 0.6f, 1.0f}),
    m_lightSpc({-0.2f, -0.2f, -0.2f, 1.0f}), m_matAmb({0.4f, 0.4f, 0.4f, 1.0f}),
    m_matDif({0.2f, 0.6f, 0.9f, 1.0f}), m_matSpc({0.0f, 0.0f, 0.0f, 1.0f}),
    m_matShn({0.0f}), m_objPos({-2.0f,-2.0f,-5.0f}), m_q(NULL), m_spherePos({-4.0f,-5.0f,-6.0f}),
    m_xrot(0.0f), m_yrot(0.0f), m_xspeed(0.0f), m_yspeed(0.0f)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
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
    gluPerspective(45.0f,(GLfloat)w/(GLfloat)h, 0.001f, 100.0f);

    glMatrixMode(GL_MODELVIEW);	//选择模型观察矩阵
    glLoadIdentity(); // 重置模型观察矩阵
}

void MyGLWidget::initializeGL()
{
    initGLObjects();// Function For Initializing Our Object(s)
    glShadeModel(GL_SMOOTH);// Enable Smooth Shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);// Black Background
    glClearDepth(1.0f);// Depth Buffer Setup
    glClearStencil(0);// Stencil Buffer Setup
    glEnable(GL_DEPTH_TEST);// Enables Depth Testing
    glDepthFunc(GL_LEQUAL);// The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);// Really Nice Perspective Calculations

    glLightfv(GL_LIGHT1, GL_POSITION, m_lightPos);// Set Light1 Position
    glLightfv(GL_LIGHT1, GL_AMBIENT, m_lightAmb);// Set Light1 Ambience
    glLightfv(GL_LIGHT1, GL_DIFFUSE, m_lightDif);// Set Light1 Diffuse
    glLightfv(GL_LIGHT1, GL_SPECULAR, m_lightSpc);// Set Light1 Specular
    glEnable(GL_LIGHT1);// Enable Light1
    glEnable(GL_LIGHTING);// Enable Lighting

    glMaterialfv(GL_FRONT, GL_AMBIENT, m_matAmb);// Set Material Ambience
    glMaterialfv(GL_FRONT, GL_DIFFUSE, m_matDif);// Set Material Diffuse
    glMaterialfv(GL_FRONT, GL_SPECULAR, m_matSpc);// Set Material Specular
    glMaterialfv(GL_FRONT, GL_SHININESS, m_matShn);// Set Material Shininess

    glCullFace(GL_BACK);// Set Culling Face To Back Face
    glEnable(GL_CULL_FACE);// Enable Culling
    glClearColor(0.1f, 1.0f, 0.5f, 1.0f);// Set Clear Color (Greenish Color)

    m_q = gluNewQuadric();// Initialize Quadratic
    gluQuadricNormals(m_q, GL_SMOOTH);// Enable Smooth Normal Generation
    gluQuadricTexture(m_q, GL_FALSE);// Disable Auto Texture Coords
}

//下一段包括了所有的绘图代码。任何您所想在屏幕上显示的东东都将在此段代码中出现。
//以后的每个教程中我都会在例程的此处增加新的代码。如果您对OpenGL已经有所了解的话，您可以在glLoadIdentity()调用之后，
//试着添加一些OpenGL代码来创建基本的形。
//如果您是OpenGL新手，等着我的下个教程。目前我们所作的全部就是将屏幕清除成我们前面所决定的颜色，清除深度缓存并且重置场景。
//我们仍没有绘制任何东东。
void MyGLWidget::paintGL()
{
    GLmatrix16f Minv;
    GLvector4f wlp, lp;
    // 清空缓存
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glLoadIdentity();// 设置灯光，并绘制球
    glTranslatef(0.0f, 0.0f, -20.0f);
    glLightfv(GL_LIGHT1, GL_POSITION, m_lightPos);
    glTranslatef(m_spherePos[0], m_spherePos[1], m_spherePos[2]);
    gluSphere(m_q, 1.5f, 32, 16);
    //下面我们计算灯光在物体坐标系中的位置
    glLoadIdentity();
    glRotatef(-m_yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(-m_xrot, 1.0f, 0.0f, 0.0f);
    glTranslatef(-m_objPos[0], -m_objPos[1], -m_objPos[2]);
    glGetFloatv(GL_MODELVIEW_MATRIX, Minv);// 计算从世界坐标系变化到物体坐标系中的坐标
    lp[0] = m_lightPos[0];// 保存灯光的位置
    lp[1] = m_lightPos[1];
    lp[2] = m_lightPos[2];
    lp[3] = m_lightPos[3];
    VMatMult(Minv, lp);// 计算最后灯光的位置
    glTranslatef(-m_objPos[0], -m_objPos[1], -m_objPos[2]);	// Move Negative On All Axis Based On ObjPos[] Values (X, Y, Z)
    glGetFloatv(GL_MODELVIEW_MATRIX,Minv);                  // Retrieve ModelView Matrix From Minv
    wlp[0] = 0.0f;                                          // World Local Coord X To 0
    wlp[1] = 0.0f;                                          // World Local Coord Y To 0
    wlp[2] = 0.0f;                                          // World Local Coord Z To 0
    wlp[3] = 1.0f;
    VMatMult(Minv, wlp);                                    // We Store The Position Of The World Origin Relative To The
                                                            // Local Coord. System In 'wlp' Array
    lp[0] += wlp[0];                                        // Adding These Two Gives Us The
    lp[1] += wlp[1];                                        // Position Of The Light Relative To
    lp[2] += wlp[2];                                        // The Local Coordinate System

    glColor4f(0.7f, 0.4f, 0.0f, 1.0f);                      // Set Color To An Orange
    //下面绘制房间，物体和它的阴影
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -20.0f);
    drawGLRoom();// 绘制房间
    glTranslatef(m_objPos[0], m_objPos[1], m_objPos[2]);
    glRotatef(m_xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(m_yrot, 0.0f, 1.0f, 0.0f);
    drawObject(m_obj);// 绘制物体
    castShadow(&m_obj, lp);// 绘制物体的阴影
    //下面的代码绘制一个黄色的球代表了灯光的位置
    glColor4f(0.7f, 0.4f, 0.0f, 1.0f);
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glTranslatef(lp[0], lp[1], lp[2]);
    gluSphere(m_q, 0.2f, 16, 8);
    glEnable(GL_LIGHTING);
    glDepthMask(GL_TRUE);
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
        case Qt::Key_Left:
        {
            m_yspeed -= 0.1f;
            break;
        }
        case Qt::Key_Right:
        {
            m_yspeed += 0.1f;
            break;
        }
        case Qt::Key_Up:
        {
            m_xspeed -= 0.1f;
            break;
        }
        case Qt::Key_Down:
        {
            m_xspeed += 0.1f;
            break;
        }
        case Qt::Key_L:
        {
            m_lightPos[0] += 0.5f;
            break;
        }
        case Qt::Key_J:
        {
            m_lightPos[0] -= 0.5f;
            break;
        }
        case Qt::Key_I:
        {
            m_lightPos[1] +=0.5f;
            break;
        }
        case Qt::Key_K:
        {
            m_lightPos[1] -=0.5f;
            break;
        }
        case Qt::Key_O:
        {
            m_lightPos[2] +=0.5f;
            break;
        }
        case Qt::Key_U:
        {
            m_lightPos[2] -=0.5f;
            break;
        }
        case Qt::Key_D:
        {
            m_spherePos[0] +=0.5f;
            break;
        }
        case Qt::Key_A:
        {
            m_spherePos[0] -=0.5f;
            break;
        }
        case Qt::Key_W:
        {
            m_spherePos[1] +=0.5f;
            break;
        }
        case Qt::Key_S:
        {
            m_spherePos[1] -=0.5f;
            break;
        }
        case Qt::Key_E:
        {
            m_spherePos[2] +=0.5f;
            break;
        }
        case Qt::Key_Q:
        {
            m_spherePos[2] -=0.5f;
            break;
        }
    }
    QGLWidget::keyPressEvent(event);
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    m_xrot += m_xspeed;// Increase xrot By xspeed
    m_yrot += m_yspeed;// Increase yrot By yspeed
    updateGL();
    QGLWidget::timerEvent(event);
}

//下面的代码用来读取模型，它的代码本身就解释了它的功能。它从文件中读取数据，并把顶点和索引存储在上面定义的结构中，并把所有的临近顶点初始化为-1，
//它代表这没有任何顶点与它相邻，我们将在以后计算它。
bool MyGLWidget::readObject(QString fileName, glObject* o)
{
    QFile file(fileName);
    QTextStream stream(&file);
    if(!file.open(QIODevice::ReadOnly))
    {
      return false;
    }
    //读取顶点
    stream >> o->nPoints;
    stream.readLine();
    for (int i=1;i <= o->nPoints;i++)
    {
      stream >> o->points[i].x;
      stream >> o->points[i].y;
      stream >> o->points[i].z;
      stream.readLine();
    }
    //读取三角形面
    stream >> o->nPlanes;
    stream.readLine();
    for (int i=0; i< o->nPlanes;i++)
    {
        stream >> o->planes[i].p[0];
        stream >> o->planes[i].p[1];
        stream >> o->planes[i].p[2];
        //读取每个顶点的法线
        stream >> o->planes[i].normals[0].x;
        stream >> o->planes[i].normals[0].y;
        stream >> o->planes[i].normals[0].z;
        stream >> o->planes[i].normals[1].x;
        stream >> o->planes[i].normals[1].y;
        stream >> o->planes[i].normals[1].z;
        stream >> o->planes[i].normals[2].x;
        stream >> o->planes[i].normals[2].y;
        stream >> o->planes[i].normals[2].z;
        stream.readLine();
    }
}

//现在从setConnectivity函数开始,事情变得越来越复杂了,这个函数用来查找每个面的相邻的顶点,下面是它的伪代码:
//对于模型中的每一个面A
//对于面A中的每一条边
//如果我们不只到这条边相邻的顶点
//那么对于模型中除了面A外的每一个面B
//对于面B中的每一条边
//如果面A的边和面B的边是同一条边,那么这两个面相邻
//设置面A和面B的相邻属性
//下面的代码完成上面伪代码中最后两行的内容,你先获得每个面中边的两个顶点,然后检测他们是否相邻,如果是则设置各自的相邻顶点信息
//int vertA1 = pFaceA->vertexIndices[edgeA];
//int vertA2 = pFaceA->vertexIndices[( edgeA+1 )%3];
//int vertB1 = pFaceB->vertexIndices[edgeB];
//int vertB2 = pFaceB->vertexIndices[( edgeB+1 )%3];
//测试他们是否为同一边,如果是则设置相应的相邻顶点信息
//if (( vertA1 == vertB1 && vertA2 == vertB2 ) || ( vertA1 == vertB2 && vertA2 == vertB1 ))
//{
//      pFaceA->neighbourIndices[edgeA] = faceB;
//		pFaceB->neighbourIndices[edgeB] = faceA;
//		edgeFound = true;
//		break;
//}
//	完整的SetConnectivity函数的代码如下
// 设置相邻顶点信息
void MyGLWidget::setConnectivity(glObject *o)
{
    unsigned int p1i, p2i, p1j, p2j;
    unsigned int P1i, P2i, P1j, P2j;
    //对于模型中的每一个面A
    for(int i=0;i < o->nPlanes-1;i++)
    {
        //对于除了此面的其它的面B
        for(int j=i+1;j < o->nPlanes;j++)
        {
            //对于面A中的每一个相邻的顶点
            for(int ki=0;ki<3;ki++)
            {
                //如果这个相邻的顶点没有被设置
                if(!o->planes[i].neigh[ki])
                {
                    for(int kj=0;kj<3;kj++)
                    {
                        p1i=ki;
                        p1j=kj;
                        p2i=(ki+1)%3;
                        p2j=(kj+1)%3;
                        p1i=o->planes[i].p[p1i];
                        p2i=o->planes[i].p[p2i];
                        p1j=o->planes[j].p[p1j];
                        p2j=o->planes[j].p[p2j];
                        //如果面A的边P1i->P1j和面B的边P2i->P2j为同一条边，则又下面的公式的P1i=P1j，并且P2i=P2j
                        P1i=((p1i+p2i)-abs(p1i-p2i))/2;
                        P2i=((p1i+p2i)+abs(p1i-p2i))/2;
                        P1j=((p1j+p2j)-abs(p1j-p2j))/2;
                        P2j=((p1j+p2j)+abs(p1j-p2j))/2;
                        //记录与这个边相邻的面的索引
                        if((P1i==P1j) && (P2i==P2j))
                        {
                            o->planes[i].neigh[ki] = j+1;
                            o->planes[j].neigh[kj] = i+1;
                        }
                    }
                }
            }
        }
    }
}

//下面的函数用来绘制模型
// 绘制模型，像以前一样它绘制组成模型的三角形
void MyGLWidget::drawObject(glObject o)
{
    glBegin(GL_TRIANGLES);
    for (int i=0; i<o.nPlanes; i++)
    {
        for (int j=0; j<3; j++)
        {
            glNormal3f(o.planes[i].normals[j].x,
                       o.planes[i].normals[j].y,
                       o.planes[i].normals[j].z);
            glVertex3f(o.points[o.planes[i].p[j]].x,
                       o.points[o.planes[i].p[j]].y,
                       o.points[o.planes[i].p[j]].z);
        }
    }
    glEnd();
}

//下面的函数用来计算平面的方程参数
void MyGLWidget::calculatePlane(glObject o, sPlane *plane)
{
    // 获得平面的三个顶点
    sPoint v[4];
    for (int i=0;i<3;i++)
    {
        v[i+1].x = o.points[plane->p[i]].x;
        v[i+1].y = o.points[plane->p[i]].y;
        v[i+1].z = o.points[plane->p[i]].z;
    }
    plane->PlaneEq.a = v[1].y*(v[2].z-v[3].z) + v[2].y*(v[3].z-v[1].z) + v[3].y*(v[1].z-v[2].z);
    plane->PlaneEq.b = v[1].z*(v[2].x-v[3].x) + v[2].z*(v[3].x-v[1].x) + v[3].z*(v[1].x-v[2].x);
    plane->PlaneEq.c = v[1].x*(v[2].y-v[3].y) + v[2].x*(v[3].y-v[1].y) + v[3].x*(v[1].y-v[2].y);
    plane->PlaneEq.d =-( v[1].x*(v[2].y*v[3].z - v[3].y*v[2].z) +
                      v[2].x*(v[3].y*v[1].z - v[1].y*v[3].z) +
                      v[3].x*(v[1].y*v[2].z - v[2].y*v[1].z) );
}

//你还可以呼吸么?好的,我们继续:) 接下来你将学习如何去投影,castShadow函数几乎用到了所有OpenGL的功能,完成这个函数后,
//把它传递到doShadowPass函数来通过两个渲染通道绘制出阴影.
//首先,我们看看哪些面面对着灯光,我们可以通过灯光位置和平面方程计算出.如果灯光到平面的位置大于0,则位于灯光的上方,
//否则位于灯光的下方(如果有什么问题,翻一下你高中的解析几何)
void MyGLWidget::castShadow(glObject *o, float *lp)
{
    unsigned int	p1, p2;
    sPoint			v1, v2;
    float			side;

    // 设置哪些面在灯光的前面
    for (int i=0;i<o->nPlanes;i++)
    {
        side =	o->planes[i].PlaneEq.a*lp[0]+
                o->planes[i].PlaneEq.b*lp[1]+
                o->planes[i].PlaneEq.c*lp[2]+
                o->planes[i].PlaneEq.d*lp[3];
        if (side >0)
        {
            o->planes[i].visible = true;
        }
        else
        {
            o->planes[i].visible = false;
        }
    }

    //下面设置必要的状态来渲染阴影.
    //首先,禁用灯光和绘制颜色,因为我们不计算光照,这样可以节约计算量.
    //接着,设置深度缓存,深度测试还是需要的,但我们不希望我们的阴影体向实体一样具有深度,所以关闭深度缓存.
    //最后我们启用蒙板缓存,让阴影体的位置在蒙板中被设置为1.
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_STENCIL_TEST);
    glColorMask(0, 0, 0, 0);
    glStencilFunc(GL_ALWAYS, 1, 0xffffffff);

    //现在到了阴影被实际渲染得地方了,我们使用了下面提到的doShadowPass函数,它用来绘制阴影体的边界面.
    //我们通过两个步骤来绘制阴影体,首先使用前向面增加阴影体在蒙板缓存中的值,接着使用后向面减少阴影体在蒙板缓存中的值.
    //如果是逆时针（即面向视点）的多边形，通过了蒙板和深度测试，则把蒙板的值增加1
    glFrontFace(GL_CCW);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    //下面的部分我们绘制构成阴影体边界的四边形,当我们循环所有的三角形面的时候,我们检测它是否是边界边,
    //如果是我们绘制从灯光到这个边界边的射线,并衍生它用来构成四边形.
    //这里要用一个蛮力,我们检测物体模型中每一个三角形面,找出其边界并连接灯光到边界的直线,
    //把直线延长出一定的距离,构成阴影体.
    //下面的代码完成这些功能,它看起来并没有想象的复杂.
    //对模型中的每一个面
    for (int i=0; i<o->nPlanes;i++)
    {
        //如果面在灯光的前面
        if (o->planes[i].visible)
        {
            //对于被灯光照射的面的每一个相邻的面
            for (int j=0;j<3;j++)
            {
                int k = o->planes[i].neigh[j];
                //如果面不存在，或不被灯光照射，那么这个边是边界
                if ((!k) || (!o->planes[k-1].visible))
                {
                    // 获得面的两个顶点
                    p1 = o->planes[i].p[j];
                    int jj = (j+1)%3;
                    p2 = o->planes[i].p[jj];

                    //计算边的顶点到灯光的方向，并放大100倍
                    v1.x = (o->points[p1].x - lp[0])*100;
                    v1.y = (o->points[p1].y - lp[1])*100;
                    v1.z = (o->points[p1].z - lp[2])*100;

                    v2.x = (o->points[p2].x - lp[0])*100;
                    v2.y = (o->points[p2].y - lp[1])*100;
                    v2.z = (o->points[p2].z - lp[2])*100;

                    //绘制构成阴影体边界的面
                    glBegin(GL_TRIANGLE_STRIP);
                        glVertex3f(o->points[p1].x,
                                   o->points[p1].y,
                                   o->points[p1].z);
                        glVertex3f(o->points[p1].x + v1.x,
                                   o->points[p1].y + v1.y,
                                   o->points[p1].z + v1.z);
                        glVertex3f(o->points[p2].x,
                                   o->points[p2].y,
                                   o->points[p2].z);
                        glVertex3f(o->points[p2].x + v2.x,
                                   o->points[p2].y + v2.y,
                                   o->points[p2].z + v2.z);
                    glEnd();
                }
            }
        }
    }
    // 如果是顺时针（即背向视点）的多边形，通过了蒙板和深度测试，则把蒙板的值减少1
    glFrontFace(GL_CW);
    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
    //下面的部分我们绘制构成阴影体边界的四边形,当我们循环所有的三角形面的时候,我们检测它是否是边界边,
    //如果是我们绘制从灯光到这个边界边的射线,并衍生它用来构成四边形.
    //这里要用一个蛮力,我们检测物体模型中每一个三角形面,找出其边界并连接灯光到边界的直线,
    //把直线延长出一定的距离,构成阴影体.
    //下面的代码完成这些功能,它看起来并没有想象的复杂.
    //对模型中的每一个面
    for (int i=0; i<o->nPlanes;i++)
    {
        //如果面在灯光的前面
        if (o->planes[i].visible)
        {
            //对于被灯光照射的面的每一个相邻的面
            for (int j=0;j<3;j++)
            {
                int k = o->planes[i].neigh[j];
                //如果面不存在，或不被灯光照射，那么这个边是边界
                if ((!k) || (!o->planes[k-1].visible))
                {
                    // 获得面的两个顶点
                    p1 = o->planes[i].p[j];
                    int jj = (j+1)%3;
                    p2 = o->planes[i].p[jj];

                    //计算边的顶点到灯光的方向，并放大100倍
                    v1.x = (o->points[p1].x - lp[0])*100;
                    v1.y = (o->points[p1].y - lp[1])*100;
                    v1.z = (o->points[p1].z - lp[2])*100;

                    v2.x = (o->points[p2].x - lp[0])*100;
                    v2.y = (o->points[p2].y - lp[1])*100;
                    v2.z = (o->points[p2].z - lp[2])*100;

                    //绘制构成阴影体边界的面
                    glBegin(GL_TRIANGLE_STRIP);
                        glVertex3f(o->points[p1].x,
                                   o->points[p1].y,
                                   o->points[p1].z);
                        glVertex3f(o->points[p1].x + v1.x,
                                   o->points[p1].y + v1.y,
                                   o->points[p1].z + v1.z);
                        glVertex3f(o->points[p2].x,
                                   o->points[p2].y,
                                   o->points[p2].z);
                        glVertex3f(o->points[p2].x + v2.x,
                                   o->points[p2].y + v2.y,
                                   o->points[p2].z + v2.z);
                    glEnd();
                }
            }
        }
    }

    glFrontFace(GL_CCW);
    glColorMask(1, 1, 1, 1);

    //draw a shadowing rectangle covering the entire screen
    glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glStencilFunc(GL_NOTEQUAL, 0, 0xffffffff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glPushMatrix();
    glLoadIdentity();
    glBegin(GL_TRIANGLE_STRIP);
        glVertex3f(-0.1f, 0.1f,-0.10f);
        glVertex3f(-0.1f,-0.1f,-0.10f);
        glVertex3f( 0.1f, 0.1f,-0.10f);
        glVertex3f( 0.1f,-0.1f,-0.10f);
    glEnd();
    glPopMatrix();
    glDisable(GL_BLEND);

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
    glDisable(GL_STENCIL_TEST);
    glShadeModel(GL_SMOOTH);
}

void MyGLWidget::drawGLRoom()// 绘制房间(盒装)
{
    glBegin(GL_QUADS);// 绘制四边形
        // 地面
        glNormal3f(0.0f, 1.0f, 0.0f);// 法线向上
        glVertex3f(-10.0f,-10.0f,-20.0f);
        glVertex3f(-10.0f,-10.0f,20.0f);
        glVertex3f(10.0f,-10.0f,20.0f);
        glVertex3f(10.0f,-10.0f,-20.0f);
        // 天花板
        glNormal3f(0.0f,-1.0f,0.0f);// 法线向下
        glVertex3f(-10.0f,10.0f,20.0f);
        glVertex3f(-10.0f,10.0f,-20.0f);
        glVertex3f(10.0f,10.0f,-20.0f);
        glVertex3f(10.0f,10.0f,20.0f);
        // 前面
        glNormal3f(0.0f, 0.0f,1.0f);// 法线向后
        glVertex3f(-10.0f,10.0f,-20.0f);
        glVertex3f(-10.0f,-10.0f,-20.0f);
        glVertex3f(10.0f,-10.0f,-20.0f);
        glVertex3f(10.0f,10.0f,-20.0f);
        // 后面
        glNormal3f(0.0f,0.0f,-1.0f);// 法线向前
        glVertex3f(10.0f,10.0f,20.0f);
        glVertex3f(10.0f,-10.0f,20.0f);
        glVertex3f(-10.0f,-10.0f,20.0f);
        glVertex3f(-10.0f,10.0f,20.0f);
        // 左面
        glNormal3f(1.0f,0.0f,0.0f);	// 法线向右
        glVertex3f(-10.0f,10.0f,20.0f);
        glVertex3f(-10.0f,-10.0f,20.0f);
        glVertex3f(-10.0f,-10.0f,-20.0f);
        glVertex3f(-10.0f,10.0f,-20.0f);
        // 右面
        glNormal3f(-1.0f,0.0f,0.0f);// 法线向左
        glVertex3f(10.0f,10.0f,-20.0f);
        glVertex3f(10.0f,-10.0f,-20.0f);
        glVertex3f(10.0f,-10.0f,20.0f);
        glVertex3f(10.0f,10.0f,20.0f);
    glEnd();// 结束绘制
}

//下面的函数用来初始化模型对象
bool MyGLWidget::initGLObjects()// 初始化模型对象
{
    if (readObject(":/data/Object2.txt", &m_obj))// 读取模型数据
    {
        return false;// 返回失败
    }
    setConnectivity(&m_obj);// 设置相邻顶点的信息
    for (int i=0;i < m_obj.nPlanes;i++)// 计算每个面的平面参数
    {
        calculatePlane(m_obj, &(m_obj.planes[i]));
    }
    return true;
}
//其他的函数我们不做过多解释了,这会分散你的注意力,好好享受阴影带给你的快感吧.
//下面还有一些说明:
//球体不会产生阴影,因为我们没有设置其投影.
//如果你发现程序很慢,买块好的显卡吧.
//最后我希望你喜欢它,如果有什么好的建议,请告诉我.
