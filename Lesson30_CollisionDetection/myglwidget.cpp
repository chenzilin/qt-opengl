#include "myglwidget.h"

//碰撞检测:
//这是一课激动的教程，你也许等待它多时了。你将学会碰撞剪裁，物理模拟太多的东西，慢慢期待吧。
//碰撞检测和物理模拟(作者:Dimitrios Christopoulos (christop@fhw.gr))
//碰撞检测
//这是一个我遇到的最困难的题目,因为它没有一个简单的解决办法.对于每一个程序都有一种检测碰撞的方法.当然这里有一种蛮力,
//它适用于各种不同的应用,当它非常的费时.
//我们将讲述一种算法,它非常的快,简单并易于扩展.下面我们来看看这个算法包含的内容:
//1) 碰撞检测
//移动的球-平面
//移动的球-圆柱
//移动的球-移动的球
//2) 基于物理的建模
//碰撞表示
//应用重力加速度
//3) 特殊效果
//爆炸的表示，利用互交叉的公告板形式
//声音使用Windows声音库
//4) 关于代码
//代码被分为以下5个部分
//Lesson30.cpp	 	: 主程序代码l
//Tmatrix.cpp,	Tmatrix.h	: 矩阵
//Tray.cpp,	Tray.h	: 射线
//Tvector.cpp,	Tvector.h	: 向量
//1) 碰撞检测
//我们使用射线来完成相关的算法，它的定义为:
//射线上的点 = 射线的原点+ t * 射线的方向
//t 用来描述它距离原点的位置，它的范围是[0, 无限远).
//现在我们可以使用射线来计算它和平面以及圆柱的交点了。
//射线和平面的碰撞检测：
//平面被描述为：
//Xn dot X = d
//Xn 是平面的法线.
//X 是平面上的一个点.
//d 是平面到原点的距离.
//现在我们得到射线和平面的两个方程:
//PointOnRay = Raystart + t * Raydirection
//Xn dot X = d
//如果他们相交，则上诉方程组有解，如下所示：
//Xn dot PointOnRay = d
//(Xn dot Raystart) + t * (Xn dot Raydirection) = d
//解得 t:
//t = (d - Xn dot Raystart) / (Xn dot Raydirection)
//t代表原点到与平面相交点的参数,把t带回原方程我们会得到与平面的碰撞点.
//如果Xn*Raydirection=0。则说明它与平面平行，则将不产生碰撞。如果t为负值，则说明交点在射线的相反方向，也不会产生碰撞。

#define EPSILON 1.0e-8
#define ZERO EPSILON

const static GLfloat spec[]={1.0, 1.0 ,1.0 ,1.0};//sets specular highlight of balls
const static  GLfloat posl[]={0,400,0,1};//position of ligth source
const static  GLfloat amb[]={0.2f, 0.2f, 0.2f ,1.0f};//global ambient
const static  GLfloat amb2[]={0.3f, 0.3f, 0.3f ,1.0f};//ambient of lightsource
const static  TVector dir(0,0,-10);//initial direction of camera
const static TVector veloc(0.5,-0.1,0.5);//initial velocity of balls
const static TVector accel(0,-0.05,0);//acceleration ie. gravity of balls

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_pos(0, -50, 1000),
    m_hook_toball1(0), m_sounds(1), m_camera_rotation(0), m_time(0.6),
    m_cylinder_obj(NULL)
{
    initVars();
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(4, &m_texture[0]);
    glDeleteLists(1, m_dlist);
    gluDeleteQuadric(m_cylinder_obj);
}

void MyGLWidget::resizeGL(int w, int h)
{
    if (h==0)// Prevent A Divide By Zero By
    {
        h=1;// Making Height Equal One
    }
    glViewport(0,0,w,h);// Reset The Current Viewport
    glMatrixMode(GL_PROJECTION);// Select The Projection Matrix
    glLoadIdentity();// Reset The Projection Matrix
    // Calculate The Aspect Ratio Of The Window
    gluPerspective(50.0f,(GLfloat)w/(GLfloat)h,10.f,1700.0f);
    glMatrixMode(GL_MODELVIEW);// Select The Modelview Matrix
    glLoadIdentity();// Reset The Modelview Matrix
}

