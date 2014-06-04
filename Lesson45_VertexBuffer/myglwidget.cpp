#include "myglwidget.h"

//顶点缓存
//你想更快地绘制么？直接操作显卡吧，这可是当前的图形技术，不要犹豫，我带你入门。接下来，你自己向前走吧。

//速度是3D程序中最重要的指标，你必须限制绘制的多边形的个数，或者提高显卡绘制多边形的效率。
//显卡最近增加了一个新的扩展，叫做顶点缓存VS，它直接把顶点放置在显卡中的高速缓存中，极大的增加了绘制速度。
//在这个教程里，我们会加载一个高度图，使用顶点数组高效的把网格数据发送到OpenGL里，并使用VBO扩展把顶点数据放入高效的显存里。
//现在让我们开始吧，我们先来定义一些程序参数。

#define MESH_RESOLUTION 4.0f							// 每个顶点使用的像素
#define MESH_HEIGHTSCALE 1.0f							// 高度的缩放比例
//#define NO_VBOS                                         // 如果定义将不使用VBO扩展
//定义VBO扩展它们在glext.h头文件中被定义
#define GL_ARRAY_BUFFER_ARB 0x8892
#define GL_STATIC_DRAW_ARB 0x88E4
typedef void (APIENTRY * PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRY * PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRY * PFNGLBUFFERDATAARBPROC) (GLenum target, int size, const GLvoid *data, GLenum usage);
// VBO 扩展函数的指针
PFNGLGENBUFFERSARBPROC glGenBuffersARB = NULL; // 创建缓存名称
PFNGLBINDBUFFERARBPROC glBindBufferARB = NULL; // 绑定缓存
PFNGLBUFFERDATAARBPROC glBufferDataARB = NULL; // 绑定缓存数据
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = NULL; // 删除缓存

// 返回是否支持指定的扩展
bool isExtensionSupported( char* szTargetExtension )
{
    const unsigned char *pszExtensions = NULL;
    const unsigned char *pszStart;
    unsigned char *pszWhere, *pszTerminator;
    pszWhere = (unsigned char *) strchr( szTargetExtension, ' ' );
    if( pszWhere || *szTargetExtension == '\0' )
        return false;
    // 返回扩展字符串
    pszExtensions = glGetString( GL_EXTENSIONS );
    // 在扩展字符串中搜索
    pszStart = pszExtensions;
    for(;;)
    {
        pszWhere = (unsigned char *) strstr( (const char *) pszStart, szTargetExtension );
        if( !pszWhere )
            break;
        pszTerminator = pszWhere + strlen( szTargetExtension );
        if( pszWhere == pszStart || *( pszWhere - 1 ) == ' ' )
            if( *pszTerminator == ' ' || *pszTerminator == '\0' )
                //如果存在返回True
                return true;
        pszStart = pszTerminator;
    }
    return false;
}

bool g_fVBOSupported = false;							// 是否支持顶点缓存对象
MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false),
    m_pMesh(NULL), m_flYRot(0.0), m_nFPS(0), m_nFrames(0),
    m_dwLastFPS(0), m_timerCounter(0)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
    if( m_pMesh )												// Deallocate Our Mesh Data
        delete m_pMesh;											// And Delete VBOs
    m_pMesh = NULL;
}

//下面的代码的作用是重新设置OpenGL场景的大小，而不管窗口的大小是否已经改变(假定您没有使用全屏模式)。
//甚至您无法改变窗口的大小时(例如您在全屏模式下)，它至少仍将运行一次--在程序开始时设置我们的透视图。
//OpenGL场景的尺寸将被设置成它显示时所在窗口的大小。
void MyGLWidget::resizeGL(int w, int h)
{
    glViewport (0, 0, (GLsizei)(w), (GLsizei)(h));				// Reset The Current Viewport
    glMatrixMode (GL_PROJECTION);										// Select The Projection Matrix
    glLoadIdentity ();													// Reset The Projection Matrix
    gluPerspective (45.0f, (GLfloat)(w)/(GLfloat)(h),			// Calculate The Aspect Ratio Of The Window
                    1.0f, 1000.0f);
    glMatrixMode (GL_MODELVIEW);										// Select The Modelview Matrix
    glLoadIdentity ();													// Reset The Modelview Matrix
}

