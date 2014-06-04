#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <GL/glaux.h>
#include <QMessageBox>
#include <QApplication>
#include <stdio.h>

//现在我们来定义自己的网格类：
class CVert														// 顶点类
{
public:
    float x;
    float y;
    float z;
};
typedef CVert CVec;

class CTexCoord													// 纹理坐标类
{
public:
    float u;
    float v;
};

//网格类

class CMesh
{
public:
    // 网格数据
    int				m_nVertexCount;								// 顶点个数
    CVert*			m_pVertices;								// 顶点数据的指针
    CTexCoord*		m_pTexCoords;								// 顶点的纹理坐标
    unsigned int	m_nTextureId;								// 纹理的ID
    unsigned int	m_nVBOVertices;								// 顶点缓存对象的名称
    unsigned int	m_nVBOTexCoords;							// 顶点纹理缓存对象的名称
    QImage          m_pTextureImage;							// 高度数据
public:
    CMesh();													// 构造函数
    ~CMesh();													// 析构函数
    // 载入高度图
    bool loadHeightmap(float flHeightScale, float flResolution );
    // 返回单个点的高度
    float ptHeight( int nX, int nY );
    // 创建顶点缓存对象
    void buildVBOs();
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
    bool m_show_full_screen;
    CMesh* m_pMesh;                                 // 网格数据
    float m_flYRot;                                 // 旋转角度
    int	m_nFPS;
    int m_nFrames;                                  // 帧率计数器
    long m_dwLastFPS;								// 上一帧的计数
    qlonglong m_timerCounter;

};

#endif // MYGLWIDGET_H