// All Setup For OpenGL Goes Here
void MyGLWidget::initializeGL()
{
    float df=100.0;
    glClearDepth(1.0f);// Depth Buffer Setup
    glEnable(GL_DEPTH_TEST);// Enables Depth Testing
    glDepthFunc(GL_LEQUAL);// The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);// Really Nice Perspective Calculations

    glClearColor(0,0,0,0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glShadeModel(GL_SMOOTH);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glMaterialfv(GL_FRONT,GL_SPECULAR,spec);
    glMaterialfv(GL_FRONT,GL_SHININESS,&df);

    glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT0,GL_POSITION,posl);
    glLightfv(GL_LIGHT0,GL_AMBIENT,amb2);
    glEnable(GL_LIGHT0);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,amb);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glEnable(GL_TEXTURE_2D);
    loadGLTexture();

    //Construct billboarded explosion primitive as display list
    //4 quads at right angles to each other
    glNewList(m_dlist=glGenLists(1), GL_COMPILE);
        glBegin(GL_QUADS);
            glRotatef(-45,0,1,0);
            glNormal3f(0,0,1);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-50,-40,0);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(50,-40,0);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(50,40,0);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-50,40,0);
            glNormal3f(0,0,-1);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-50,40,0);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(50,40,0);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(50,-40,0);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(-50,-40,0);

            glNormal3f(1,0,0);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(0,-40,50);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(0,-40,-50);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(0,40,-50);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(0,40,50);
            glNormal3f(-1,0,0);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(0,40,50);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(0,40,-50);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(0,-40,-50);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(0,-40,50);
        glEnd();
    glEndList();
}

