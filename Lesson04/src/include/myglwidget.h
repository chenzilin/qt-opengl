#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLWidget>
#include <QKeyEvent>

class MyGLWidget : public QGLWidget
{
	Q_OBJECT

public:
	explicit MyGLWidget(QWidget *parent = 0);

	~MyGLWidget();

protected:

	virtual void paintGL();
	virtual void initializeGL();
	virtual void resizeGL(int width, int height);
	virtual void timerEvent(QTimerEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);

private:

	bool mFullScreen;

	GLfloat mRotateTriangle;
	GLfloat mRotateQuad;
};

#endif // MYGLWIDGET_H

