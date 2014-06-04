#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>
#include "model.h"

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
    bool m_show_full_screen;  
    Model *m_pModel;	// 定义一个指向模型类的指针
    GLfloat m_yrot;
};

#endif // MYGLWIDGET_H
