#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>
#include <GL/glu.h>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>

typedef struct Texture_Image
{
    int width;// 宽
    int height;// 高
    int format;// 像素格式
    unsigned char *data;// 纹理数据
} TEXTURE_IMAGE;
typedef TEXTURE_IMAGE *P_TEXTURE_IMAGE;

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
    int readTextureData(QString filename, P_TEXTURE_IMAGE buffer);

    void buildTexture (P_TEXTURE_IMAGE tex);

    void blit(P_TEXTURE_IMAGE src, P_TEXTURE_IMAGE dst, int src_xstart,
              int src_ystart, int src_width, int src_height,int dst_xstart,
              int dst_ystart, int blend, int alpha);
private:
    bool m_show_full_screen;
    //接下来定义了两个指向这个结构的指针
    P_TEXTURE_IMAGE m_t1;// 指向保存图像结构的指针
    P_TEXTURE_IMAGE m_t2;// 指向保存图像结构的指针
    GLuint m_texture[1];

    GLfloat m_xrot;
    GLfloat m_yrot;
    GLfloat m_zrot;
};

#endif // MYGLWIDGET_H
