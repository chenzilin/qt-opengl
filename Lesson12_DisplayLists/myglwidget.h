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
private:
    void buildLists();					// 创建盒子的显示列表

    void loadGLTexture();
private:
    bool m_show_full_screen;

    //下面设置变量。首先是存储纹理的变量，然后两个新的变量用于显示列表。这些变量是指向内存中显示列表的指针。命名为box和top。
    //然后用两个变量xloop,yloop表示屏幕上立方体的位置，两个变量xrot，yrot表示立方体的旋转。
    GLuint m_box;						// 保存盒子的显示列表
    GLuint m_top;						// 保存盒子顶部的显示列表
    GLfloat m_yrot;
    GLfloat m_xrot;
    GLuint m_texture[1];
};

#endif // MYGLWIDGET_H
