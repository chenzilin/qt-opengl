#include "myglwidget.h"

//在OpenGL中使用FreeType库
//使用FreeType库可以创建非常好看的反走样的字体，记住暴雪公司就是使用这个库的，就是那个做魔兽世界的。
//尝试一下吧，我只告诉你了基本的使用方式，你可以走的更远。

//在OpenGL中使用FreeType库
//这里是一个快速的介绍，它告诉你如何在OpenGL中使用FreeType渲染TrueType字体。使用这个库我们可以渲染反走样的文本，
//它看起来更加的漂亮。
//创建程序
//第一步你需要从下面的网站上下载FreeType库：http://gnuwin32.sourceforge.net/packages/freetype.htm
//接着在你使用它创建一个新的程序时，你需要链接libfreetype.lib库，并包含FreeType的头文件。

namespace freetype
{
    //我们使用纹理去显示字符，在OpenGL中纹理大小必须为2的次方，这个函数用来字符的大小近似到这个值。所以我们有了如下的方程：
    // 这个函数返回比a大的，并且是最接近a的2的次方的数
    inline int next_p2(int a )
    {
        int rval=1;
        // rval<<=1 Is A Prettier Way Of Writing rval*=2;
        while(rval<a) rval<<=1;
        return rval;
    }
    //下面一个函数为make_dlist, 它是这个代码的核心。它包含FT_Face对象，它是FreeType用来保存字体信息的类，
    //接着创建一个显示列表。
    // 为给定的字符创建一个显示列表
    void make_dlist(FT_Face face, char ch, GLuint list_base, GLuint *tex_base)
    {
        // 载入给定字符的轮廓
        if(FT_Load_Glyph(face, FT_Get_Char_Index( face, ch ), FT_LOAD_DEFAULT ))
        {
            qDebug() << "FT_Load_Glyph failed";
        }

        // 保存轮廓对象
        FT_Glyph glyph;
        if(FT_Get_Glyph( face->glyph, &glyph ))
        {
            qDebug() << "FT_Get_Glyph failed";
        }

        // 把轮廓转化为位图
        FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );
        FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
        // 保存位图
        FT_Bitmap& bitmap=bitmap_glyph->bitmap;
        //现在我们已经从FreeType中获得了位图，我们需要把它转化为一个满足OpenGL纹理要求的位图。
        //你必须知道，在OpenGL中位图表示黑白的数据，而在FreeType中我们使用8位的颜色表示位图，
        //所以FreeType的位图可以保存亮度信息。
        // 转化为OpenGl可以使用的大小
        int width = next_p2( bitmap.width );
        int height = next_p2( bitmap.rows );
        // 保存位图数据
        GLubyte* expanded_data = new GLubyte[ 2 * width * height];

