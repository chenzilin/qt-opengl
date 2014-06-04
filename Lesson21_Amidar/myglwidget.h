#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>
#include <QSound>

class MyGLWidget : public QGLWidget
{
    Q_OBJECT

public:
    //接下来我们需要一个结构来记录游戏中的对象。fx和fy每次在网格上移动我们的英雄和敌人一些较小的象素，以创建一个平滑的动画效果。
    //x和y则记录着对象处于网格的那个交点上。
    //上下左右各有11个点，因此x和y可以是0到10之间的任意值。这也是我们为什么需要fx和fy的原因。
    //考虑如果我们只能够在上下和左右方向的11个点间移动的话，我们的英雄不得不
    //在各个点间跳跃前进。这样显然是不够平滑美观的。
    //最后一个变量spin用来使对象在Z轴上旋转。
    struct Object								// 记录游戏中的对象
    {
        int	fx, fy;								// 使移动变得平滑
        int	x, y;								// 当前游戏者的位置
        float spin;							// 旋转方向
        Object()
        {
            fx = 0;
            fy = 0;
            x = 0;
            y = 0;
            spin = 0.0f;
        }
    };

    explicit MyGLWidget(QWidget *parent = 0);
    ~MyGLWidget();
protected:
    void resizeGL(int w, int h);

    void initializeGL();

    void paintGL();

    void keyPressEvent(QKeyEvent *event);

    void timerEvent(QTimerEvent *event);
private:
    void resetObjects();

    void loadGLTexture();

    void buildFont();

    void glPrint(GLint x, GLint y, int set, char *fmt);

    void resetLines();
private:
    bool m_show_full_screen;

    //bool类型的变量，vline保存了组成我们游戏网格垂直方向上的121条线，上下水平各11条。hline保存了水平方向上的 121条线，
    //用ap来检查A键是否已经按下。
    //当网格被填满时， filled被设置为TRUE而反之则为FALSE。gameover这个变量的作用显而易见，当他的值为TRUE时，游戏结束。
    //anti指出抗锯齿功能是否打开，当设置为TRUE时，该功能是打开着的。
    bool m_vline[11][10];  // 保存垂直方向的11根线条中，每根线条中的10段是否被走过
    bool m_hline[10][11];	//保存水平方向的11根线条中，每根线条中的10段是否被走过
    bool m_filled;	// 网格是否被填满?
    bool m_gameover; // 游戏是否结束?
    bool m_anti; // 是否启用反走样?
    //delay 是一个计数器，我用他来减慢那些坏蛋的动作。当delay的值大于某一个馈值的时候，敌人才可以行动，此时delay将被重置。
    int m_delay;
    //我们把lives的值设置成5，这样我们的英雄一出场就拥有5条命。level是一个内部变量，用来指出当前游戏的难度。
    //当然，这并不是你在屏幕上所看到的那个Level。变量level2开始的时候和Level拥有相同的值，但是随着你技能的提高，这个值也会增加。
    //当你成功通过难度3之后，这个值也将在难度3上停止增加。level 是一个用来表示游戏难度的内部变量，stage才是用来记录当前游戏关卡的变量。
    int m_lives;							// 玩家的生命
    int	m_level;							// 内部游戏的等级
    int	m_stage;							// 游戏的关卡

    //既然我们已经为我们的玩家，敌人，甚至是秘密武器。设置了结构体，那么同样的，为了表现刚刚创设的结构体的功能和特性，
    //我们也可以为此设置新的结构体。
    //为我们的玩家创设结构体之下的第一条直线。基本上我们将会为玩家提供fx，fy，x，y和spin值几种不同的结构体。
    //通过增加这些直线，仅需查看玩家的x值我们就很容易取得玩家的位置，同时我们也可以通过增加玩家的旋转度来改变玩家的spin值。
    //第二条直线略有不同。因为同一屏幕我们可以同时拥有至多15个敌人。我们需要为每个敌人创造上面所提到的可变量。我
    //们通过设置一个有15个敌人的组来实现这个目标，如第一个敌人的位置被设定为敌人（0）.x.第二个敌人的位置为（1），x等等
    //第三条直线使得为宝物创设结构体实现了可能。宝物是一个会时不时在屏幕上出现的沙漏。我们需要通过沙漏来追踪x和y值。
    //但是因为沙漏的位置是固定的所以我们不需要寻找最佳位置，而通过为程序后面的其他物品寻找好的可变量来实现（如fx和fy）
    Object m_player; // 玩家信息
    Object m_enemy[9]; // 最多9个敌人的信息
    Object m_hourglass; // 宝物信息

    GLuint m_texture[2]; // 字符纹理
    GLuint m_base; // 字符显示列表的开始值

    QSound m_freezeSound;
};

#endif // MYGLWIDGET_H
