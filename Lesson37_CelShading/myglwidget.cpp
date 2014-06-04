#include "myglwidget.h"

//卡通映射:
//什么是卡通了，一个轮廓加上少量的几种颜色。使用一维纹理映射，你也可以实现这种效果。

//看到人们仍然e-mail我请求在文章中使用我方才在GameDev.net上写的源代码，还看到文章的第二版（在那每一个API附带源码）
//不是在中途完成之前连贯的结束。我已经把这篇指南一并出租给了NeHe（这实际上是写文章的最初意图）因此你们所有的OpenGL领袖可以玩转它。
//对模型的选择表示抱歉，但是我最近一直在玩Quake 2。
//注释：这篇文章的源代码可以在这里找到：
//http://www.gamedev.net/reference/programming/features/celshading.
//这篇指南实际上并不解释原理，仅仅解释代码。在上面的连接中可以发现为什么它能工作。
//现在不断地大声抱怨STOP E-MAILING ME REQUESTS FOR SOURCE CODE!!!!
//首先，我们需要包含一些额外的头文件。第一个（math.h）我们可以使用sqrtf (square root)函数，第二个用来访问文件。

//一些基本的数学函数而已。DotProduct计算2个向量或平面之间的角，Magnitude函数计算向量的长度，Normalize函数缩放向量到一个单位长度。

//计算两个向量之间的角度
inline float DotProduct (VECTOR &V1, VECTOR &V2)
{
    return V1.X * V2.X + V1.Y * V2.Y + V1.Z * V2.Z;
}

// 计算向量的长度
inline float Magnitude (VECTOR &V)
{
    return sqrtf(V.X * V.X + V.Y * V.Y + V.Z * V.Z);
}

// 创建一个单位长度的向量
void Normalize(VECTOR &V)
{
    float M = Magnitude(V);
    // 确保我们没有被0隔开
    if (M != 0.0f)
    {
        V.X /= M;
        V.Y /= M;
        V.Z /= M;
    }
}

//这个函数利用给定的矩阵旋转一个向量。请注意它仅旋转这个向量——与向量的位置相比它算不了什么。
//它用来当旋转法线确保当我们在计算灯光时它们停留在正确的方向上。
// 利用提供的矩阵旋转一个向量
void RotateVector (MATRIX &M, VECTOR &V, VECTOR &D)
{
    D.X = (M.Data[0] * V.X) + (M.Data[4] * V.Y) + (M.Data[8]  * V.Z);
    D.Y = (M.Data[1] * V.X) + (M.Data[5] * V.Y) + (M.Data[9]  * V.Z);
    D.Z = (M.Data[2] * V.X) + (M.Data[6] * V.Y) + (M.Data[10] * V.Z);
}

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false),
    m_outlineDraw(true), m_outlineSmooth(false),
    m_outlineColor({ 0.0f, 0.0f, 0.0f }), m_outlineWidth(3.0f),
    m_lightRotate(false), m_modelAngle(0.0f), m_modelRotate(false),
    m_polyData(NULL), m_polyNum(0)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
    glDeleteTextures(1, &m_shaderTexture[0]);// 删除阴影纹理
    delete []m_polyData;// 删除多边形数据
}

void MyGLWidget::resizeGL(int w, int h)
{
    glViewport (0, 0, (GLsizei)(w), (GLsizei)(h));// Reset The Current Viewport
    glMatrixMode (GL_PROJECTION);	// Select The Projection Matrix
    glLoadIdentity ();// Reset The Projection Matrix
    gluPerspective (45.0f, (GLfloat)(w)/(GLfloat)(h),1.0f, 100.0f);// Calculate The Aspect Ratio Of The Window
    glMatrixMode (GL_MODELVIEW);// Select The Modelview Matrix
    glLoadIdentity ();
}

