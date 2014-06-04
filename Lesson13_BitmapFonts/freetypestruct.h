#ifndef FREETYPESTRUCT_H
#define FREETYPESTRUCT_H

#include <GL/glu.h>
#include <qdebug.h>
#include <freetype2/ft2build.h>
#include <freetype2/freetype.h>
#include <freetype2/ftglyph.h>

//我们将把每个字符需要的信息封装在一个结构中，这样就像使用WGL字体一样，我们可以分别控制每个字符的显示状态。
namespace freetype
{
    struct font_data//这个结构保存字体信息
    {
        float h; // 字体的高度
        GLuint *textures; // 使用的纹理
        GLuint list_base; // 显示列表的值

        // 初始化结构
        void init(const char *fname, unsigned int h);
        // 清楚所有的资源
        void clean();
    };
    //最后一件事是定义我们输出字符串的原形:
    // 把字符输出到屏幕
    void print(const font_data &ft_font, float x, float y, const char *fmt, ...);
}


#endif // FREETYPESTRUCT_H