//好了，现在到了初始化的地方了。首先我将分配并载入纹理数据。接着检测是否支持VBO扩展。
//如果支持我们将把函数指针和它对应的函数关联起来，如果不支持将只返回数据。
void MyGLWidget::initializeGL()
{
    // 载入纹理数据
    m_pMesh = new CMesh();
    if( !m_pMesh->loadHeightmap(MESH_HEIGHTSCALE, MESH_RESOLUTION) )
    {
        QMessageBox::critical(this, "Error", "Error Loading Heightmap");
    }
    // 检测是否支持VBO扩展
#ifndef NO_VBOS
    g_fVBOSupported = isExtensionSupported( "GL_ARB_vertex_buffer_object" );
    if( g_fVBOSupported )
    {
        // 获得函数的指针
        glGenBuffersARB = (PFNGLGENBUFFERSARBPROC) wglGetProcAddress("glGenBuffersARB");
        glBindBufferARB = (PFNGLBINDBUFFERARBPROC) wglGetProcAddress("glBindBufferARB");
        glBufferDataARB = (PFNGLBUFFERDATAARBPROC) wglGetProcAddress("glBufferDataARB");
        glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) wglGetProcAddress("glDeleteBuffersARB");
        // 创建VBO对象
        m_pMesh->buildVBOs();
    }
#else
    g_fVBOSupported = false;
#endif
    //设置OpenGL状态
    glClearColor (0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepth (1.0f);
    glDepthFunc (GL_LEQUAL);
    glEnable (GL_DEPTH_TEST);
    glShadeModel (GL_SMOOTH);
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable( GL_TEXTURE_2D );
    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
}

//好了,几乎结束了,我们下面来看看我们的渲染代码.
void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity ();
    // 显示当前的帧率
    if( m_timerCounter - m_dwLastFPS >= 1000 )
    {
        m_dwLastFPS = m_timerCounter;
        m_nFPS = m_nFrames;
        m_nFrames = 0;

        char szTitle[256]={0};
        sprintf( szTitle, "Lesson 45: NeHe & Paul Frazee's VBO Tut - %d Triangles, %d FPS",
                 m_pMesh->m_nVertexCount / 3, m_nFPS );
        if( g_fVBOSupported ) // 是否支持VBO
            strcat( szTitle, ", Using VBOs" );
        else
            strcat( szTitle, ", Not Using VBOs" );
        setWindowTitle(szTitle);// 设置窗口标题
    }
    m_nFrames++;
    // 设置视口
    glTranslatef( 0.0f, -220.0f, 0.0f );
    glRotatef( 10.0f, 1.0f, 0.0f, 0.0f );
    glRotatef( m_flYRot, 0.0f, 1.0f, 0.0f );

    // 使用顶点，纹理坐标数组
    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    //为了使用VBO，你必须告诉OpenGL内存中的那部分需要加载到VBO中。所以第一步我们要起用顶点数组和纹理坐标数组。
    //接着我们必须告诉OpenGL去把数据的指针设置到特定的地方，glVertexPointer函数可以完成这个功能。
    //我们分为启用和不启用VBO两个路径来渲染，他们都差不多，唯一的区别是当你需要把指针指向VBO缓存时，记得把数据指针设置NULL。
    // 如果支持VBO扩展
    if( g_fVBOSupported )
    {
        glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_pMesh->m_nVBOVertices );
        glVertexPointer( 3, GL_FLOAT, 0, (char *) NULL );		// 设置顶点数组的指针为顶点缓存
        glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_pMesh->m_nVBOTexCoords );
        glTexCoordPointer( 2, GL_FLOAT, 0, (char *) NULL );		// 设置顶点数组的指针为纹理坐标缓存
    }
    // 不支持VBO扩展
    else
    {
        glVertexPointer( 3, GL_FLOAT, 0, m_pMesh->m_pVertices );
        glTexCoordPointer( 2, GL_FLOAT, 0, m_pMesh->m_pTexCoords );
    }
    //好了,渲染所有的三角形吧
    // 渲染
    glDrawArrays( GL_TRIANGLES, 0, m_pMesh->m_nVertexCount );
    //最后,别忘了恢复到默认的OpenGL状态.
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
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
    QGLWidget::keyPressEvent(event);
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    m_timerCounter += 15;
    m_flYRot += (float) ( 15.0f ) / 1000.0f * 25.0f;		// Consistantly Rotate The Scenery
    updateGL();
    QGLWidget::timerEvent(event);
}