// Here's Where We Do All The Drawing
void MyGLWidget::paintGL()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //set camera in hookmode
    if (m_hook_toball1)
    {
        TVector unit_followvector=m_arrayVel[0];
        unit_followvector.unit();
        gluLookAt(m_arrayPos[0].X()+250, m_arrayPos[0].Y()+250 , m_arrayPos[0].Z(),
                m_arrayPos[0].X()+m_arrayPos[0].X(), m_arrayPos[0].Y()+m_arrayPos[0].Y(),
                m_arrayPos[0].Z()+m_arrayPos[0].Z() ,0,1,0);
    }
    else
    {
        gluLookAt(m_pos.X(),m_pos.Y(),m_pos.Z(), m_pos.X()+dir.X(),
                  m_pos.Y()+dir.Y(),m_pos.Z()+dir.Z(), 0,1.0,0.0);
    }

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glRotatef(m_camera_rotation,0,1,0);

    //render balls
    for (int i=0;i<m_nrOfBalls;i++)
    {
        switch(i)
        {
        case 1: glColor3f(1.0f,1.0f,1.0f);
                   break;
        case 2: glColor3f(1.0f,1.0f,0.0f);
                   break;
        case 3: glColor3f(0.0f,1.0f,1.0f);
                   break;
        case 4: glColor3f(0.0f,1.0f,0.0f);
                   break;
        case 5: glColor3f(0.0f,0.0f,1.0f);
                   break;
        case 6: glColor3f(0.65f,0.2f,0.3f);
                   break;
        case 7: glColor3f(1.0f,0.0f,1.0f);
                   break;
        case 8: glColor3f(0.0f,0.7f,0.4f);
                   break;
        default: glColor3f(1.0f,0,0);
        }
        glPushMatrix();
            glTranslated(m_arrayPos[i].X(),m_arrayPos[i].Y(),m_arrayPos[i].Z());
            gluSphere(m_cylinder_obj,20,20,20);
        glPopMatrix();
    }

    glEnable(GL_TEXTURE_2D);

    //render walls(planes) with texture
    glBindTexture(GL_TEXTURE_2D, m_texture[3]);
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(320,320,320);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(320,-320,320);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-320,-320,320);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-320,320,320);

        glTexCoord2f(1.0f, 0.0f); glVertex3f(-320,320,-320);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-320,-320,-320);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(320,-320,-320);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(320,320,-320);

        glTexCoord2f(1.0f, 0.0f); glVertex3f(320,320,-320);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(320,-320,-320);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(320,-320,320);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(320,320,320);

        glTexCoord2f(1.0f, 0.0f); glVertex3f(-320,320,320);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-320,-320,320);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-320,-320,-320);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-320,320,-320);
    glEnd();

    //render floor (plane) with colours
    glBindTexture(GL_TEXTURE_2D, m_texture[2]);
    glBegin(GL_QUADS);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-320,-320,320);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(320,-320,320);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(320,-320,-320);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-320,-320,-320);
    glEnd();

    //render columns(cylinders)
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);/* choose the texture to use.*/
    glColor3f(0.5,0.5,0.5);
    glPushMatrix();
        glRotatef(90, 1,0,0);
        glTranslatef(0,0,-500);
        gluCylinder(m_cylinder_obj, 60, 60, 1000, 20, 2);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(200,-300,-500);
        gluCylinder(m_cylinder_obj, 60, 60, 1000, 20, 2);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(-200,0,0);
        glRotatef(135, 1,0,0);
        glTranslatef(0,0,-500);
        gluCylinder(m_cylinder_obj, 30, 30, 1000, 20, 2);
    glPopMatrix();

    //爆炸
    //最好的表示爆炸效果的就是使用两个互相垂直的平面，并使用alpha混合在窗口中显示它们。接着让alpha变为0，
    //设定爆炸效果不可见。
    glEnable(GL_BLEND);// 使用混合
    glDepthMask(GL_FALSE);// 禁用深度缓存
    glBindTexture(GL_TEXTURE_2D, m_texture[1]);// 设置纹理
    for(int i=0; i<20; i++)// 渲染20个爆炸效果
    {
        if(m_explosionArray[i]._Alpha>=0)
        {
           glPushMatrix();
               m_explosionArray[i]._Alpha-=0.01f;// 设置alpha
               m_explosionArray[i]._Scale+=0.03f;// 设置缩放
               // 设置颜色
               glColor4f(1,1,0,m_explosionArray[i]._Alpha);
               glScalef(m_explosionArray[i]._Scale,m_explosionArray[i]._Scale,m_explosionArray[i]._Scale);
               // 设置位置
               glTranslatef((float)m_explosionArray[i]._Position.X()/m_explosionArray[i]._Scale,
                            (float)m_explosionArray[i]._Position.Y()/m_explosionArray[i]._Scale,
                            (float)m_explosionArray[i]._Position.Z()/m_explosionArray[i]._Scale);
               glCallList(m_dlist);// 调用显示列表绘制爆炸效果
           glPopMatrix();
        }
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void MyGLWidget::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Escape:
        {
            qApp->exit();
            break;
        }
        case Qt::Key_Up:
        {
            m_pos+=TVector(0,0,-10);
            break;
        }
        case Qt::Key_Down:
        {
            m_pos+=TVector(0,0,10);
            break;
        }
        case Qt::Key_Left:
        {
            m_camera_rotation+=10;
            break;
        }
        case Qt::Key_Right:
        {
            m_camera_rotation-=10;
            break;
        }
        case Qt::Key_PageUp:
        {
            m_time+=0.1;
            break;
        }
        case Qt::Key_PageDown:
        {
            m_time-=0.1;
            break;
        }
        case Qt::Key_F1:
        {
            m_hook_toball1^=1;
            m_camera_rotation=0;
            break;
        }
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
        case Qt::Key_F3:
        {
            m_sounds^=1;
            break;
        }
    }
    QGLWidget::keyPressEvent(event);
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    idle();
    updateGL();
    QGLWidget::timerEvent(event);
}

//判断球和球是否相交，是则返回1，否则返回0
int MyGLWidget::findBallCol(TVector& point, double& TimePoint, double Time2, int& BallNr1, int& BallNr2)
{
    TVector RelativeV;
    TRay rays;
    double MyTime=0.0, Add=Time2/150.0, Timedummy=10000;
    TVector posi;

    //判断球和球是否相交
    for (int i=0;i<m_nrOfBalls-1;i++)
    {
        for (int j=i+1;j<m_nrOfBalls;j++)
        {
            RelativeV=m_arrayVel[i]-m_arrayVel[j];
            rays=TRay(m_oldPos[i],TVector::unit(RelativeV));
            MyTime=0.0;
            if ( (rays.dist(m_oldPos[j])) > 40) continue;
            while (MyTime<Time2)
            {
               MyTime+=Add;
               posi=m_oldPos[i]+RelativeV*MyTime;
               if (posi.dist(m_oldPos[j])<=40)
               {
                   point=posi;
                   if (Timedummy>(MyTime-Add))
                   {
                       Timedummy=MyTime-Add;
                   }
                   BallNr1=i;
                   BallNr2=j;
                   break;
               }
            }
        }
    }

    if (Timedummy!=10000)
    {
        TimePoint=Timedummy;
        return 1;
    }
    return 0;
}

