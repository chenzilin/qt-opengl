#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>
#include <QSound>
#include <QTime>

//现在为我们物体的结构. 这个结构存了所有一个物体的信息. 旋转的方向, 若被打中, 在屏幕的位置, 等等.
//一个快速运动的变量... rot 我想让物体旋转特别的方向. hit 若物体没被打中将是 FALSE . 若物体给打中或飞出, 变量将是 TRUE.
//变量frame 是用来存我们爆炸动画的周期. 每一帧改变增加一个爆炸材质. 在这课有更多在不久.
//保存单个物体的移动方向, 我们用变量 dir. 一个dir 能有4 个值: 0 - 物体左移, 1 - 物体右移, 2 - 物体上移 和最后 3 - 物体下移
//texid 能是从0到4的数. 0 表示是蓝面材质, 1 是水桶材质， 2 是靶子的材质 , 3 是 可乐的材质 和 4 是 花瓶 材质.
//最近在调入材质的代码, 你将看到先前5种材质来自目标图片.
//x 和 y 两者都用来记屏模上物体的位置. x 表示物体在 x-轴, y 表示物体在 y-轴.
//物体在z-轴上的旋转是记在变量spin. 在以后的代码, 我们将加或减spin基数在旅行的方向上.
//最后, distance 保存我们物体到屏幕的距离. 距离是极端重要的变量, 我们将用他来计算屏幕的左右两边, 而且在对象关闭之前排序物体，
//画出物体的距离.

struct objects
{
    GLuint	rot;// 旋转 (0-不转, 1-顺时针转, 2-逆时针转)
    bool	hit;// 物体碰撞?
    GLuint	frame;//当前爆炸效果的动画帧
    GLuint	dir;// 物体的方向 (0-左, 1-右, 2-上, 3-下)
    GLuint	texid;// 物体材质 ID
    GLfloat	x;//  物体 X 位置
    GLfloat y;// 物体 Y 位置
    GLfloat	spin;//物体旋转
    GLfloat	distance;// 物体距离
};

//我不想限制每个物体的大小. 我想瓶子(vase)比can高, 我想水桶bucket比瓶子宽. 去改变一切是简单的,
//我建了一个结构存物体的宽和高.
//我然后在最后一行代码中设每个物体的宽高. 得到这个coke cans的宽, 我将检查size[3].w. 蓝面是 0, 水桶是 1, 和靶子是 2,
//等. 宽度表现在 w. 使有意义?
struct dimensions // 物体维数
{
    GLfloat	w;								// 物体宽
    GLfloat h;								// 物体高
};

typedef struct
{
    QImage imageData;// 图片数据
    QImage::Format bpp;// 图片格式
    GLuint width;// 图片宽度
    GLuint height;// 图片高度
    GLuint texID;// 贴图材质 ID 用来选择一个材质
} TextureImage;// 结构 名称

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

    void mousePressEvent(QMouseEvent *event);

    void mouseMoveEvent(QMouseEvent *event);
private:
    bool loadTGA(TextureImage *texture, const QString &filename);

    void buildFont();

    void glPrint(GLint x, GLint y, const QString &string);

    void initObject(int num);

    void selection();

    void object(float width,float height,GLuint texid);

    void explosion(int num);

    void drawTargets();

    void update(quint32 milliseconds);
private:
    bool m_show_full_screen;
    GLint m_level;// 现在的等级
    GLint m_miss;// 丢失的数
    GLint m_kills;// 打到的数
    GLint m_score;// 当前的分数
    bool m_game;// 游戏是否结束?

    TextureImage m_textures[10];// 定义10个材质
    objects	m_object[30];// 定义 30 个物体
    GLuint m_base;// 字体显示列表
    GLfloat m_roll;// 旋转的云

    QPoint m_mousePoint;
};

#endif // MYGLWIDGET_H