        // 这里我们使用8位表示亮度8位表示alpha值
        for(int j=0; j <height;j++)
        {
            for(int i=0; i < width; i++)
            {
                expanded_data[2*(i+j*width)]= expanded_data[2*(i+j*width)+1] =
                (i>=bitmap.width || j>=bitmap.rows) ? 0 : bitmap.buffer[i + bitmap.width*j];
            }
        }
        //接下来我们选则字体纹理，并生成字体的贴图纹理
        //设置字体纹理的纹理过滤器
        glBindTexture(GL_TEXTURE_2D, tex_base[ch]);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        // 绑定纹理
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
        GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data);

        // 释放分配的内存
        delete [] expanded_data;

        //接着创建一个显示列表，它用来绘制一个字符
        // 创建显示列表
        glNewList(list_base+ch,GL_COMPILE);
        glBindTexture(GL_TEXTURE_2D,tex_base[ch]);
        //首先我们向左移动一点
        glTranslatef(bitmap_glyph->left,0,0);
        //接着我们向下移动一点，这只队'g','y'之类的字符有用
        //它使得所有的字符都有一个基线
        glPushMatrix();
        glTranslatef(0,bitmap_glyph->top-bitmap.rows,0);
        // 计算位图中字符图像的宽度
        float x=(float)bitmap.width / (float)width,
        y=(float)bitmap.rows / (float)height;
        //绘制一个正方形，显示字符
        glBegin(GL_QUADS);
            glTexCoord2d(0,0); glVertex2f(0,bitmap.rows);
            glTexCoord2d(0,y); glVertex2f(0,0);
            glTexCoord2d(x,y); glVertex2f(bitmap.width,0);
            glTexCoord2d(x,0); glVertex2f(bitmap.width,bitmap.rows);
        glEnd();
        glPopMatrix();
        glTranslatef(face->glyph->advance.x >> 6 ,0,0);
        //结束显示列表的绘制
        glEndList();
    }

    //下面的函数将使用make_dlist创建一个字符集的显示列表，fname为你要使用的FreeType字符文件。
    void font_data::init(const char * fname, unsigned int h)
    {
        // 保存纹理ID.
        textures = new GLuint[128];
        this->h=h;

        // 创建FreeType库
        FT_Library library;
        if (FT_Init_FreeType( &library ))
        {
            qDebug() << "FT_Init_FreeType failed";
        }

        // 在FreeType库中保存字体信息的类叫做face
        FT_Face face;

        // 使用你输入的Freetype字符文件初始化face类
        if (FT_New_Face( library, fname, 0, &face ))
        {
            qDebug() << "FT_New_Face failed (there is probably a problem with your font file)";
        }

        // 在FreeType中使用1/64作为一个像素的高度所以我们需要缩放h来满足这个要求
        FT_Set_Char_Size( face, h << 6, h << 6, 96, 96);

        // 创建128个显示列表
        list_base=glGenLists(128);
        glGenTextures( 128, textures );
        for(unsigned char i=0;i<128;i++)
        {
            make_dlist(face,i,list_base,textures);
        }
        // 释放face类
        FT_Done_Face(face);

        // 释放FreeType库
        FT_Done_FreeType(library);
    }

    //下面的函数完成释放资源的工作
    void font_data::clean()
    {
        glDeleteLists(list_base,128);
        glDeleteTextures(128,textures);
        delete [] textures;
    }

    //在print函数中要用到下面的两个方程，pushScreenCoordinateMatrix函数用来保存当前的矩阵，并设置视口与当前的窗口大小匹配。
    //pop_projection_matrix函数用来返回pushScreenCoordinateMatrix保存的矩阵。reference manual.
    // 保存当前的矩阵，并设置视口与当前的窗口大小匹配
    inline void pushScreenCoordinateMatrix()
    {
        glPushAttrib(GL_TRANSFORM_BIT);
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(viewport[0],viewport[2],viewport[1],viewport[3]);
        glPopAttrib();
    }

    //返回pushScreenCoordinateMatrix保存的矩阵
    inline void pop_projection_matrix()
    {
        glPushAttrib(GL_TRANSFORM_BIT);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glPopAttrib();
    }

    //我们的print函数和13课的函数非常的像，但在实现上有一些不同。我们实际上是使用2通道的纹理而不是图像。
    // 输出文字
    void print(const font_data &ft_font, float x, float y, const char *fmt, ...)
    {
        // 保存当前矩阵
        pushScreenCoordinateMatrix();

        GLuint font=ft_font.list_base;
        float h=ft_font.h/.63f;
        char text[256];
        va_list	ap;
        if (fmt == NULL)
        {
            *text=0;
        }
        else
        {
            va_start(ap, fmt);
            vsprintf(text, fmt, ap);
            va_end(ap);
        }

        // 把输入的字符串按回车分割
        const char *start_line=text;
        QVector<QString> lines;
        const char *c=text;
        for(;*c;c++)
        {
            if(*c=='\n')
            {
                QString line;
                for(const char *n=start_line;n<c;n++)
                {
                    line.append(*n);
                }
                lines.push_back(line);
                start_line=c+1;
            }
        }

        if(start_line)
        {
            QString line;
            for(const char *n=start_line;n<c;n++)
            {
                line.append(*n);
            }
            lines.push_back(line);
        }

        glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
        glMatrixMode(GL_MODELVIEW);
        glDisable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glListBase(font);

        float modelview_matrix[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, modelview_matrix);

        // 下面的代码完成具体的绘制过程
        for(int i=0;i<lines.size();i++)
        {
            glPushMatrix();
            glLoadIdentity();
            glTranslatef(x,y-h*i,0);
            glMultMatrixf(modelview_matrix);

            //调用显示列表绘制
            glCallLists(lines[i].length(), GL_UNSIGNED_BYTE, lines[i].toLatin1().constData());

            glPopMatrix();
        }
        glPopAttrib();
        pop_projection_matrix();
    }
}

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_cnt1(0.0f)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
    m_our_font.clean();
}