//While (Timestep!=0)
//{
//	对每一个球
//	{
//		计算最近的与平面碰撞的位置;
//		计算最近的与圆柱碰撞的位置;
//		如果碰撞发生，则保存并替换最近的碰撞点;
//	}
//	检测各个球之间的碰撞;
//	如果碰撞发生，则保存并替换最近的碰撞点;
//	If (碰撞发生)
//	{
//		移动所有的球道碰撞点的时间;
//		(We already have computed the point, normal and collision time.)
//		计算碰撞后的效果;
//		Timestep-=CollisonTime;
//	}
//	else
//		移动所有的球体一步
//}
//模拟函数，计算碰撞检测和物理模拟
void MyGLWidget::idle()
{
    double rt,rt2,rt4,lamda=10000;
    TVector norm,uveloc;
    TVector normal,point;
    double RestTime,BallTime;
    TVector Pos2;
    int BallNr=0,BallColNr1,BallColNr2;
    TVector Nc;
    //如果没有锁定到球上，旋转摄像机
    if (!m_hook_toball1)
    {
        m_camera_rotation+=0.1f;
        if (m_camera_rotation>360)
        {
            m_camera_rotation=0;
        }
    }

    RestTime=m_time;
    lamda=1000;

    //计算重力加速度
    for (int j=0;j<m_nrOfBalls;j++)
    {
        m_arrayVel[j]+=accel*RestTime;
    }

    //如果在一步的模拟时间内(如果来不及计算，则跳过几步)
    while (RestTime>ZERO)
    {
        lamda=10000;//initialize to very large value
        //对于每个球，找到它们最近的碰撞点
        for (int i=0;i<m_nrOfBalls;i++)
        {
            //计算新的位置和移动的距离
            m_oldPos[i]=m_arrayPos[i];
            TVector::unit(m_arrayVel[i],uveloc);
            m_arrayPos[i]=m_arrayPos[i]+m_arrayVel[i]*RestTime;
            rt2=m_oldPos[i].dist(m_arrayPos[i]);

            //测试是否和墙面碰撞
            if (testIntersionPlane(m_pl1,m_oldPos[i],uveloc,rt,norm))
            {
                //计算碰撞的时间
                rt4=rt*RestTime/rt2;
                //如果小于当前保存的碰撞时间，则更新它
                if (rt4<=lamda)
                {
                  if (rt4<=RestTime+ZERO)
                  {
                      if (! ((rt<=ZERO)&&(uveloc.dot(norm)>ZERO)) )
                      {
                          normal=norm;
                          point=m_oldPos[i]+uveloc*rt;
                          lamda=rt4;
                          BallNr=i;
                      }
                  }
                }
            }

            if (testIntersionPlane(m_pl2,m_oldPos[i],uveloc,rt,norm))
            {
                rt4=rt*RestTime/rt2;
                if (rt4<=lamda)
                {
                  if (rt4<=RestTime+ZERO)
                  {
                      if (! ((rt<=ZERO)&&(uveloc.dot(norm)>ZERO)) )
                      {
                          normal=norm;
                          point=m_oldPos[i]+uveloc*rt;
                          lamda=rt4;
                          BallNr=i;
                      }
                  }
                }
            }

            if (testIntersionPlane(m_pl3,m_oldPos[i],uveloc,rt,norm))
            {
                rt4=rt*RestTime/rt2;
                if (rt4<=lamda)
                {
                  if (rt4<=RestTime+ZERO)
                  {
                      if (! ((rt<=ZERO)&&(uveloc.dot(norm)>ZERO)) )
                      {
                          normal=norm;
                          point=m_oldPos[i]+uveloc*rt;
                          lamda=rt4;
                          BallNr=i;
                      }
                  }
                }
            }

            if (testIntersionPlane(m_pl4,m_oldPos[i],uveloc,rt,norm))
            {
                rt4=rt*RestTime/rt2;
                if (rt4<=lamda)
                {
                  if (rt4<=RestTime+ZERO)
                  {
                      if (! ((rt<=ZERO)&&(uveloc.dot(norm)>ZERO)) )
                      {
                          normal=norm;
                          point=m_oldPos[i]+uveloc*rt;
                          lamda=rt4;
                          BallNr=i;
                      }
                  }
                }
            }

            if (testIntersionPlane(m_pl5,m_oldPos[i],uveloc,rt,norm))
            {
                rt4=rt*RestTime/rt2;
                if (rt4<=lamda)
                {
                  if (rt4<=RestTime+ZERO)
                  {
                      if (! ((rt<=ZERO)&&(uveloc.dot(norm)>ZERO)) )
                      {
                          normal=norm;
                          point=m_oldPos[i]+uveloc*rt;
                          lamda=rt4;
                          BallNr=i;
                      }
                  }
                }
            }

            //测试是否与三个圆柱相碰
            if (testIntersionCylinder(m_cyl1,m_oldPos[i],uveloc,rt,norm,Nc))
            {
                rt4=rt*RestTime/rt2;
                if (rt4<=lamda)
                {
                  if (rt4<=RestTime+ZERO)
                  {
                      if (! ((rt<=ZERO)&&(uveloc.dot(norm)>ZERO)) )
                      {
                          normal=norm;
                          point=Nc;
                          lamda=rt4;
                          BallNr=i;
                      }
                  }
                }
            }

            if (testIntersionCylinder(m_cyl2,m_oldPos[i],uveloc,rt,norm,Nc))
            {
                rt4=rt*RestTime/rt2;
                if (rt4<=lamda)
                {
                  if (rt4<=RestTime+ZERO)
                  {
                      if (! ((rt<=ZERO)&&(uveloc.dot(norm)>ZERO)) )
                      {
                          normal=norm;
                          point=Nc;
                          lamda=rt4;
                          BallNr=i;
                      }
                  }
                }
            }

            if (testIntersionCylinder(m_cyl3,m_oldPos[i],uveloc,rt,norm,Nc))
            {
                rt4=rt*RestTime/rt2;
                if (rt4<=lamda)
                {
                  if (rt4<=RestTime+ZERO)
                  {
                      if (! ((rt<=ZERO)&&(uveloc.dot(norm)>ZERO)) )
                      {
                          normal=norm;
                          point=Nc;
                          lamda=rt4;
                          BallNr=i;
                      }
                  }
                }
            }
       }

        //球体之间的碰撞
        //计算每个球之间的碰撞，如果碰撞时间小于与上面的碰撞，则替换它们
       if (findBallCol(Pos2,BallTime,RestTime,BallColNr1,BallColNr2))
       {
            if (m_sounds)
            {
               QSound::play(":/voice/Explode.wav");
            }
            if ( (lamda==10000) || (lamda>BallTime) )
            {
                RestTime=RestTime-BallTime;
                TVector pb1,pb2,xaxis,U1x,U1y,U2x,U2y,V1x,V1y,V2x,V2y;
                double a,b;

                pb1=m_oldPos[BallColNr1]+m_arrayVel[BallColNr1]*BallTime;// 球1的位置
                pb2=m_oldPos[BallColNr2]+m_arrayVel[BallColNr2]*BallTime;// 球2的位置
                xaxis=(pb2-pb1).unit();// X-Axis轴
                a=xaxis.dot(m_arrayVel[BallColNr1]);// X_Axis投影系数
                U1x=xaxis*a;// 计算在X_Axis轴上的速度
                U1y=m_arrayVel[BallColNr1]-U1x;// 计算在垂直轴上的速度

                xaxis=(pb1-pb2).unit();
                b=xaxis.dot(m_arrayVel[BallColNr2]);
                U2x=xaxis*b;
                U2y=m_arrayVel[BallColNr2]-U2x;
                V1x=(U1x+U2x-(U1x-U2x))*0.5;// 计算新的速度
                V2x=(U1x+U2x-(U2x-U1x))*0.5;
                V1y=U1y;
                V2y=U2y;
                for (int j=0;j<m_nrOfBalls;j++)// 更新所有球的位置
                {
                    m_arrayPos[j]=m_oldPos[j]+m_arrayVel[j]*BallTime;
                }
                m_arrayVel[BallColNr1]=V1x+V1y;// 设置新的速度
                m_arrayVel[BallColNr2]=V2x+V2y;

                //Update explosion array
                for(int j=0;j<20;j++)
                {
                    if (m_explosionArray[j]._Alpha<=0)
                    {
                      m_explosionArray[j]._Alpha=1;
                      m_explosionArray[j]._Position=m_arrayPos[BallColNr1];
                      m_explosionArray[j]._Scale=1;
                      break;
                    }
                }
                continue;
            }
       }

       //最后的测试，替换下次碰撞的时间，并更新爆炸效果的数组
       if (lamda!=10000)
       {
            RestTime-=lamda;
            for (int j=0;j<m_nrOfBalls;j++)
            {
               m_arrayPos[j]=m_oldPos[j]+m_arrayVel[j]*lamda;
            }

            rt2=m_arrayVel[BallNr].mag();// 返回速度向量的模
            m_arrayVel[BallNr].unit();// 归一化速度向量
            //计算反射向量
            m_arrayVel[BallNr]=TVector::unit((normal*(2*normal.dot(-m_arrayVel[BallNr])))
                                             + m_arrayVel[BallNr] );
            m_arrayVel[BallNr]=m_arrayVel[BallNr]*rt2;

            //Update explosion array
            for(int j=0;j<20;j++)
            {
                if (m_explosionArray[j]._Alpha<=0)
                {
                  m_explosionArray[j]._Alpha=1;
                  m_explosionArray[j]._Position=point;
                  m_explosionArray[j]._Scale=1;
                  break;
                }
            }
       }
       else
       {
           RestTime=0;
       }
    }
}