void MyGLWidget::initializeGL()
{
    //这3个变量用来装载着色文件。在文本文件中为了单一的线段线段包含了空间，虽然shaderData存储了真实的着色值。
    //你可能奇怪为什么我们的96个值被32个代替了。好了，我们需要转换greyscale 值为RGB以便OpenGL能使用它们。
    //我们仍然可以以greyscale存储这些值，但向上负载纹理时我们至于R，G和B成分仅仅使用同一值。
    char Line[255];	// 255个字符的存储量
    float shaderData[32][3];// 96个着色值的存储量
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// 提高计算精度
    glClearColor (0.7f, 0.7f, 0.7f, 0.0f);// 设置为灰色背景
    glClearDepth (1.0f);// 设置深度缓存值
    glEnable (GL_DEPTH_TEST);// 启用深度测试
    glDepthFunc (GL_LESS);// 设置深度比较函数
    //当绘制线条时，我们想要确保很平滑。初值被关闭，但是按“2”键，它可以被toggled on/off。
    glShadeModel (GL_SMOOTH);// 使用色彩阴影平滑
    glDisable (GL_LINE_SMOOTH);	// 线条平滑初始化不可用
    glEnable (GL_CULL_FACE);// 启用剔除多边形功能
    glDisable (GL_LIGHTING);// 使 OpenGL 灯光不可用
    //这里是我们装载阴影文件的地方。它简单地以32个浮点值ASCII码存放（为了轻松修改），每一个在separate线上。
    QFile in(":/Data/Shader.txt");
    // 打开阴影文件
    if(in.open(QIODevice::ReadOnly))
    {
        // 循环32次
        for (int i = 0; i < 32; i++)
        {
            if(in.atEnd())
            {
                break;
            }
            // 获得当前线条
            in.readLine(Line, 255);
            //这里我们转换 greyscale 值为 RGB, 正象上面所描述的。
            // 从头到尾复制这个值
            shaderData[i][0] = shaderData[i][1] = shaderData[i][2] = float(atof (Line));
        }
        in.close();
    }
    else
    {
        return;
    }
    //现在我们向上装载这个纹理。同样它清楚地规定，不要使用任何一种过滤在纹理上否则它看起来奇怪，至少可以这样说。
    //GL_TEXTURE_1D被使用因为它是值的一维数组。
    glGenTextures (1, &m_shaderTexture[0]);// 获得一个自由的纹理ID
    glBindTexture (GL_TEXTURE_1D, m_shaderTexture[0]);// 绑定这个纹理。 从现在开始它变为一维
    // 使用邻近点过滤
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // 设置纹理
    glTexImage1D (GL_TEXTURE_1D, 0, GL_RGB, 32, 0, GL_RGB , GL_FLOAT, shaderData);
    //现在调整灯光方向。我已经使得它向下指向Z轴正方向，这意味着它将正面碰撞模型
    m_lightAngle.X = 0.0f;
    m_lightAngle.Y = 0.0f;
    m_lightAngle.Z = 1.0f;
    Normalize (m_lightAngle);
    //读取Mesh文件,并返回
    readMesh();
}