//加载高度图
bool CMesh::loadHeightmap(float flHeightScale, float flResolution)
{
    m_pTextureImage = QImage(":/image/Terrain.bmp");
    m_pTextureImage = m_pTextureImage.convertToFormat(QImage::Format_RGB888);
    m_pTextureImage = m_pTextureImage.mirrored();

    // 读取顶点数据
    m_nVertexCount = (int) ( m_pTextureImage.width() * m_pTextureImage.height() * 6 / ( flResolution * flResolution ) );
    m_pVertices = new CVec[m_nVertexCount];
    m_pTexCoords = new CTexCoord[m_nVertexCount];
    int nX, nZ, nTri, nIndex=0;
    float flX, flZ;
    for( nZ = 0; nZ < m_pTextureImage.height(); nZ += (int) flResolution )
    {
        for( nX = 0; nX < m_pTextureImage.width(); nX += (int) flResolution )
        {
            for( nTri = 0; nTri < 6; nTri++ )
            {
                flX = (float) nX + ( ( nTri == 1 || nTri == 2 || nTri == 5 ) ? flResolution : 0.0f );
                flZ = (float) nZ + ( ( nTri == 2 || nTri == 4 || nTri == 5 ) ? flResolution : 0.0f );
                m_pVertices[nIndex].x = flX - ( m_pTextureImage.width() / 2 );
                m_pVertices[nIndex].y = ptHeight( (int) flX, (int) flZ ) *  flHeightScale;
                m_pVertices[nIndex].z = flZ - ( m_pTextureImage.height() / 2 );
                m_pTexCoords[nIndex].u = flX / m_pTextureImage.width();
                m_pTexCoords[nIndex].v = flZ / m_pTextureImage.height();
                nIndex++;
            }
        }
    }
    // 载入纹理，它和高度图是同一副图像
    glGenTextures( 1, &m_nTextureId );
    glBindTexture( GL_TEXTURE_2D, m_nTextureId );
    glTexImage2D( GL_TEXTURE_2D, 0, 3, m_pTextureImage.width(), m_pTextureImage.height(),
                  0, GL_RGB, GL_UNSIGNED_BYTE, m_pTextureImage.bits() );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    return true;
}

//计算(x,y)处的亮度
float CMesh::ptHeight(int nX, int nY)
{
    int nPos = ( ( nX % m_pTextureImage.width() )  + ( ( nY % m_pTextureImage.height() ) * m_pTextureImage.width() ) ) * 3;
    float flR = (float) m_pTextureImage.bits()[ nPos ];			// 返回红色分量
    float flG = (float) m_pTextureImage.bits()[ nPos + 1 ];		// 返回绿色分量
    float flB = (float) m_pTextureImage.bits()[ nPos + 2 ];		// 返回蓝色分量
    return ( 0.299f * flR + 0.587f * flG + 0.114f * flB );		// 计算亮度
}

//下面的代码把顶点数据绑定到顶点缓存，即把内存中的数据发送到显存
void CMesh::buildVBOs()
{
    glGenBuffersARB( 1, &m_nVBOVertices );// 创建一个顶点缓存，并把顶点数据绑定到缓存
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOVertices );
    glBufferDataARB( GL_ARRAY_BUFFER_ARB, m_nVertexCount*3*sizeof(float), m_pVertices, GL_STATIC_DRAW_ARB );
    glGenBuffersARB( 1, &m_nVBOTexCoords ); // 创建一个纹理缓存，并把纹理数据绑定到缓存
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOTexCoords );
    glBufferDataARB( GL_ARRAY_BUFFER_ARB, m_nVertexCount*2*sizeof(float), m_pTexCoords, GL_STATIC_DRAW_ARB );

    // 删除分配的内存
    delete [] m_pVertices;
    m_pVertices = NULL;
    delete [] m_pTexCoords;
    m_pTexCoords = NULL;
}

CMesh::CMesh()
{
    // Set Pointers To NULL
    m_pVertices = NULL;
    m_pTexCoords = NULL;
    m_nVertexCount = 0;
    m_nVBOVertices = m_nVBOTexCoords = m_nTextureId = 0;
}

CMesh::~CMesh()
{
    // Delete VBOs
    if( g_fVBOSupported )
    {
        unsigned int nBuffers[2] = { m_nVBOVertices, m_nVBOTexCoords };
        glDeleteBuffersARB( 2, nBuffers );						// Free The Memory
    }
    // Delete Data
    if( m_pVertices )											// Deallocate Vertex Data
        delete [] m_pVertices;
    m_pVertices = NULL;
    if( m_pTexCoords )											// Deallocate Texture Coord Data
        delete [] m_pTexCoords;
    m_pTexCoords = NULL;
}
