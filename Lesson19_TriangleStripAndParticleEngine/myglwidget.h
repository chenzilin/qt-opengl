#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <GL/glaux.h>
#include <QMessageBox>
#include <QApplication>

#define	MAX_PARTICLES	1000		// 定义最大的粒子数

class MyGLWidget : public QGLWidget
{
    Q_OBJECT

public:
    //好!现在是有趣的东西.下段程序描述单一粒子结构,这是我们给予粒子的属性.
    //我们用布尔型变量active开始,如果为true,我们的粒子为活跃的.如果为false则粒子为死的,此时我们就删除它.
    //在程序中我没有使用活跃的,因为它很好的实现.变量life和fade来控制粒子显示多久以及显示时候的亮度.
    //随着life数值的降低fade的数值也相应降低.这将导致一些粒子比其他粒子燃烧的时间长.
    typedef struct						// 创建粒子数据结构
    {
        bool active;					// 是否激活
        float life;					// 粒子生命
        float fade;					// 衰减速度
        //变量r,g和b用来表示粒子的红色强度,绿色强度和蓝色强度.当r的值变成1.0f时粒子将会很红,当三个变量全为1.0f时则粒子将变成白色.
        float r;					// 红色值
        float g;					// 绿色值
        float b;					// 蓝色值
        //变量x.y和z控制粒子在屏幕上显示的位置.x表示粒子在x轴上的位置.y表示y轴上的位置.z表示粒子z轴上的位置
        float x;					// X 位置
        float y;					// Y 位置
        float z;					// Z 位置
        //下面三个变量很重要.这三个变量控制粒子在每个轴上移动的快慢和方向.如果xi是负价粒子将会向左移动,正值将会向右移动.
        //如果yi是负值粒子将会向下移动,正值将向上.最后,如果zi负值粒子将会向荧屏内部移动,正植将移向观察者.
        float xi;					// X 方向
        float yi;					// Y 方向
        float zi;					// Z 方向
        //最后,另外3个变量!每一个变量可被看成加速度.如果xg正值时,粒子将会被拉倒右边,负值将拉向左边.
        //所以如果粒子向左移动(负的)而我们给它一个正的加速度,粒子速度将变慢.最后将向反方向移动(高中物理).
        //yg拉下或拉上.zg拉进或拉出屏幕.
        float xg;					// X 方向重力加速度
        float yg;					// Y 方向重力加速度
        float zg;					// Z 方向重力加速度
        //结构的名字为particles.
    }particles;

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
private:
    bool m_show_full_screen;
    GLuint m_texture[1];

    bool m_rainbow;			// 是否为彩虹模式

    //下面四行是复杂的变量.变量slowdown控制粒子移动的快慢.数值愈高,移动越慢.数值越底,移动越快.
    //如果数值降低,粒子将快速的移动!粒子的速度影响它们在荧屏中移动的距离.记住速度慢的粒子不会射很远的.
    //变量xspeed和yspeed控制尾部的方向.xspeed将会增加粒子在x轴上速度.如果xspeed是正值粒子将会向右边移动多.
    //如果xspeed负价值,粒子将会向左边移动多.那个值越高,就向那个方向移动比较多.yspeed工作相同的方法,但是在y轴上.
    //因为有其它的因素影响粒子的运动,所以我要说"多".xspeed和yspeed有助于在我们想要的方向上移动粒子.最后是变量zoom,
    //我们用该变量移入或移出我们的屏幕.在粒子引擎里,有时可看见更多的图象,而且当接近你时很酷
    float m_slowdown;			// 减速粒子
    float m_xspeed;				// X方向的速度
    float m_yspeed;				// Y方向的速度
    float m_zoom;			// 沿Z轴缩放
    //下面我们创建一个数组叫particle.数组存储MAX_PARTICLES个元素.也就是说我们创建1000(MAX_PARTICLES)个粒子,
    //存储空间为每个粒子提供相应的信息
    particles particle[MAX_PARTICLES];				// 保存1000个粒子的数组

    GLuint m_col;				// 当前的颜色
    GLuint m_delay;				// 彩虹效果延迟
};

#endif // MYGLWIDGET_H
