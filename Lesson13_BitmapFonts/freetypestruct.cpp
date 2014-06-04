#include "freetypestruct.h"

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
            va_start(ap, fmt);// 分析可变参数
            vsprintf(text, fmt, ap);// 把参数值写入字符串
            va_end(ap);// 结束分析
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
