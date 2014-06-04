#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>

#define	MAP_SIZE	  1024
#define	STEP_SIZE	  16	// 相邻顶点的距离
#define	HEIGHT_RATIO  1.5f

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
private:
    void loadRawFile(const QString &fileName, int nSize, unsigned char *pHeightMap);

    int height(unsigned char *pHeightMap, int x, int y);

    void setVertexColor(unsigned char *pHeightMap, int x, int y);

    void renderHeightMap(unsigned char *pHeightMap);
private:
    bool m_show_full_screen;
    unsigned char m_gHeightMap[MAP_SIZE*MAP_SIZE];// 保存高度数据
    bool m_bRender;//true为多边形渲染，false为线渲染
    GLfloat m_scaleValue;// 地形的缩放比例
};

#endif // MYGLWIDGET_H