//下面的代码的作用是重新设置OpenGL场景的大小，而不管窗口的大小是否已经改变(假定您没有使用全屏模式)。
//甚至您无法改变窗口的大小时(例如您在全屏模式下)，它至少仍将运行一次--在程序开始时设置我们的透视图。
//OpenGL场景的尺寸将被设置成它显示时所在窗口的大小。
void MyGLWidget::resizeGL(int w, int h)
{
    if(h == 0)// 防止被零除
    {
        h = 1;// 将高设为1
    }
    glViewport(0, 0, w, h); //重置当前的视口
    //下面几行为透视图设置屏幕。意味着越远的东西看起来越小。这么做创建了一个现实外观的场景。
    //此处透视按照基于窗口宽度和高度的45度视角来计算。0.1f，100.0f是我们在场景中所能绘制深度的起点和终点。
    //glMatrixMode(GL_PROJECTION)指明接下来的两行代码将影响projection matrix(投影矩阵)。
    //投影矩阵负责为我们的场景增加透视。 glLoadIdentity()近似于重置。它将所选的矩阵状态恢复成其原始状态。
    //调用glLoadIdentity()之后我们为场景设置透视图。
    //glMatrixMode(GL_MODELVIEW)指明任何新的变换将会影响 modelview matrix(模型观察矩阵)。
    //模型观察矩阵中存放了我们的物体讯息。最后我们重置模型观察矩阵。如果您还不能理解这些术语的含义，请别着急。
    //在以后的教程里，我会向大家解释。只要知道如果您想获得一个精彩的透视场景的话，必须这么做。
    glMatrixMode(GL_PROJECTION);// 选择投影矩阵
    glLoadIdentity();// 重置投影矩阵
    //设置视口的大小
    gluPerspective(45.0f,(GLfloat)w/(GLfloat)h,0.1f,100.0f);

    glMatrixMode(GL_MODELVIEW);	//选择模型观察矩阵
    glLoadIdentity(); // 重置模型观察矩阵
}

