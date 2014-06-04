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

//设计绳索的物理模型:
//我们在经典力学和高于500Mhz的计算机上模拟这个问题。首先我们需要设定需要的精度，我们使用一系列互相用弹簧连接的质点来模拟绳索，精度决定了我们用多少个点来模拟，
//当然越多越精确。在下面我决定用50或100个点来模拟绳子一段3或4m长的绳子，换句话说，我们的模拟精度就是3到8厘米。
//设计运动模型:
//在绳子中，施加给各个质点的力来自于自身的质量和相连的内力（参见大学里的普通力学）。如下我们用"O"表示质点，“—”表示连接质点的弹簧。
//O----O----O----O
//1    2    3    4
//弹簧的力学公式如下:
//力 = -k * x
//k: 弹性系数
//x: 相距平衡位置的位移
//上面的公式说明，如果相邻点的距离为平衡距离，那么它们不受到任何力的作用。如果我们设置平衡位置为5cm，那么100个点的绳子长5m。如果相连质点之间的位置小于5cm，它们受到排斥力。
//上面的公式只是一个基础，现在我们可以加上摩擦力，如果没有这项，那么绳子将永远动下去。
//弹簧类:
//这个类包含相连接的两个物体，它们之间具有作用力。
class Spring
{
public:
    Mass* mass1;// 质点1
    Mass* mass2;// 质点2

    float springConstant;// 弹性系数
    float springLength;//弹簧长度
    float frictionConstant;//摩擦系数

    Spring(Mass* mass1, Mass* mass2, float springConstant, float springLength, float frictionConstant)// 构造函数
    {
        this->springConstant = springConstant;									//set the springConstant
        this->springLength = springLength;										//set the springLength
        this->frictionConstant = frictionConstant;								//set the frictionConstant

        this->mass1 = mass1;													//set mass1
        this->mass2 = mass2;													//set mass2
    }

    void solve()// 计算各个物体的受力
    {
        Vector3D springVector = mass1->pos - mass2->pos;							//vector between the two masses

        float r = springVector.length();// 计算两个物体之间的距离

        Vector3D force;																//force initially has a zero value

        if (r != 0)// 计算力
            force += (springVector / r) * (r - springLength) * (-springConstant);	//the spring force is added to the force

        force += -(mass1->vel - mass2->vel) * frictionConstant;// 加上摩擦力
                                                                                    //with this addition we obtain the net force of the spring

        mass1->applyForce(force);// 给物体1施加力
        mass2->applyForce(-force);// 给物体2施加力
    }
};

//下面我们把绳子钉在墙上，所以我们的模拟就多了一个万有引力，空气摩擦力。万有引力的公式如下：
//力 = (重力加速度) * 质量
//万有引力会作用在每一个质点上，地面也会给每个物体一个作用力。在我们的模型中将考虑绳子和地面之间的接触，地面给绳子向上的力，并提供摩擦力。
//设置模拟的初始值
//现在我们已经设置好模拟环境了，长度单位是m，时间单位是秒，质量单位是kg。
//为了设置初始值，我们必须提供供模拟开始的参数。我们定义一下参数：
//1. 重力加速度: 9.81 m/s/s 垂直向下
//2. 质点个数: 80
//3. 相连质点的距离: 5 cm (0.05 meters)
//4. 质量: 50 克(0.05 kg)
//5. 绳子开始处于垂直状态
//下面计算绳子受到的力
//f = (绳子质量) * (重力加速度) = (4 kg) * (9.81) ~= 40 N
//弹簧必须平衡这个力 40 N，它伸长1cm，计算弹性系数:
//合力= -k * x = -k * 0.01 m
//合力应该为0 :
//40 N + (-k * 0.01 meters) = 0
//弹性系数 k 为:
//k = 4000 N / m
//设置弹簧的摩擦系数:
//springFrictionConstant = 0.2 N/(m/s)
//下面我们看看这个绳索类：
//1. virtual void init()	--->	重置力
//2. virtual void solve()	--->	计算各个质点的力
//3. virtual void simulate(float dt)	--->	模拟一次
//4. virtual void operate(float dt)	--->	执行一次操作
//绳索类如下所示 :
class RopeSimulation : public Simulation				//An object to simulate a rope interacting with a planer surface and air
{
public:
    Spring** springs;// 弹簧类结构的数组的指针

    Vector3D gravitation;// 万有引力

    Vector3D ropeConnectionPos;// 绳索的连接点

    Vector3D ropeConnectionVel;//连接点的速度，我们使用这个移动绳子

    float groundRepulsionConstant;//地面的反作用力

    float groundFrictionConstant;//地面的摩擦系数

    float groundAbsorptionConstant;//地面的缓冲力

    float groundHeight;//地面高度

    float airFrictionConstant;//空气的摩擦系数