void MyGLWidget::initVars()
{
     //create palnes
    m_pl1._Position=TVector(0,-300,0);
    m_pl1._Normal=TVector(0,1,0);
    m_pl2._Position=TVector(300,0,0);
    m_pl2._Normal=TVector(-1,0,0);
    m_pl3._Position=TVector(-300,0,0);
    m_pl3._Normal=TVector(1,0,0);
    m_pl4._Position=TVector(0,0,300);
    m_pl4._Normal=TVector(0,0,-1);
    m_pl5._Position=TVector(0,0,-300);
    m_pl5._Normal=TVector(0,0,1);


    //create cylinders
    m_cyl1._Position=TVector(0,0,0);
    m_cyl1._Axis=TVector(0,1,0);
    m_cyl1._Radius=60+20;
    m_cyl2._Position=TVector(200,-300,0);
    m_cyl2._Axis=TVector(0,0,1);
    m_cyl2._Radius=60+20;
    m_cyl3._Position=TVector(-200,0,0);
    m_cyl3._Axis=TVector(0,1,1);
    m_cyl3._Axis.unit();
    m_cyl3._Radius=30+20;
    //create quadratic object to render cylinders
    m_cylinder_obj= gluNewQuadric();
    gluQuadricTexture(m_cylinder_obj, GL_TRUE);

    //Set initial positions and velocities of balls
    //also initialize array which holds explosions
    m_nrOfBalls=10;
    m_arrayVel[0]=veloc;
    m_arrayPos[0]=TVector(199,180,10);
    m_explosionArray[0]._Alpha=0;
    m_explosionArray[0]._Scale=1;
    m_arrayVel[1]=veloc;
    m_arrayPos[1]=TVector(0,150,100);
    m_explosionArray[1]._Alpha=0;
    m_explosionArray[1]._Scale=1;
    m_arrayVel[2]=veloc;
    m_arrayPos[2]=TVector(-100,180,-100);
    m_explosionArray[2]._Alpha=0;
    m_explosionArray[2]._Scale=1;
    for (int i=3; i<10; i++)
    {
         m_arrayVel[i]=veloc;
         m_arrayPos[i]=TVector(-500+i*75, 300, -500+i*50);
         m_explosionArray[i]._Alpha=0;
         m_explosionArray[i]._Scale=1;
    }
    for (int i=10; i<20; i++)
    {
         m_explosionArray[i]._Alpha=0;
         m_explosionArray[i]._Scale=1;
    }
}