//接下的代码段中，我们将对OpenGL进行所有的设置。我们将设置清除屏幕所用的颜色，打开深度缓存，启用smooth shading(阴影平滑)，等等。
//这个例程直到OpenGL窗口创建之后才会被调用。此过程将有返回值。但我们此处的初始化没那么复杂，现在还用不着担心这个返回值。
void MyGLWidget::initializeGL()
{
    //下一行启用smooth shading(阴影平滑)。阴影平滑通过多边形精细的混合色彩，并对外部光进行平滑。
    //我将在另一个教程中更详细的解释阴影平滑。
    glShadeModel(GL_SMOOTH); // 启用阴影平滑

    //下一行设置清除屏幕时所用的颜色。如果您对色彩的工作原理不清楚的话，我快速解释一下。
    //色彩值的范围从0.0f到1.0f。0.0f代表最黑的情况，1.0f就是最亮的情况。
    //glClearColor 后的第一个参数是Red Intensity(红色分量),第二个是绿色，第三个是蓝色。
    //最大值也是1.0f，代表特定颜色分量的最亮情况。最后一个参数是Alpha值。当它用来清除屏幕的时候，我们不用关心第四个数字。
    //现在让它为0.0f。我会用另一个教程来解释这个参数。
    //通过混合三种原色(红、绿、蓝)，您可以得到不同的色彩。希望您在学校里学过这些。
    //因此，当您使用glClearColor(0.0f,0.0f,1.0f,0.0f)，您将用亮蓝色来清除屏幕。
    //如果您用 glClearColor(0.5f,0.0f,0.0f,0.0f)的话，您将使用中红色来清除屏幕。不是最亮(1.0f)，也不是最暗 (0.0f)。
    //要得到白色背景，您应该将所有的颜色设成最亮(1.0f)。要黑色背景的话，您该将所有的颜色设为最暗(0.0f)。
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // 黑色背景

    //接下来的三行必须做的是关于depth buffer(深度缓存)的。将深度缓存设想为屏幕后面的层。
    //深度缓存不断的对物体进入屏幕内部有多深进行跟踪。
    //我们本节的程序其实没有真正使用深度缓存，但几乎所有在屏幕上显示3D场景OpenGL程序都使用深度缓存。
    //它的排序决定那个物体先画。这样您就不会将一个圆形后面的正方形画到圆形上来。深度缓存是OpenGL十分重要的部分。
    glClearDepth(1.0f);	// 设置深度缓存
    glEnable(GL_DEPTH_TEST); // 启用深度测试
    glDepthFunc(GL_LEQUAL);	// 所作深度测试的类型

    //接着告诉OpenGL我们希望进行最好的透视修正。这会十分轻微的影响性能。但使得透视图看起来好一点。
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// 告诉系统对透视进行修正

    QString fontPath = QApplication::applicationDirPath() + "/Test.ttf";
    m_our_font.init(fontPath.toLatin1().data(), 24);
}

//下一段包括了所有的绘图代码。任何您所想在屏幕上显示的东东都将在此段代码中出现。
//以后的每个教程中我都会在例程的此处增加新的代码。如果您对OpenGL已经有所了解的话，您可以在glLoadIdentity()调用之后，
//试着添加一些OpenGL代码来创建基本的形。
//如果您是OpenGL新手，等着我的下个教程。目前我们所作的全部就是将屏幕清除成我们前面所决定的颜色，清除深度缓存并且重置场景。
//我们仍没有绘制任何东东。
void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f,0.0f,-1.0f);

    // 红色文字
    glColor3ub(0xff,0,0);
    glPushMatrix();
    glLoadIdentity();
    glRotatef(m_cnt1,0,0,1);
    glScalef(1,.8+.3*qCos(m_cnt1/5),1);
    glTranslatef(-180,0,0);
    //绘制freetype文字
    freetype::print(m_our_font, 320, 200, "Active FreeType Text - %7.2f", m_cnt1);
    glPopMatrix();
}

void MyGLWidget::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_F2:
        {
            m_show_full_screen = !m_show_full_screen;
            if(m_show_full_screen)
            {
                showFullScreen();
            }
            else
            {
                showNormal();
            }
            updateGL();
            break;
        }
        case Qt::Key_Escape:
        {
            qApp->exit();
            break;
        }
    }
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    m_cnt1+=0.051f;
    updateGL();
    QGLWidget::timerEvent(event);
}

//在这一课中，我已试着尽量详细解释一切。每一步都与设置有关，并创建了一个全屏OpenGL程序。
//当您按下ESC键程序就会退出，并监视窗口是否激活。
//如果您有什么意见或建议请给我EMAIL。如果您认为有什么不对或可以改进，请告诉我。
//我想做最好的OpenGL教程并对您的反馈感兴趣。
