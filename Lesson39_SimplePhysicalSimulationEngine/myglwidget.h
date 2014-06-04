#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>
#include <qmath.h>

//物理模拟介绍
//如果你很熟悉物理规律，并且想实现它，这篇文章很适合你。
//在这篇教程里，你会创建一个非常简单的物理引擎，我们将创建以下类：
//内容:
//位置类
//* class Vector3D	--->	用来记录物体的三维坐标的类
//力和运动
//* class Mass	--->	表示一个物体的物理属性
//模拟类
//* class Simulation	--->	模拟物理规律
//模拟匀速运动
//* class ConstantVelocity : public Simulation	--->	模拟匀速运动
//模拟在力的作用下运动
//* class MotionUnderGravitation : public Simulation	--->	模拟在引力的作用下运动
//* class MassConnectedWithSpring : public Simulation	--->	模拟在弹簧的作用下运动

// class Vector3D		---> An object to represent a 3D vector or a 3D point in space
class Vector3D
{
public:
    float x;									// the x value of this Vector3D
    float y;									// the y value of this Vector3D
    float z;									// the z value of this Vector3D

    Vector3D()									// Constructor to set x = y = z = 0
    {
        x = 0;
        y = 0;
        z = 0;
    }

    Vector3D(float x, float y, float z)			// Constructor that initializes this Vector3D to the intended values of x, y and z
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    Vector3D& operator= (Vector3D v)			// operator= sets values of v to this Vector3D. example: v1 = v2 means that values of v2 are set onto v1
    {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }

    Vector3D operator+ (Vector3D v)				// operator+ is used to add two Vector3D's. operator+ returns a new Vector3D
    {
        return Vector3D(x + v.x, y + v.y, z + v.z);
    }

    Vector3D operator- (Vector3D v)				// operator- is used to take difference of two Vector3D's. operator- returns a new Vector3D
    {
        return Vector3D(x - v.x, y - v.y, z - v.z);
    }

    Vector3D operator* (float value)			// operator* is used to scale a Vector3D by a value. This value multiplies the Vector3D's x, y and z.
    {
        return Vector3D(x * value, y * value, z * value);
    }

    Vector3D operator/ (float value)			// operator/ is used to scale a Vector3D by a value. This value divides the Vector3D's x, y and z.
    {
        return Vector3D(x / value, y / value, z / value);
    }

    Vector3D& operator+= (Vector3D v)			// operator+= is used to add another Vector3D to this Vector3D.
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    Vector3D& operator-= (Vector3D v)			// operator-= is used to subtract another Vector3D from this Vector3D.
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    Vector3D& operator*= (float value)			// operator*= is used to scale this Vector3D by a value.
    {
        x *= value;
        y *= value;
        z *= value;
        return *this;
    }

    Vector3D& operator/= (float value)			// operator/= is used to scale this Vector3D by a value.
    {
        x /= value;
        y /= value;
        z /= value;
        return *this;
    }


    Vector3D operator- ()						// operator- is used to set this Vector3D's x, y, and z to the negative of them.
    {
        return Vector3D(-x, -y, -z);
    }

    float length()								// length() returns the length of this Vector3D
    {
        return sqrtf(x*x + y*y + z*z);
    }

    void unitize()								// unitize() normalizes this Vector3D that its direction remains the same but its length is 1.
    {
        float length = this->length();

        if (length == 0)
            return;

        x /= length;
        y /= length;
        z /= length;
    }

    Vector3D unit()								// unit() returns a new Vector3D. The returned value is a unitized version of this Vector3D.
    {
        float length = this->length();

        if (length == 0)
            return *this;

        return Vector3D(x / length, y / length, z / length);
    }

};

class Mass
{
public:
    float m;// 质量
    Vector3D pos;// 位置
    Vector3D vel;// 速度
    Vector3D force;// 力

    Mass(float m)// 构造函数
    {
        this->m = m;
    }
    //下面的代码给物体增加一个力，在初始时这个力为0
    void applyForce(Vector3D force)
    {
        this->force += force;// 增加一个力
    }
    // 初始时设为0
    void init()
    {
        force.x = 0;
        force.y = 0;
        force.z = 0;
    }
    //下面的步骤完成一个模拟：
    //1.设置力
    //2.应用外力
    //3.根据力的时间，计算物体的位置和速度
    void simulate(float dt)
    {
        vel += (force / m) * dt;// 更新速度

        pos += vel * dt;// 更新位置
    }

};

//模拟类怎样运作:
//在一个物理模拟中，我们按以下规律进行模拟，设置力，更新物体的位置和速度，按时间一次又一次的进行模拟。下面是它的实现代码：
// class Simulation		---> A container object for simulating masses
class Simulation
{
public:
    int numOfMasses;// 物体的个数
    Mass** masses;// 指向物体结构的指针