//你一直在等待的函数。Draw 函数做每一件事情——计算阴影的值，着色网孔，着色轮廓，等等，这是它作的。
void MyGLWidget::paintGL()
{
    //TmpShade用来存储当前顶点的色度值。所有顶点数据同时被计算，意味着我们只需使用我们能继续使用的单个的变量。
    //TmpMatrix, TmpVector 和 TmpNormal同样被用来计算顶点数据，TmpMatrix在函数开始时被调整一次并一直保持到Draw函数被再次调用。
    //TmpVector 和 TmpNormal则相反，当另一个顶点被处理时改变。
    float TmpShade;// 临时色度值
    MATRIX TmpMatrix;// 临时 MATRIX 结构体
    VECTOR TmpVector, TmpNormal;// 临时 VECTOR结构体
    //清除缓冲区矩阵数据
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//  清除缓冲区
    glLoadIdentity ();// 重置矩阵
    //首先检查我们是否想拥有平滑的轮廓。如果是，我们就打开anti-alaising 。否则把它关闭。简单！
    if (m_outlineSmooth)// 检查我们是否想要 Anti-Aliased 线条
    {
        glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);// 启用它们
        glEnable (GL_LINE_SMOOTH);
    }
    else
    {
        glDisable (GL_LINE_SMOOTH);// 否则不启用
    }
    //然后我们设置视口。我们反向移动摄象机2个单元，之后以一定角度旋转模型。注：由于我们首先移动摄象机，这个模型将在现场旋转。
    //如果我们以另一种方法做，模型将绕摄象机旋转。我们之后从OpenGL中取最新创建的矩阵并把它存储在 TmpMatrix。
    glTranslatef (0.0f, 0.0f, -2.0f);// 移入屏幕两个单位
    glRotatef (m_modelAngle, 0.0f, 1.0f, 0.0f);// 绕Y轴旋转这个模型

    glGetFloatv (GL_MODELVIEW_MATRIX, TmpMatrix.Data);	// 获得产生的矩阵
    //戏法开始了。首先我们启用一维纹理，然后启用着色纹理。这被OpenGL用来当作一个look-up表格。
    //我们之后调整模型的颜色（白色）我选择白色是因为它亮度高并且描影法比其它颜色好。我建议你不要使用黑色：）
    // 卡通渲染代码
    glEnable (GL_TEXTURE_1D);// 启用一维纹理
    glBindTexture (GL_TEXTURE_1D, m_shaderTexture[0]);// 锁定我们的纹理

    glColor3f (1.0f, 1.0f, 1.0f);// 调整模型的颜色
    //现在我们开始绘制那些三角形。尽管我们看到在数组中的每一个多边形，然后旋转它的每一个顶点。第一步是拷贝法线信息到一个临时的结构中。
    //因此我们能旋转法线，但仍然保留原来保存的值（没有精确降级）。
    glBegin (GL_TRIANGLES);// 告诉 OpenGL 我们即将绘制三角形
        for (int i = 0; i < m_polyNum; i++)// 从头到尾循环每一个多边形
        {
            for (int j = 0; j < 3; j++)// 从头到尾循环每一个顶点
            {
                TmpNormal.X = m_polyData[i].Verts[j].Nor.X;// 用当前顶点的法线值填充TmpNormal结构
                TmpNormal.Y = m_polyData[i].Verts[j].Nor.Y;
                TmpNormal.Z = m_polyData[i].Verts[j].Nor.Z;
                //第二，我们通过初期从OpenGL中攫取的矩阵来旋转这个法线。我们之后规格化因此它并不全部变为螺旋形。
                // 通过矩阵旋转
                RotateVector(TmpMatrix, TmpNormal, TmpVector);
                Normalize (TmpVector);// 规格化这个新法线
                //第三，我们获得那个旋转的法线的点积灯光方向（称为lightAngle，因为我忘了从我的旧的light类中改变它）。
                //我们之后约束这个值在0——1的范围。（从-1到+1）
                // 计算色度值
                TmpShade = DotProduct (TmpVector, m_lightAngle);
                if (TmpShade < 0.0f)
                {
                    TmpShade = 0.0f;	// 如果负值约束这个值到0
                }
                //第四，对于OpenGL我们象忽略纹理坐标一样忽略这个值。阴影纹理与一个查找表一样来表现（色度值正成为指数），
                //这是（我认为）为什么1D纹理被创造主要原因。对于OpenGL我们之后忽略这个顶点位置，并不断重复，重复。
                //至此我认为你已经抓到了概念。
                glTexCoord1f (TmpShade);	// 规定纹理的纵坐标当作这个色度值
                // 送顶点
                glVertex3fv (&m_polyData[i].Verts[j].Pos.X);
            }
        }
    glEnd ();// 告诉OpenGL 完成绘制
    glDisable (GL_TEXTURE_1D);// 1D 纹理不可用
    //现在我们转移到轮廓之上。一个轮廓能以“它的相邻的边，一边为可见，另一边为不可见”定义。在OpenGL中，
    //这是深度测试被规定小于或等于(GL_LEQUAL)当前值的地方，并且就在那时所有前面的面被精选。我们同样也要混合线条，以使它看起来不错：）
    //那么，我们使混合可用并规定混合模式。我们告诉OpenGL与着色线条一样着色backfacing多边形，并且规定这些线条的宽度。
    //我们精选所有前面多边形，并规定测试深度小于或等于当前的Z值。在这个线条的的颜色被规定后，我们从头到尾循环每一个多边形，
    //绘制它的顶点。我们仅需忽略顶点位置，而不是法线或着色值因为我们需要的仅仅是轮廓。
    // 轮廓代码
    // 检查看是否我们需要绘制轮廓
    if (m_outlineDraw)
    {
        glEnable (GL_BLEND);// 使混合可用
        // 调整混合模式
        glBlendFunc (GL_SRC_ALPHA ,GL_ONE_MINUS_SRC_ALPHA);
        glPolygonMode (GL_BACK, GL_LINE);// 绘制轮廓线
        glLineWidth (m_outlineWidth);// 调整线宽
        glCullFace (GL_FRONT);// 剔出前面的多边形
        glDepthFunc (GL_LEQUAL);// 改变深度模式
        glColor3fv (&m_outlineColor[0]);// 规定轮廓颜色
        glBegin (GL_TRIANGLES);// 告诉OpenGL我们想要绘制什么
            // 从头到尾循环每一个多边形
            for (int i = 0; i < m_polyNum; i++)
            {
                // 从头到尾循环每一个顶点
                for (int j = 0; j < 3; j++)
                {
                    // 送顶点
                    glVertex3fv (&m_polyData[i].Verts[j].Pos.X);
                }
            }
        glEnd ();// 告诉 OpenGL我们已经完成
        //这样以后，我们就把它规定为以前的状态，然后退出
        glDepthFunc (GL_LESS);// 重置深度测试模式
        glCullFace (GL_BACK);// 重置剔出背面多边形
        glPolygonMode (GL_BACK, GL_FILL);// 重置背面多边形绘制方式
        glDisable (GL_BLEND);				//  混合不可用
    }
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
        case Qt::Key_Space:
        {
            m_modelRotate = !m_modelRotate;// 锁定模型旋转开/关
            break;
        }
        case Qt::Key_1:
        {
            m_outlineDraw = !m_outlineDraw;// 切换是否绘制轮廓线
            break;
        }
        case Qt::Key_2:
        {
            m_outlineSmooth = !m_outlineSmooth;// 切换是否使用反走样
            break;
        }
        case Qt::Key_Up:
        {
            m_outlineWidth++;// 上键增加线的宽度
            break;
        }
        case Qt::Key_Down:
        {
            m_outlineWidth--;// 下减少线的宽度
            break;
        }
    }
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    // 是否旋转
    if (m_modelRotate)
    {
        m_modelAngle += (float) (15) / 10.0f;// 更新旋转角度
    }
    updateGL();
    QGLWidget::timerEvent(event);
}

//这是得到的再简单不过的模型文件格式。 最初的少量字节存储在场景中的多边形的编号，文件的其余是tagPOLYGON结构体的一个数组。
//正因如此，数据在没有任何需要去分类到详细的顺序的情况下被读出。
// 读“model.txt” 文件
bool MyGLWidget::readMesh()
{
    QFile in(":/Data/Model.txt");
    if(!in.open(QIODevice::ReadOnly))
    {
        return false;
    }

    in.read((char*)&m_polyNum, sizeof(int));// 读文件头，多边形的个数
    m_polyData = new POLYGON[m_polyNum];// 分配内存
    in.read((char*)m_polyData, sizeof(POLYGON)*m_polyNum);//把所有多边形的数据读入
    in.close();
    return true;// 工作完成
}


//在这一课中，我已试着尽量详细解释一切。每一步都与设置有关，并创建了一个全屏OpenGL程序。
//当您按下ESC键程序就会退出，并监视窗口是否激活。
//如果您有什么意见或建议请给我EMAIL。如果您认为有什么不对或可以改进，请告诉我。
//我想做最好的OpenGL教程并对您的反馈感兴趣。
