#include "tgaloader.h"

//我们需要定义一对文件头，那样我们能够告诉程序什么类型的文件头处于有效的图像上。如果是未压缩的TGA图像，
//前12字节将会是0 0 2 0 0 0 0 0 0 0 0 0，如果是RLE压缩的，则是0 0 10 0 0 0 0 0 0 0 0 0。
//这两个值允许我们检查正在读取的文件是否有效。
GLubyte uTGAcompare[12] = {0,0,2, 0,0,0,0,0,0,0,0,0};	//未压缩的TGA头
GLubyte cTGAcompare[12] = {0,0,10,0,0,0,0,0,0,0,0,0};	// 压缩的TGA头

// 读取一个TGA文件!
bool LoadTGA(Texture * texture, const QString &filename)// Load a TGA file
{
    TGAHeader tgaheader;// 用来存储我们的文件头
    QFile *fTGA = new QFile(filename, NULL);
    //接下来的几行检查指定的文件是否已经正确地打开。
    if(!fTGA->open(QIODevice::ReadOnly))// 以读模式打开文件
    {
        qDebug() << QObject::tr("Could not open texture file.");
        return false;
    }
    //下一步，我们尝试读取文件的首12个字节的内容并且将它们存储在我们的TGAHeader结构中，这样，我们得以检查文件类型。
    //如果fread失败，则关闭文件，显示一个错误，并且函数返回false。
    if(fTGA->read((char*)&tgaheader, sizeof(TGAHeader)) == 0)
    {
        qDebug() << QObject::tr("Could not read file header.");
        fTGA->close();
        delete fTGA;
        return false;
    }
    //接着，通过我们用辛苦编的程序刚读取的头，我们继续尝试确定文件类型。
    //这可以告诉我们它是压缩的、未压缩甚至是错误的文件类型。为了达到这个目的，我们将会使用memcmp(…)函数。
    if(memcmp(uTGAcompare, &tgaheader, sizeof(tgaheader)) == 0)
    {
        //如果文件头附合未压缩的文件头格式
        // 读取未压缩的TGA文件
        LoadUncompressedTGA(texture, fTGA);
    }
    else if(memcmp(cTGAcompare, &tgaheader, sizeof(tgaheader)) == 0)
    {
        // 如果文件头附合压缩的文件头格式
        // 读取压缩的TGA格式
        LoadCompressedTGA(texture, fTGA);
    }
    else
    {
        // 如果任一个都不符合
        qDebug() << QObject::tr("TGA file be type 2 or type 10");
        fTGA->close();
        delete fTGA;
        return false;
    }
    delete fTGA;
    return true;
}

// 读取一个未压缩的文件
bool LoadUncompressedTGA(Texture * texture, QFile * fTGA)
{
    TGA tga;// 用来存储文件信息
    // 尝试继续读取6个字节的内容
    if(fTGA->read((char*)&tga.header, sizeof(tga.header)) == 0)
    {
        qDebug() << QObject::tr("Could not read info header.");
        fTGA->close();
        return false;
    }

    //现在我们有了计算图像的高度、宽度和BPP的全部信息。我们在纹理和本地结构中都将存储它。
    texture->width  = tga.header[1] * 256 + tga.header[0];	// 计算高度
    texture->height = tga.header[3] * 256 + tga.header[2];	// 计算宽度
    texture->bpp	= tga.header[4];// 计算BPP
    tga.Width		= texture->width;// 拷贝Width到本地结构中去
    tga.Height		= texture->height;// 拷贝Height到本地结构中去
    tga.Bpp			= texture->bpp;// 拷贝Bpp到本地结构中去

    //现在，我们需要确认高度和宽度至少为1个像素，并且bpp是24或32。如果这些值中的任何一个超出了
    //它们的界限，我们将再一次显示一个错误，关闭文件，并且离开此函数。
    // 确认所有的信息都是有效的
    if((texture->width <= 0) ||
            (texture->height <= 0) ||
            ((texture->bpp != 24) &&
             (texture->bpp !=32)))
    {
        qDebug() << QObject::tr("Invalid texture information.");
        fTGA->close();
        return false;														// Return failed
    }

    //接下来我们设置图像的类型。24 bit图像是GL_RGB，32 bit 图像是GL_RGBA
    if(texture->bpp == 24)// 24bit图像吗？
    {
        texture->type	= GL_RGB;//如果是，设置类型为GL_RGB
    }
    else// 如果不是24bit,则必是32bit
    {
        texture->type	= GL_RGBA;//这样设置类型为GL_RGBA
    }
    //现在我们计算每像素的字节数和总共的图像数据
    tga.bytesPerPixel	= (tga.Bpp / 8);// 计算BPP
    tga.imageSize		= (tga.bytesPerPixel * tga.Width * tga.Height);// 计算存储图像所需的内存
    //我们需要一些空间去存储整个图像数据，因此我们将要使用malloc分配正确的内存数量
    //然后我们确认内存已经分配，并且它不是NULL。如果出现了错误，则运行错误处理代码。
    texture->imageData	= (GLubyte *)malloc(tga.imageSize);	// 分配内存
    if(texture->imageData == NULL)// 确认已经分配成功
    {
        qDebug() << QObject::tr("Could not allocate memory for image.");
        fTGA->close();
        return false;														// Return failed
    }

    //这里我们尝试读取所有的图像数据。如果不能，我们将再次触发错误处理代码。
    // 尝试读取所有图像数据
    if(fTGA->read((char*)texture->imageData, tga.imageSize) != tga.imageSize)
    {
        qDebug() << QObject::tr("Could not read image data.");
        fTGA->close();
        return false;
    }

    //TGA文件用逆OpenGL需求顺序的方式存储图像，因此我们必须将格式从BGR到RGB。为了达到这一点，
    //我们交换每个像素的第一个和第三个字节的内容。
    //Steve Thomas补充：我已经编写了能稍微更快速读取TGA文件的代码。它涉及到仅用3个二进制操作将BGR转换到RGB的方法。
    //然后我们关闭文件，并且成功退出函数。
    for(GLuint cswap = 0; cswap < (int)tga.imageSize; cswap += tga.bytesPerPixel)
    {
        // 第一字节 XOR第三字节XOR 第一字节 XOR 第三字节
        texture->imageData[cswap] ^= texture->imageData[cswap+2] ^=
        texture->imageData[cswap] ^= texture->imageData[cswap+2];
    }

    fTGA->close();// 关闭文件
    return true;
}

