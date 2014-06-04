#ifndef TGALOADER_H
#define TGALOADER_H

#include <GL/glu.h>
#include <QFile>
#include <QObject>
#include <QDebug>

typedef	struct
{
    GLubyte	*imageData;// 控制整个图像的颜色值
    GLuint	bpp;// 控制单位像素的bit数
    GLuint	width;// 整个图像的宽度
    GLuint	height;// 整个图像的高度
    GLuint	texID;// 使用glBindTexture所需的纹理ID.
    GLuint	type;// 描述存储在*ImageData中的数据(GL_RGB Or GL_RGBA)
} Texture;

typedef struct
{
    GLubyte Header[12];// 文件头决定文件类型
} TGAHeader;

typedef struct
{
    GLubyte		header[6];// 控制前6个字节
    GLuint		bytesPerPixel;// 每像素的字节数 (3 或 4)
    GLuint		imageSize;// 控制存储图像所需的内存空间
    GLuint		type;// 图像类型 GL_RGB 或 GL_RGBA
    GLuint		Height;// 图像的高度
    GLuint		Width;// 图像宽度
    GLuint		Bpp;// 每像素的比特数 (24 或 32)
} TGA;

bool LoadTGA(Texture * texture, const QString &filename);
bool LoadUncompressedTGA(Texture *, QFile *);	// Load an Uncompressed file
bool LoadCompressedTGA(Texture *, QFile *);		// Load a Compressed file

#endif // TGALOADER_H
