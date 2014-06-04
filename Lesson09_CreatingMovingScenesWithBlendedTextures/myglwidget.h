#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>
#include <GL/glaux.h>

class MyGLWidget : public QGLWidget
{
    Q_OBJECT

public:
    //我们来创建一个结构。 结构这词听起来有点可怕，但实际上并非如此。
    //一个结构使用一组简单类型的数据 (以及变量等)来表达较大的具有相似性的数据组合。
    //我们知道我们在保持对星星的跟踪。 您可以看到下面的第七行就是 stars；并且每个星星有三个整型的色彩值。
    //第三行 int r,g,b设置了三个整数. 一个红色 (r), 一个绿色 (g), 以及一个蓝色 (b).
    //此外，每个星星离屏幕中心的距离不同, 而且可以是以屏幕中心为原点的任意360度中的一个角度。
    //如果你看下面第四行的话, 会发现我们使用了一个叫做 dist的浮点数来保持对距离 的跟踪.
    //第五行则用一个叫做 angle的浮点数保持对星星角度值的跟踪。
    //因此我们使用了一组数据来描述屏幕上星星的色彩, 距离, 和角度。 不幸的是我们不止对一个星星进行跟踪。
    //但是无需创建 50 个红色值、 50 个绿色值、 50 个蓝色值、 50 个距离值 以及 50 个角度值，而只需创建一个数组star。
    //star数组的每个元素都是stars类型的，里面存放了描述星星的所有数据。
    //star数组在下面的第八行创建。 第八行的样子是这样的： stars star[num]。数组类型是 stars结构.
    //所数组 能存放所有stars结构的信息。 数组名字是 star. 数组大小是 [num]。 数组中存放着 stars结构的元素.
    //跟踪结构元素会比跟踪各自分开的变量容易的多. 不过这样也很笨, 因为我们竟然不能改变常量 num来增减星星数量。

    typedef struct							// 为星星创建一个结构
    {
        int r, g, b;						// 星星的颜色
        GLfloat dist;						// 星星距离中心的距离
        GLfloat angle;						// 当前星星所处的角度
    }stars;								// 结构命名为stars

    explicit MyGLWidget(QWidget *parent = 0);
    ~MyGLWidget();
protected:
    void resizeGL(int w, int h);

    void initializeGL();

    void paintGL();

    void keyPressEvent(QKeyEvent *event);

    void timerEvent(QTimerEvent *event);
private:
    void loadTexture();
private:
    bool m_show_full_screen;

    bool m_twinkle;
    QVector<stars> m_stars;
    GLfloat	m_zoom;	// 星星离观察者的距离
    GLfloat m_tilt;	// 星星的倾角
    GLfloat	m_spin;	// 闪烁星星的自转
    GLuint	m_texture[1]; // 存放一个纹理

};

#endif // MYGLWIDGET_H
