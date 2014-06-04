#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>
#include <qmath.h>

//现在我们将定义一些结构体来帮助我们存贮我们的数据（保存好几百浮点数组）。第一个是tagMATRIX结构体。如果你仔细地看，
//你将看到我们正象包含一个十六个浮点数的1维数组~一个2维4×4数族一样存储那个矩阵。
//这下至OpenGL存储它的矩阵的方式。如果我们使用4x4数组，这些值将发生错误的顺序。

// 保存OpenGL矩阵的结构体
typedef struct tagMATRIX
{
    float Data[16];// 由于OpenGL的矩阵的格式我们使用[16
}MATRIX;

//第二是向量的类。 仅存储X，Y和Z的值
// 存储一个单精度向量的结构体
typedef struct tagVECTOR
{
    float X, Y, Z;// 向量的分量
}VECTOR;

//第三，我们持有顶点的结构。每一个顶点仅需要它的法线和位置（没有纹理的现行纵坐标）信息。它们必须以这样的次序被存放，
//否则当它停止装载文件的事件将发生严重的错误（我发现艰难的情形：（教我分块出租我的代码。）。
// 存放单一顶点的结构
typedef struct tagVERTEX
{
    VECTOR Nor;// 顶点法线
    VECTOR Pos;// 顶点位置
}VERTEX;

//最后是多边形的结构。我知道这是存储顶点的愚蠢的方法，要不是它完美工作的简单的缘故。
//通常我愿意使用一个顶点数组，一个多边形数组，和包括一个在多边形中的3个顶点的指数，但这比较容易显示你想干什么。
// 存储单一多边形的结构
typedef struct tagPOLYGON
{
    VERTEX Verts[3];					// 3个顶点结构数组
}POLYGON;

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
    bool readMesh();
private:
    bool m_show_full_screen;

    //优美简单的材料也在这里了。为每一个变量的一个解释考虑那个注释。
    bool m_outlineDraw;// 绘制轮廓的标记
    bool m_outlineSmooth;// Anti-Alias 线段的标记
    float m_outlineColor[3];// 线段的颜色
    float m_outlineWidth;// 线段的宽度
    VECTOR m_lightAngle;// 灯光的方向
    bool m_lightRotate;// 是否我们旋转灯光的标记
    float m_modelAngle;// 模型的Y轴角度
    bool m_modelRotate;// 旋转模型的标记
    POLYGON	*m_polyData;// 多边形数据
    int	m_polyNum;// 多边形的编号
    GLuint m_shaderTexture[1];// 存储纹理ID
};

#endif // MYGLWIDGET_H