    RopeSimulation(										//a long long constructor with 11 parameters starts here
        int numOfMasses,								//1. the number of masses
        float m,										//2. weight of each mass
        float springConstant,							//3. how stiff the springs are
        float springLength,								//4. the length that a spring does not exert any force
        float springFrictionConstant,					//5. inner friction constant of spring
        Vector3D gravitation,							//6. gravitational acceleration
        float airFrictionConstant,						//7. air friction constant
        float groundRepulsionConstant,					//8. ground repulsion constant
        float groundFrictionConstant,					//9. ground friction constant
        float groundAbsorptionConstant,					//10. ground absorption constant
        float groundHeight								//11. height of the ground (y position)
        ) : Simulation(numOfMasses, m)					//The super class creates masses with weights m of each
    {
        this->gravitation = gravitation;

        this->airFrictionConstant = airFrictionConstant;

        this->groundFrictionConstant = groundFrictionConstant;
        this->groundRepulsionConstant = groundRepulsionConstant;
        this->groundAbsorptionConstant = groundAbsorptionConstant;
        this->groundHeight = groundHeight;

        for (int a = 0; a < numOfMasses; ++a)// 设置质点位置
        {
            masses[a]->pos.x = a * springLength;		//Set x position of masses[a] with springLength distance to its neighbor
            masses[a]->pos.y = 0;						//Set y position as 0 so that it stand horizontal with respect to the ground
            masses[a]->pos.z = 0;						//Set z position as 0 so that it looks simple
        }

        springs = new Spring*[numOfMasses - 1];			//create [numOfMasses - 1] pointers for springs
                                                        //([numOfMasses - 1] springs are necessary for numOfMasses)

        for (int a = 0; a < numOfMasses - 1; ++a)//创建各个质点之间的模拟弹簧
        {
            //Create the spring with index "a" by the mass with index "a" and another mass with index "a + 1".
            springs[a] = new Spring(masses[a], masses[a + 1],
                springConstant, springLength, springFrictionConstant);
        }
    }

    void release()										//release() is overriden because we have springs to delete
    {
        Simulation::release();							//Have the super class release itself

        for (int a = 0; a < numOfMasses - 1; ++a)		//to delete all springs, start a loop
        {
            delete(springs[a]);
            springs[a] = NULL;
        }

        delete(springs);
        springs = NULL;
    }

    void solve()// 计算施加给各个质点的力
    {
        for (int a = 0; a < numOfMasses - 1; ++a)// 弹簧施加给各个物体的力
        {
            springs[a]->solve();						//Spring with index "a" should apply its force
        }

        for (int a = 0; a < numOfMasses; ++a)// 计算各个物体受到的其它的力
        {
            masses[a]->applyForce(gravitation * masses[a]->m);// 万有引力

            masses[a]->applyForce(-masses[a]->vel * airFrictionConstant);	//The air friction

            if (masses[a]->pos.y < groundHeight)// 计算地面对质点的作用
            {
                Vector3D v;								//A temporary Vector3D

                v = masses[a]->vel;// 返回速度
                v.y = 0;// y方向的速度为0

                // 计算地面给质点的力
                masses[a]->applyForce(-v * groundFrictionConstant);		//ground friction force is applied

                v = masses[a]->vel;						//get the velocity
                v.x = 0;								//omit the x and z components of the velocity
                v.z = 0;								//we will use v in the absorption effect

                // 计算地面的缓冲力
                if (v.y < 0)							//let's absorb energy only when a mass collides towards the ground
                    masses[a]->applyForce(-v * groundAbsorptionConstant);		//the absorption force is applied

                // 计算地面的反作用力
                Vector3D force = Vector3D(0, groundRepulsionConstant, 0) *
                    (groundHeight - masses[a]->pos.y);

                masses[a]->applyForce(force);// 施加地面对质点的力
            }

        }


    }

    void simulate(float dt)	// 模拟一次
    {
        Simulation::simulate(dt);// 调用基类的模拟函数

        ropeConnectionPos += ropeConnectionVel * dt;// 计算绳子的连接点

        if (ropeConnectionPos.y < groundHeight)			//ropeConnectionPos shall not go under the ground
        {
            ropeConnectionPos.y = groundHeight;
            ropeConnectionVel.y = 0;
        }

        masses[0]->pos = ropeConnectionPos;// 更新绳子的连接点和速度
        masses[0]->vel = ropeConnectionVel;
    }

    void setRopeConnectionVel(Vector3D ropeConnectionVel)	//the method to set ropeConnectionVel
    {
        this->ropeConnectionVel = ropeConnectionVel;
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

    void keyReleaseEvent(QKeyEvent *event);

    void timerEvent(QTimerEvent *event);
private:
    void update(quint32 milliseconds);
private:
    bool m_show_full_screen;
    RopeSimulation* m_ropeSimulation;
};

#endif // MYGLWIDGET_H
