#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>

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

    void buildFont();

    void glPrint(GLint x, GLint y, int set, char *string);
private:
    bool m_show_full_screen;
    //接下来我们添加一些变量，第一个为滚动参数。第二给变量记录扩展的个数，swidth和sheight记录剪切矩形的大小。
    //base为字体显示列表的开始值。
    int m_scroll; // 用来滚动屏幕
    int m_maxtokens; // 保存扩展的个数
    int m_swidth; // 剪裁宽度
    int m_sheight; // 剪裁高度
    GLuint m_base;// 字符显示列表的开始值

    GLuint m_textures[1];
};

#endif // MYGLWIDGET_H