    Simulation(int numOfMasses, float m)// 构造函数
    {
        this->numOfMasses = numOfMasses;

        masses = new Mass*[numOfMasses];

        for (int a = 0; a < numOfMasses; ++a)
            masses[a] = new Mass(m);
    }

    virtual void release()// 释放所有的物体
    {
        for (int a = 0; a < numOfMasses; ++a)
        {
            delete(masses[a]);
            masses[a] = NULL;
        }

        delete(masses);
        masses = NULL;
    }

    Mass* getMass(int index)
    {
        if (index < 0 || index >= numOfMasses)// 返回第i个物体
            return NULL;

        return masses[index];
    }

    virtual void init()// 初始化所有的物体
    {
        for (int a = 0; a < numOfMasses; ++a)
            masses[a]->init();
    }

    virtual void solve()
    {
    }

    virtual void simulate(float dt)//让所有的物体模拟一步
    {
        for (int a = 0; a < numOfMasses; ++a)
            masses[a]->simulate(dt);
    }

    //整个模拟的部分被封装到下面的函数中
    virtual void operate(float dt)//  完整的模拟过程
    {
        init();// 设置力为0
        solve();// 应用力
        simulate(dt);// 模拟
    }

};

//下面让我们来写着两个具体的模拟类:

//1. 具有恒定速度的物体
//* class ConstantVelocity : public Simulation ---> 模拟一个匀速运动的物体
class ConstantVelocity : public Simulation
{
public:
    ConstantVelocity() : Simulation(1, 1.0f)
    {
        masses[0]->pos = Vector3D(0.0f, 0.0f, 0.0f);// 初始位置为0
        masses[0]->vel = Vector3D(1.0f, 0.0f, 0.0f);// 向右运动
    }

};

//下面我们来创建一个具有恒定加速的物体：
class MotionUnderGravitation : public Simulation
{
public:
    Vector3D gravitation;// 加速度

    MotionUnderGravitation(Vector3D gravitation) : Simulation(1, 1.0f)//  构造函数
    {
        this->gravitation = gravitation;// 设置加速度
        masses[0]->pos = Vector3D(-10.0f, 0.0f, 0.0f);// 设置位置为左边-10处
        masses[0]->vel = Vector3D(10.0f, 15.0f, 0.0f);// 设置速度为右上
    }

    //下面的函数设置施加给物体的力
    virtual void solve()
    {
        // 设置当前的力
        for (int a = 0; a < numOfMasses; ++a)								//we will apply force to all masses (actually we have 1 mass, but we can extend it in the future)
            masses[a]->applyForce(gravitation * masses[a]->m);				//gravitational force is as F = m * g. (mass times the gravitational acceleration)
    }

};

//下面的类创建一个受到与距离成正比的力的物体：
class MassConnectedWithSpring : public Simulation
{
public:
    float springConstant;// 弹性系数
    Vector3D connectionPos;// 连接方向

    MassConnectedWithSpring(float springConstant) : Simulation(1, 1.0f)// 构造函数
    {
        this->springConstant = springConstant;								//set the springConstant

        connectionPos = Vector3D(0.0f, -5.0f, 0.0f);						//set the connectionPos

        masses[0]->pos = connectionPos + Vector3D(10.0f, 0.0f, 0.0f);		//set the position of the mass 10 meters to the right side of the connectionPos
        masses[0]->vel = Vector3D(0.0f, 0.0f, 0.0f);						//set the velocity of the mass to zero
    }

    //下面的函数设置当前物体所受到的力：
    virtual void solve()// 设置当前的力
    {
        for (int a = 0; a < numOfMasses; ++a)
        {
            Vector3D springVector = masses[a]->pos - connectionPos;			//find a vector from the position of the mass to the connectionPos
            masses[a]->applyForce(-springVector * springConstant);			//apply the force according to the famous spring force formulation
        }
    }

};

class MyGLWidget : public QGLWidget
{
    Q_OBJECT

public:
    explicit MyGLWidget(QWidget *parent = 0);
    ~MyGLWidget();
protected:
    void resizeGL(int w, int h);

    void initializeGL();

    void paintGL();

    void keyPressEvent(QKeyEvent *event);

    void timerEvent(QTimerEvent *event);
private:
    void loadGLTexture();

    void buildFont(); // 创建我们的字符显示列表

    void glPrint(GLint x, GLint y, GLint z, char *string, int set = 1);

    void update(quint32 milliseconds);
private:
    bool m_show_full_screen;
    ConstantVelocity* m_constantVelocity;
    MotionUnderGravitation* m_motionUnderGravitation;
    MassConnectedWithSpring* m_massConnectedWithSpring;

    float m_slowMotionRatio;
    float m_timeElapsed;

    GLuint m_texture[1];
    GLuint m_base;
};

#endif // MYGLWIDGET_H
