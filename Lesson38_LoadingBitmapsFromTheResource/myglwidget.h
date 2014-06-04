#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>

// 定义一个物体
struct object
{
    int tex;// 纹理值
    float x;// 位置
    float y;
    float z;
    float yi;// 速度
    float spinz;// 沿Z轴旋转的角度和速度
    float spinzi;
    float flap;	// 是否翻转三角形
    float fi;
    object()
    {
        tex=qrand()%3;	// 纹理
        x=qrand()%34-17.0f;// 位置
        y=18.0f;
        z=-((qrand()%30000/1000.0f)+10.0f);
        spinzi=(qrand()%10000)/5000.0f-1.0f;// 旋转
        flap=0.0f;
        fi=0.05f+(qrand()%100)/1000.0f;
        yi=0.001f+(qrand()%1000)/10000.0f;
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
private:
    bool m_show_full_screen;
    GLuint m_textures[3];// 保存三个纹理
    object m_obj[50];// 创建50个物体
};

#endif // MYGLWIDGET_H