void MyGLWidget::loadGLTexture()
{
    QImage image1(":/image/Marble.bmp");
    QImage image2(":/image/Spark.bmp");
    QImage image3(":/image/Boden.bmp");
    QImage image4(":/image/Wand.bmp");
    image1 = image1.convertToFormat(QImage::Format_RGB888);
    image1 = image1.mirrored();
    image2 = image2.convertToFormat(QImage::Format_RGB888);
    image2 = image2.mirrored();
    image3 = image3.convertToFormat(QImage::Format_RGB888);
    image3 = image3.mirrored();
    image4 = image4.convertToFormat(QImage::Format_RGB888);
    image4 = image4.mirrored();
    /* Create Texture	*****************************************/
    glGenTextures(2, &m_texture[0]);
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image1.width(), image1.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, image1.bits());

    /* Create Texture	******************************************/
    glBindTexture(GL_TEXTURE_2D, m_texture[1]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image2.width(), image2.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, image2.bits());

    /* Create Texture	********************************************/
    glGenTextures(2, &m_texture[2]);
    glBindTexture(GL_TEXTURE_2D, m_texture[2]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image3.width(), image3.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, image3.bits());

    /* Create Texture	*********************************************/
    glBindTexture(GL_TEXTURE_2D, m_texture[3]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image4.width(), image4.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, image4.bits());
}

