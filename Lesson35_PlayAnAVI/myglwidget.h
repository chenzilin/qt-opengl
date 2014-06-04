#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>
#include <QMediaPlayer>
#include <QApplication>
#include "videowidgetsurface.h"

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
private slots:
    void newAVIImage(QImage image);
private:
    bool m_show_full_screen;
    float m_angle;// 旋转用
    int	m_effect;// 当前效果
    bool m_env;// 环境映射(默认开)
    bool m_bg;// 背景(默认开)
    GLUquadricObj *m_quadratic;// 存储二次曲面对象
    QMediaPlayer *m_player;
    VideoWidgetSurface *m_surface;
    QImage m_oriImage;
};

#endif // MYGLWIDGET_H