// 读取一个压缩的文件
bool LoadCompressedTGA(Texture * texture, QFile *fTGA)
{
    TGA tga;
    if(fTGA->read((char*)&tga.header, sizeof(tga.header)) == 0)
    {
        qDebug() << QObject::tr("Could not read info header.");
        fTGA->close();
        return false;
    }

    texture->width  = tga.header[1] * 256 + tga.header[0];
    texture->height = tga.header[3] * 256 + tga.header[2];
    texture->bpp	= tga.header[4];
    tga.Width		= texture->width;
    tga.Height		= texture->height;
    tga.Bpp			= texture->bpp;

    if((texture->width <= 0) ||
            (texture->height <= 0) ||
            ((texture->bpp != 24) &&
             (texture->bpp !=32)))
    {
        qDebug() << QObject::tr("Invalid texture information.");
        fTGA->close();
        return false;
    }

    if(texture->bpp == 24)
    {
        texture->type	= GL_RGB;
    }
    else
    {
        texture->type	= GL_RGBA;
    }

    tga.bytesPerPixel	= (tga.Bpp / 8);
    tga.imageSize		= (tga.bytesPerPixel * tga.Width * tga.Height);
    //现在我们需要分配存储图像所需的空间，这是为我们解压缩之后准备的，我们将使用malloc。
    //如果内存分配失败，运行错误处理代码，并且返回false
    // 分配存储图像所需的内存空间
    texture->imageData	= (GLubyte *)malloc(tga.imageSize);

    if(texture->imageData == NULL)// 如果不能分配内存
    {
        qDebug() << QObject::tr("Could not allocate memory for image");
        fTGA->close();
        return false;
    }

    //下一步我们需要决定组成图像的像素数。我们将它存储在变量“pixelcount”中。
    //我们也需要存储当前所处的像素，以及我们正在写入的图像数据的字节，这样避免溢出写入过多的旧数据。
    //我们将要分配足够的内存来存储一个像素。
    GLuint pixelcount	= tga.Height * tga.Width;// 图像中的像素数
    GLuint currentpixel	= 0;// 当前正在读取的像素
    GLuint currentbyte	= 0;// 当前正在向图像中写入的像素
    GLubyte * colorbuffer = (GLubyte *)malloc(tga.bytesPerPixel);// 一个像素的存储空间

    //接下来我们将要进行一个大循环。
    //让我们将它分解为更多可管理的块。
    //首先我们声明一个变量来存储“块”头。块头指示接下来的段是RLE还是RAW，它的长度是多少。
    //如果一字节头小于等于127，则它是一个RAW头。头的值是颜色数，是负数，在我们处理其它头字节之前，
    //我们先读取它并且拷贝到内存中。这样我们将我们得到的值加1，然后读取大量像素并且将它们拷贝到ImageData中，
    //就像我们处理未压缩型图像一样。如果头大于127，那么它是下一个像素值随后将要重复的次数。
    //要获取实际重复的数量，我们将它减去127以除去1bit的的头标示符。然后我们读取下一个像素并且依照上述次数
    //连续拷贝它到内存中。
    do
    {
        GLubyte chunkheader = 0;// 存储Id块值的变量
        if(fTGA->read((char*)&chunkheader, sizeof(GLubyte)) == 0)// 尝试读取块的头
        {
            qDebug() << QObject::tr("Could not read RLE header");
            fTGA->close();
            if(texture->imageData != NULL)
            {
                free(texture->imageData);
            }
            return false;
        }
        //接下来我们将要看看它是否是RAW头。如果是，我们需要将此变量的值加1以获取紧随头之后的像素总数。
        if(chunkheader < 128)// 如果是RAW块
        {
            chunkheader++;// 变量值加1以获取RAW像素的总数
            //我们开启另一个循环读取所有的颜色信息。它将会循环块头中指定的次数，并且每次循环读取和存储一个像素。
            //首先，我们读取并检验像素数据。单个像素的数据将被存储在colorbuffer变量中。
            //然后我们将检查它是否为RAW头。如果是，我们需要添加一个到变量之中以获取头之后的像素总数。
            for(short counter = 0; counter < chunkheader; counter++)
            {
                // 尝试读取一个像素
                if(fTGA->read((char*)colorbuffer, tga.bytesPerPixel) != tga.bytesPerPixel)
                {
                    qDebug() << QObject::tr("Could not read image data");
                    fTGA->close();
                    if(colorbuffer != NULL)
                    {
                        free(colorbuffer);
                    }
                    if(texture->imageData != NULL)
                    {
                        free(texture->imageData);
                    }
                    return false;
                }

                //我们循环中的下一步将要获取存储在colorbuffer中的颜色值并且将其写入稍后将要使用的imageData
                //变量中。在这个过程中，数据格式将会由BGR翻转为RGB或由BGRA转换为RGBA，
                //具体情况取决于每像素的比特数。当我们完成任务后我们增加当前的字节和当前的像素计数器。
                texture->imageData[currentbyte		] = colorbuffer[2];// “R”字节
                texture->imageData[currentbyte + 1	] = colorbuffer[1];// “G”字节
                texture->imageData[currentbyte + 2	] = colorbuffer[0];// “B”字节

                if(tga.bytesPerPixel == 4)// 如果是32位图像...
                {
                    texture->imageData[currentbyte + 3] = colorbuffer[3];// “A”字节
                }

                // 依据每像素的字节数增加字节计数器
                currentbyte += tga.bytesPerPixel;
                currentpixel++;	// 像素计数器加1

                if(currentpixel > pixelcount)
                {
                    qDebug() << QObject::tr("Toll many pixels read.");
                    fTGA->close();
                    if(colorbuffer != NULL)
                    {
                        free(colorbuffer);
                    }
                    if(texture->imageData != NULL)
                    {
                        free(texture->imageData);
                    }
                    return false;
                }
            }
        }
        else
        {
            //下一段处理描述RLE段的“块”头。首先我们将chunkheader减去127来得到获取下一个颜色重复的次数。
            // 如果是RLE头
            chunkheader -= 127;//  减去127获得ID Bit的Rid
            // 读取下一个像素
            if(fTGA->read((char*)colorbuffer, tga.bytesPerPixel) != tga.bytesPerPixel)
            {
                qDebug() << QObject::tr("Could not read from file.");
                fTGA->close();
                if(colorbuffer != NULL)
                {
                    free(colorbuffer);
                }
                if(texture->imageData != NULL)
                {
                    free(texture->imageData);
                }
                return false;
            }
            //接下来，我们开始循环拷贝我们多次读到内存中的像素，这由RLE头中的值规定。
            //然后，我们将颜色值拷贝到图像数据中，预处理R和B的值交换。
            //随后，我们增加当前的字节数、当前像素，这样我们再次写入值时可以处在正确的位置。
            for(short counter = 0; counter < chunkheader; counter++)
            {
                texture->imageData[currentbyte		] = colorbuffer[2];// 拷贝“R”字节
                texture->imageData[currentbyte + 1	] = colorbuffer[1];// 拷贝“G”字节
                texture->imageData[currentbyte + 2	] = colorbuffer[0];// 拷贝“B”字节

                if(tga.bytesPerPixel == 4)// 如果是32位图像
                {
                    texture->imageData[currentbyte + 3] = colorbuffer[3];// 拷贝“A”字节
                }

                currentbyte += tga.bytesPerPixel;// 增加字节计数器
                currentpixel++;	// 增加字节计数器

                if(currentpixel > pixelcount)
                {
                    qDebug() << QObject::tr("Too many pixels read");
                    fTGA->close();
                    if(colorbuffer != NULL)
                    {
                        free(colorbuffer);
                    }
                    if(texture->imageData != NULL)
                    {
                        free(texture->imageData);
                    }
                    return false;
                }
            }
        }
        //只要仍剩有像素要读取，我们将会继续主循环。
        //最后，我们关闭文件并返回成功。
    }
    while(currentpixel < pixelcount);// 是否有更多的像素要读取？开始循环直到最后
    fTGA->close();
    return true;
}