//判断是否和平面相交，是则返回1，否则返回0
int MyGLWidget::testIntersionPlane(const Plane& plane,const TVector& position,
                                   const TVector& direction,double& lamda, TVector& pNormal)
{
    double DotProduct=direction.dot(plane._Normal);
    double l2;

    //判断是否平行于平面
    if ((DotProduct<ZERO)&&(DotProduct>-ZERO))
        return 0;

    l2=(plane._Normal.dot(plane._Position-position))/DotProduct;

    if (l2<-ZERO)
        return 0;

    pNormal=plane._Normal;
    lamda=l2;
    return 1;

}

//射线-圆柱的碰撞检测
//计算射线和圆柱方程组得解。
int MyGLWidget::testIntersionCylinder(const Cylinder& cylinder,const TVector& position,
                                      const TVector& direction,
                                      double& lamda, TVector& pNormal,TVector& newposition)
{
    TVector RC;
    double d;
    double t,s;
    TVector n,O;
    double ln;
    double in,out;

    TVector::subtract(position,cylinder._Position,RC);
    TVector::cross(direction,cylinder._Axis,n);

    ln=n.mag();

    if ( (ln<ZERO)&&(ln>-ZERO) )
    {
        return 0;
    }
    n.unit();
    d= fabs( RC.dot(n) );

    if (d<=cylinder._Radius)
    {
        TVector::cross(RC,cylinder._Axis,O);
        t= - O.dot(n)/ln;
        TVector::cross(n,cylinder._Axis,O);
        O.unit();
        s= fabs( sqrt(cylinder._Radius*cylinder._Radius - d*d) / direction.dot(O) );
        in=t-s;
        out=t+s;

        if (in<-ZERO)
        {
            if (out<-ZERO)
            {
                return 0;
            }
            else
            {
                lamda=out;
            }
        }
        else if (out<-ZERO)
        {
            lamda=in;
        }
        else if (in<out)
        {
            lamda=in;
        }
        else
        {
            lamda=out;
        }
        newposition=position+direction*lamda;
        TVector HB=newposition-cylinder._Position;
        pNormal=HB - cylinder._Axis*(HB.dot(cylinder._Axis));
        pNormal.unit();
        return 1;
    }
    return 0;
}

//你可以从源代码得到全部的信息,我尽了最大的努力来解释每一行代码,一旦碰撞的原理知道了,代码是非常简单的.
//就像我开头所说的,碰撞检测这个题目是非常难得,你已经学会了很多新的知识,并能够用它创建出非常棒的演示.
//但在这个课题,你认友很多需要学习,既然你已经开始了,其它的原理和模型就非常容易了.
