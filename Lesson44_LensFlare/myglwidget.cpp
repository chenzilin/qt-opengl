#include "myglwidget.h"

//3D 光晕
//当镜头对准太阳的时候就会出现这种效果，模拟它非常的简单，一点数学和纹理贴图就够了。好好看看吧。

//大家好,欢迎来到新的一课,在这一课中我们将扩展glCamera类，来实现镜头光晕的效果。
//在日常生活中，当我们对着光源看时，会发现强烈的反光。
//为了完成这个效果，我们需要一些数学知识。首先，我们需要一些函数，用来检测某个点或球是否在当前的视景体内。
//接着我们需要一些纹理作为我们的光晕效果，我们可以把它贴在显示面上。

glVector::glVector()
{
    i = j = k = m_Mag = 0.0f;// Initalize the i, j, k, and magnitude to 0.0
}

glVector::~glVector()
{

}

// Scale the vector times some number
void glVector::operator *=(GLfloat scalar)
{
    i *= scalar;
    j *= scalar;
    k *= scalar;
}

// Computes the magnitude and saves it in m_Mag also returns the
// Magnitude
GLfloat glVector::Magnitude()
{
    GLfloat result;

    result = GLfloat(qSqrt(i * i + j * j + k * k));
    m_Mag = result;
    return(result);
}

// Makes the vector unit length.
void glVector::Normalize()
{
    if(m_Mag != 0.0f)
    {
        i /= m_Mag;
        j /= m_Mag;
        k /= m_Mag;

        Magnitude();
    }
}

// Multiply this vector times another and return the result
glVector glVector::operator *(GLfloat scalar)
{
    glVector r;

    r.i = i * scalar;
    r.j = j * scalar;
    r.k = k * scalar;

    return(r);
}

// Add this vector to another and return the result
glVector glVector::operator +(glVector v)
{
    glVector r;

    r.i = i + v.i;
    r.j = j + v.j;
    r.k = k + v.k;

    return(r);
}

// Assign this vector to the vector passed in.
void glVector::operator =(glVector v)
{
    i = v.i;
    j = v.j;
    k = v.k;
    m_Mag = v.m_Mag;
}

glPoint::glPoint()
{
    x = y = z = 0.0f;												// Initalize the x, y, z components to zero
}

glPoint::~glPoint()
{

}

void glPoint::operator =(glPoint p)
{
    x = p.x;														// Set this point to = the point passed in.
    y = p.y;
    z = p.z;
}

// Usually this operator get called when we scaled a vector to get
// a 3D point IE  point = unit_vector * 25.0f;
void glPoint::operator =(glVector v)
{
    x = v.i;														// Set this point to the vectors i component
    y = v.j;														// Set this point to the vectors j component
    z = v.k;														// Set this point to the vectors k component
}

// Subrtacts the components of two points to get a vector.
glVector glPoint::operator -(glPoint p)
{
    glVector r;

    r.i = x - p.x;
    r.j = y - p.y;
    r.k = z - p.z;

    return(r);
}

// Technically your not supposed to be able to add 2 points
// together by mathmatical definations but I don't mind
// bending the rules a little.
glPoint glPoint::operator +(glPoint p)
{
    glPoint r;

    r.x = x + p.x;
    r.y = y + p.y;
    r.z = z + p.z;

    return(r);
}

// Technically your not supposed to be able to add 2 points
// together by mathmatical definations but I don't mind
// bending the rules a little.
void glPoint::operator +=(glPoint p)
{
    x += p.x;
    y += p.y;
    z += p.z;
}

glCamera::glCamera()
{
    // Initalize all our member varibles.
    m_MaxPitchRate			= 0.0f;
    m_MaxHeadingRate		= 0.0f;
    m_HeadingDegrees		= 0.0f;
    m_PitchDegrees			= 0.0f;
    m_MaxForwardVelocity	= 0.0f;
    m_ForwardVelocity		= 0.0f;
    m_LightSourcePos.x      = 0.0f;
    m_LightSourcePos.y      = 0.0f;
    m_LightSourcePos.z      = 0.0f;
    m_GlowTexture           = 0;
    m_HaloTexture			= 0;
    m_StreakTexture			= 0;
    m_MaxPointSize			= 0.0f;
}

glCamera::~glCamera()
{
    if(m_GlowTexture != 0) {										// If the glow texture is valid
        glDeleteTextures(1, &m_GlowTexture);						// delete the glow texture
    }

    if(m_HaloTexture != 0) {										// If the halo texture is valid
        glDeleteTextures(1, &m_HaloTexture);						// delete the halo texture
    }

    if(m_BigGlowTexture != 0) {										// If the BigGlow texture is valid
        glDeleteTextures(1, &m_BigGlowTexture);						// delete the BigGlow texture
    }

    if(m_StreakTexture != 0) {										// If the Streak texture is valid
        glDeleteTextures(1, &m_StreakTexture);						// delete the StreakTexture
    }
}

void glCamera::SetPrespective()
{
    GLfloat Matrix[16];												// A array to hold the model view matrix.
    glVector v;														// A vector to hold our cameras direction * the forward velocity
                                                                    // we don't want to destory the Direction vector by using it instead.

    // 根据当前的偏转角旋转视线
    glRotatef(m_HeadingDegrees, 0.0f, 1.0f, 0.0f);
    glRotatef(m_PitchDegrees, 1.0f, 0.0f, 0.0f);

    // 返回模型变换矩阵
    glGetFloatv(GL_MODELVIEW_MATRIX, Matrix);

    // 获得视线的方向
    m_DirectionVector.i = Matrix[8];
    m_DirectionVector.j = Matrix[9];
    m_DirectionVector.k = -Matrix[10];

    // 重置矩阵
    glLoadIdentity();

    // 旋转场景
    glRotatef(m_PitchDegrees, 1.0f, 0.0f, 0.0f);
    glRotatef(m_HeadingDegrees, 0.0f, 1.0f, 0.0f);

    // 设置当前摄像机的位置
    v = m_DirectionVector;
    v *= m_ForwardVelocity;

    // Increment our position by the vector
    m_Position.x += v.i;
    m_Position.y += v.j;
    m_Position.z += v.k;

    // 变换到新的位置
    glTranslatef(-m_Position.x, -m_Position.y, -m_Position.z);
}

void glCamera::ChangePitch(GLfloat degrees)
{
    if(qFabs(degrees) < qFabs(m_MaxPitchRate))
    {
        // Our pitch is less than the max pitch rate that we
        // defined so lets increment it.
        m_PitchDegrees += degrees;
    }
    else
    {
        // Our pitch is greater than the max pitch rate that
        // we defined so we can only increment our pitch by the
        // maximum allowed value.
        if(degrees < 0)
        {
            // We are pitching down so decrement
            m_PitchDegrees -= m_MaxPitchRate;
        }
        else
        {
            // We are pitching up so increment
            m_PitchDegrees += m_MaxPitchRate;
        }
    }

    // We don't want our pitch to run away from us. Although it
    // really doesn't matter I prefer to have my pitch degrees
    // within the range of -360.0f to 360.0f
    if(m_PitchDegrees > 360.0f)
    {
        m_PitchDegrees -= 360.0f;
    }
    else if(m_PitchDegrees < -360.0f)
    {
        m_PitchDegrees += 360.0f;
    }
}

void glCamera::ChangeHeading(GLfloat degrees)
{
    if(qFabs(degrees) < qFabs(m_MaxHeadingRate))
    {
        // Our Heading is less than the max heading rate that we
        // defined so lets increment it but first we must check
        // to see if we are inverted so that our heading will not
        // become inverted.
        if((m_PitchDegrees > 90 && m_PitchDegrees < 270) || (m_PitchDegrees < -90 && m_PitchDegrees > -270))
        {
            m_HeadingDegrees -= degrees;
        }
        else
        {
            m_HeadingDegrees += degrees;
        }
    }
    else
    {
        // Our heading is greater than the max heading rate that
        // we defined so we can only increment our heading by the
        // maximum allowed value.
        if(degrees < 0)
        {
            // Check to see if we are upside down.
            if((m_PitchDegrees > 90 && m_PitchDegrees < 270) || (m_PitchDegrees < -90 && m_PitchDegrees > -270))
            {
                // Ok we would normally decrement here but since we are upside
                // down then we need to increment our heading
                m_HeadingDegrees += m_MaxHeadingRate;
            }
            else
            {
                // We are not upside down so decrement as usual
                m_HeadingDegrees -= m_MaxHeadingRate;
            }
        }
        else
        {
            // Check to see if we are upside down.
            if((m_PitchDegrees > 90 && m_PitchDegrees < 270) || (m_PitchDegrees < -90 && m_PitchDegrees > -270))
            {
                // Ok we would normally increment here but since we are upside
                // down then we need to decrement our heading.
                m_HeadingDegrees -= m_MaxHeadingRate;
            }
            else
            {
                // We are not upside down so increment as usual.
                m_HeadingDegrees += m_MaxHeadingRate;
            }
        }
    }

    // We don't want our heading to run away from us either. Although it
    // really doesn't matter I prefer to have my heading degrees
    // within the range of -360.0f to 360.0f
    if(m_HeadingDegrees > 360.0f)
    {
        m_HeadingDegrees -= 360.0f;
    }
    else if(m_HeadingDegrees < -360.0f)
    {
        m_HeadingDegrees += 360.0f;
    }
}

void glCamera::ChangeVelocity(GLfloat vel)
{
    if(qFabs(vel) < qFabs(m_MaxForwardVelocity))
    {
        // Our velocity is less than the max velocity increment that we
        // defined so lets increment it.
        m_ForwardVelocity += vel;
    }
    else
    {
        // Our velocity is greater than the max velocity increment that
        // we defined so we can only increment our velocity by the
        // maximum allowed value.
        if(vel < 0)
        {
            // We are slowing down so decrement
            m_ForwardVelocity -= -m_MaxForwardVelocity;
        }
        else
        {
            // We are speeding up so increment
            m_ForwardVelocity += m_MaxForwardVelocity;
        }
    }
}

//好了，我们现在开始吧。我将使用4个对立的纹理来制造我们的镜头光晕，第一和二个光晕图像被放置在光源处，
//第三和第四个图像将根据视点的位置和方向动态的生成。
//现在你在头脑里应该有了一个大慨地图像了吧。我们来说说何时我们应该绘制光晕，一般来说平时我们是看不见这些光晕的，
//只有当我们对准光源的时候才能看见这些。所以我们首先要获得视景体的数据，下面的函数可以帮我们完成这个功能。

// I found this code here: http://www.markmorley.com/opengl/frustumculling.html
// and decided to make it part of
// the camera class just in case I might want to rotate
// and translate the projection matrix. This code will
// make sure that the Frustum is updated correctly but
// this member is computational expensive with:
// 82 muliplications, 72 additions, 24 divisions, and
// 12 subtractions for a total of 190 operations. Ouch!
void glCamera::UpdateFrustum()
{
    GLfloat   clip[16];
    GLfloat   proj[16];
    GLfloat   modl[16];
    GLfloat   t;

    //返回投影矩阵
    glGetFloatv( GL_PROJECTION_MATRIX, proj );

    //返回模型变换矩阵
    glGetFloatv( GL_MODELVIEW_MATRIX, modl );

    //计算剪切矩阵，即上面两个矩阵的乘积
    clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
    clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
    clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
    clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

    clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
    clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
    clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
    clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

    clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
    clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
    clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
    clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

    clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
    clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
    clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
    clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

    //提取左面的平面方程系数
    m_Frustum[0][0] = clip[ 3] - clip[ 0];
    m_Frustum[0][1] = clip[ 7] - clip[ 4];
    m_Frustum[0][2] = clip[11] - clip[ 8];
    m_Frustum[0][3] = clip[15] - clip[12];

    /* Normalize the result */
    t = GLfloat(qSqrt( m_Frustum[0][0] * m_Frustum[0][0] + m_Frustum[0][1] * m_Frustum[0][1] + m_Frustum[0][2] * m_Frustum[0][2] ));
    m_Frustum[0][0] /= t;
    m_Frustum[0][1] /= t;
    m_Frustum[0][2] /= t;
    m_Frustum[0][3] /= t;

    //提取右面的平面方程系数
    m_Frustum[1][0] = clip[ 3] + clip[ 0];
    m_Frustum[1][1] = clip[ 7] + clip[ 4];
    m_Frustum[1][2] = clip[11] + clip[ 8];
    m_Frustum[1][3] = clip[15] + clip[12];

    /* Normalize the result */
    t = GLfloat(qSqrt( m_Frustum[1][0] * m_Frustum[1][0] + m_Frustum[1][1] * m_Frustum[1][1] + m_Frustum[1][2] * m_Frustum[1][2] ));
    m_Frustum[1][0] /= t;
    m_Frustum[1][1] /= t;
    m_Frustum[1][2] /= t;
    m_Frustum[1][3] /= t;

    //提取下面的平面方程系数
    m_Frustum[2][0] = clip[ 3] + clip[ 1];
    m_Frustum[2][1] = clip[ 7] + clip[ 5];
    m_Frustum[2][2] = clip[11] + clip[ 9];
    m_Frustum[2][3] = clip[15] + clip[13];

    /* Normalize the result */
    t = GLfloat(qSqrt( m_Frustum[2][0] * m_Frustum[2][0] + m_Frustum[2][1] * m_Frustum[2][1] + m_Frustum[2][2] * m_Frustum[2][2] ));
    m_Frustum[2][0] /= t;
    m_Frustum[2][1] /= t;
    m_Frustum[2][2] /= t;
    m_Frustum[2][3] /= t;

    //提取上面的平面方程系数
    m_Frustum[3][0] = clip[ 3] - clip[ 1];
    m_Frustum[3][1] = clip[ 7] - clip[ 5];
    m_Frustum[3][2] = clip[11] - clip[ 9];
    m_Frustum[3][3] = clip[15] - clip[13];

    /* Normalize the result */
    t = GLfloat(qSqrt( m_Frustum[3][0] * m_Frustum[3][0] + m_Frustum[3][1] * m_Frustum[3][1] + m_Frustum[3][2] * m_Frustum[3][2] ));
    m_Frustum[3][0] /= t;
    m_Frustum[3][1] /= t;
    m_Frustum[3][2] /= t;
    m_Frustum[3][3] /= t;

    //提取远面的平面方程系数
    m_Frustum[4][0] = clip[ 3] - clip[ 2];
    m_Frustum[4][1] = clip[ 7] - clip[ 6];
    m_Frustum[4][2] = clip[11] - clip[10];
    m_Frustum[4][3] = clip[15] - clip[14];

    /* Normalize the result */
    t = GLfloat(qSqrt( m_Frustum[4][0] * m_Frustum[4][0] + m_Frustum[4][1] * m_Frustum[4][1] + m_Frustum[4][2] * m_Frustum[4][2] ));
    m_Frustum[4][0] /= t;
    m_Frustum[4][1] /= t;
    m_Frustum[4][2] /= t;
    m_Frustum[4][3] /= t;

    //提取近面的平面方程系数
    m_Frustum[5][0] = clip[ 3] + clip[ 2];
    m_Frustum[5][1] = clip[ 7] + clip[ 6];
    m_Frustum[5][2] = clip[11] + clip[10];
    m_Frustum[5][3] = clip[15] + clip[14];

    /* Normalize the result */
    t = GLfloat(qSqrt( m_Frustum[5][0] * m_Frustum[5][0] + m_Frustum[5][1] * m_Frustum[5][1] + m_Frustum[5][2] * m_Frustum[5][2] ));
    m_Frustum[5][0] /= t;
    m_Frustum[5][1] /= t;
    m_Frustum[5][2] /= t;
    m_Frustum[5][3] /= t;
}

// This is the much faster version of the above member
// function, however the speed increase is not gained
// without a cost. If you rotate or translate the projection
// matrix then this member will not work correctly. That is acceptable
// in my book considering I very rarely do such a thing.
// This function has far fewer operations in it and I
// shaved off 2 square root functions by passing in the
// near and far values. This member has:
// 38 muliplications, 28 additions, 24 divisions, and
// 12 subtractions for a total of 102 operations. Still hurts
// but at least it is decent now. In practice this will
// run about 2 times faster than the above function.
void glCamera::UpdateFrustumFaster()
{
    GLfloat   clip[16];
    GLfloat   proj[16];
    GLfloat   modl[16];
    GLfloat   t;

    /* Get the current PROJECTION matrix from OpenGL */
    glGetFloatv( GL_PROJECTION_MATRIX, proj );

    /* Get the current MODELVIEW matrix from OpenGL */
    glGetFloatv( GL_MODELVIEW_MATRIX, modl );

    /* Combine the two matrices (multiply projection by modelview)
       but keep in mind this function will only work if you do NOT
       rotate or translate your projection matrix                  */
    clip[ 0] = modl[ 0] * proj[ 0];
    clip[ 1] = modl[ 1] * proj[ 5];
    clip[ 2] = modl[ 2] * proj[10] + modl[ 3] * proj[14];
    clip[ 3] = modl[ 2] * proj[11];

    clip[ 4] = modl[ 4] * proj[ 0];
    clip[ 5] = modl[ 5] * proj[ 5];
    clip[ 6] = modl[ 6] * proj[10] + modl[ 7] * proj[14];
    clip[ 7] = modl[ 6] * proj[11];

    clip[ 8] = modl[ 8] * proj[ 0];
    clip[ 9] = modl[ 9] * proj[ 5];
    clip[10] = modl[10] * proj[10] + modl[11] * proj[14];
    clip[11] = modl[10] * proj[11];

    clip[12] = modl[12] * proj[ 0];
    clip[13] = modl[13] * proj[ 5];
    clip[14] = modl[14] * proj[10] + modl[15] * proj[14];
    clip[15] = modl[14] * proj[11];

    /* Extract the numbers for the RIGHT plane */
    m_Frustum[0][0] = clip[ 3] - clip[ 0];
    m_Frustum[0][1] = clip[ 7] - clip[ 4];
    m_Frustum[0][2] = clip[11] - clip[ 8];
    m_Frustum[0][3] = clip[15] - clip[12];

    /* Normalize the result */
    t = GLfloat(qSqrt( m_Frustum[0][0] * m_Frustum[0][0] + m_Frustum[0][1] * m_Frustum[0][1] + m_Frustum[0][2] * m_Frustum[0][2] ));
    m_Frustum[0][0] /= t;
    m_Frustum[0][1] /= t;
    m_Frustum[0][2] /= t;
    m_Frustum[0][3] /= t;

    /* Extract the numbers for the LEFT plane */
    m_Frustum[1][0] = clip[ 3] + clip[ 0];
    m_Frustum[1][1] = clip[ 7] + clip[ 4];
    m_Frustum[1][2] = clip[11] + clip[ 8];
    m_Frustum[1][3] = clip[15] + clip[12];

    /* Normalize the result */
    t = GLfloat(qSqrt( m_Frustum[1][0] * m_Frustum[1][0] + m_Frustum[1][1] * m_Frustum[1][1] + m_Frustum[1][2] * m_Frustum[1][2] ));
    m_Frustum[1][0] /= t;
    m_Frustum[1][1] /= t;
    m_Frustum[1][2] /= t;
    m_Frustum[1][3] /= t;

    /* Extract the BOTTOM plane */
    m_Frustum[2][0] = clip[ 3] + clip[ 1];
    m_Frustum[2][1] = clip[ 7] + clip[ 5];
    m_Frustum[2][2] = clip[11] + clip[ 9];
    m_Frustum[2][3] = clip[15] + clip[13];

    /* Normalize the result */
    t = GLfloat(qSqrt( m_Frustum[2][0] * m_Frustum[2][0] + m_Frustum[2][1] * m_Frustum[2][1] + m_Frustum[2][2] * m_Frustum[2][2] ));
    m_Frustum[2][0] /= t;
    m_Frustum[2][1] /= t;
    m_Frustum[2][2] /= t;
    m_Frustum[2][3] /= t;

    /* Extract the TOP plane */
    m_Frustum[3][0] = clip[ 3] - clip[ 1];
    m_Frustum[3][1] = clip[ 7] - clip[ 5];
    m_Frustum[3][2] = clip[11] - clip[ 9];
    m_Frustum[3][3] = clip[15] - clip[13];

    /* Normalize the result */
    t = GLfloat(qSqrt( m_Frustum[3][0] * m_Frustum[3][0] + m_Frustum[3][1] * m_Frustum[3][1] + m_Frustum[3][2] * m_Frustum[3][2] ));
    m_Frustum[3][0] /= t;
    m_Frustum[3][1] /= t;
    m_Frustum[3][2] /= t;
    m_Frustum[3][3] /= t;

    /* Extract the FAR plane */
    m_Frustum[4][0] = clip[ 3] - clip[ 2];
    m_Frustum[4][1] = clip[ 7] - clip[ 6];
    m_Frustum[4][2] = clip[11] - clip[10];
    m_Frustum[4][3] = clip[15] - clip[14];

    /* Normalize the result */
    t = GLfloat(qSqrt( m_Frustum[4][0] * m_Frustum[4][0] + m_Frustum[4][1] * m_Frustum[4][1] + m_Frustum[4][2] * m_Frustum[4][2] ));
    m_Frustum[4][0] /= t;
    m_Frustum[4][1] /= t;
    m_Frustum[4][2] /= t;
    m_Frustum[4][3] /= t;

    /* Extract the NEAR plane */
    m_Frustum[5][0] = clip[ 3] + clip[ 2];
    m_Frustum[5][1] = clip[ 7] + clip[ 6];
    m_Frustum[5][2] = clip[11] + clip[10];
    m_Frustum[5][3] = clip[15] + clip[14];

    /* Normalize the result */
    t = GLfloat(qSqrt( m_Frustum[5][0] * m_Frustum[5][0] + m_Frustum[5][1] * m_Frustum[5][1] + m_Frustum[5][2] * m_Frustum[5][2] ));
    m_Frustum[5][0] /= t;
    m_Frustum[5][1] /= t;
    m_Frustum[5][2] /= t;
    m_Frustum[5][3] /= t;
}

// This member function checks to see if a sphere is in
// the viewing volume.
BOOL glCamera::SphereInFrustum(glPoint p, GLfloat Radius)
{
    int i;

    // The idea here is the same as the PointInFrustum function.

    for(i = 0; i < 6; i++)
    {
        // If the point is outside of the plane then its not in the viewing volume.
        if(m_Frustum[i][0] * p.x + m_Frustum[i][1] * p.y + m_Frustum[i][2] * p.z + m_Frustum[i][3] <= -Radius)
        {
            return(FALSE);
        }
    }

    return(TRUE);
}

//现在我们可以测试一个点或圆是否在视景体内了。下面的函数可以测试一个点是否在视景体内。
// This member fuction checks to see if a point is in
// the viewing volume.
BOOL glCamera::PointInFrustum(glPoint p)
{
    int i;

    // The idea behind this algorithum is that if the point
    // is inside all 6 clipping planes then it is inside our
    // viewing volume so we can return true.

    for(i = 0; i < 6; i++)
    {
        if(m_Frustum[i][0] * p.x + m_Frustum[i][1] * p.y + m_Frustum[i][2] * p.z + m_Frustum[i][3] <= 0)
        {
            return(FALSE);
        }
    }

    return(TRUE);
}

// This member function checks to see if a sphere is in
// the viewing volume.
BOOL glCamera::SphereInFrustum(GLfloat x, GLfloat y, GLfloat z, GLfloat Radius)
{
    int i;

    // The idea here is the same as the PointInFrustum function.

    for(i = 0; i < 6; i++)
    {
        // If the point is outside of the plane then its not in the viewing volume.
        if(m_Frustum[i][0] * x + m_Frustum[i][1] * y + m_Frustum[i][2] * z + m_Frustum[i][3] <= -Radius)
        {
            return(FALSE);
        }
    }

    return(TRUE);
}

// This member fuction checks to see if a point is in
// the viewing volume.
BOOL glCamera::PointInFrustum(GLfloat x, GLfloat y, GLfloat z)
{
    int i;

    // The idea behind this algorithum is that if the point
    // is inside all 6 clipping planes then it is inside our
    // viewing volume so we can return true.

    for(i = 0; i < 6; i++)	// Loop through all our clipping planes
    {
        // If the point is outside of the plane then its not in the viewing volume.
        if(m_Frustum[i][0] * x + m_Frustum[i][1] * y + m_Frustum[i][2] * z + m_Frustum[i][3] <= 0)
        {
            return(FALSE);
        }
    }

    return(TRUE);
}

//下面的函数用来测试某个点是否位于当前场景物体的前面:
bool glCamera::IsOccluded(glPoint p)
{
    GLint viewport[4];											//space for viewport data
    GLdouble mvmatrix[16], projmatrix[16];  //space for transform matricex
    GLdouble winx, winy, winz;							//space for returned projected coords
    GLdouble flareZ;												//here we will store the transformed flare Z
    GLfloat bufferZ;												//here we will store the read Z from the buffer

    // Now we will ask OGL to project some geometry for us using the gluProject function.
    // Practically we ask OGL to guess where a point in space will be projected in our current viewport,
    // using arbitrary viewport and transform matrices we pass to the function.
    // If we pass to the function the current matrices  (retrievede with the glGet funcs)
    // we will have the real position on screen where the dot will be drawn.
    // The interesting part is that we also get a Z value back, this means that
    // reading the REAL buffer for Z values we can discover if the flare is in front or
    // if it's occluded by some objects.


    glGetIntegerv (GL_VIEWPORT, viewport);						//get actual viewport
    glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);			//get actual model view matrix
    glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);	//get actual projiection matrix

    // 返回顶点p在单位立方体中的位置
    gluProject(p.x, p.y, p.z, mvmatrix, projmatrix, viewport, &winx, &winy, &winz);
    flareZ = winz;

    // 读取点(winx,winy)的深度坐标
    glReadPixels(winx, winy,1,1,GL_DEPTH_COMPONENT, GL_FLOAT, &bufferZ);

    // 如果深度坐标小于点的坐标，则返回true
    if (bufferZ < flareZ)
        return true;
    else
        return false;
}

//我们通过检测光源是否正对我们的视线来决定是否绘制光晕，但如果你的视点超过了光源的位置，则会发生看不见光晕的现象。
//为了避免这种现象，我们在移动视点的使用，也相应的移动我们的光源。
//为了在视点和光源之间绘制多个光晕，我们需要计算之间的向量，下面的代码完成这个功能：
//下面的函数完成具体的渲染光晕的任务
void glCamera::RenderLensFlare()
{
    GLfloat Length = 0.0f;

    // Draw the flare only If the light source is in our line of sight
    // 如果我们的光源在我们的视线范围内，则绘制它
    if(SphereInFrustum(m_LightSourcePos, 1.0f) == TRUE)
    {
        vLightSourceToCamera = m_Position - m_LightSourcePos;		// 计算光源到我们视线的距离
                                                                    // Lets compute the vector that points to the camera from
                                                                    // the light source.

        Length = vLightSourceToCamera.Magnitude();					// Save the length we will need it in a minute

                                                                    //下面三个函数计算光源位置到光晕结束位置之间的向量
        ptIntersect = m_DirectionVector * Length;					// Now lets find an point along the cameras direction
                                                                    // vector that we can use as an intersection point.
                                                                    // Lets translate down this vector the same distance
                                                                    // that the camera is away from the light source.
        ptIntersect += m_Position;

        vLightSourceToIntersect = ptIntersect - m_LightSourcePos;	// Lets compute the vector that points to the Intersect
                                                                    // point from the light source
        Length = vLightSourceToIntersect.Magnitude();				// Save the length we will need it later.
        vLightSourceToIntersect.Normalize();						// Normalize the vector so its unit length

        glEnable(GL_BLEND);											// You should already know what this does
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);							// You should already know what this does
        glDisable(GL_DEPTH_TEST);									// You should already know what this does
        glEnable(GL_TEXTURE_2D);									// You should already know what this does

        /////////// Differenet Color Glows & Streaks /////////////////////
        //RenderBigGlow(1.0f, 1.0f, 1.0f, 1.0f, m_LightSourcePos, 1.0f);
        //RenderStreaks(1.0f, 1.0f, 0.8f, 1.0f, m_LightSourcePos, 0.7f);
        //
        //RenderBigGlow(1.0f, 0.9f, 1.0f, 1.0f, m_LightSourcePos, 1.0f);
        //RenderStreaks(1.0f, 0.9f, 1.0f, 1.0f, m_LightSourcePos, 0.7f);
        //////////////////////////////////////////////////////////////////

        //首先我们需要找到光源位置和视点位置之间的向量，接下来我们需要在视线的方向设置一个插值点，
        //这个点的距离必须和光源位置和视点位置之间的距离相等。完成以后，我们找出可以产生光晕的方向，即下图红线的方向，
        //在这个线上我们可以绘制我们的光晕。
        //########################## NEW STUFF ##################################

        //如果光晕可见
        if (!IsOccluded(m_LightSourcePos))	//Check if the center of the flare is occluded
        {
            // 渲染中间的光晕
            RenderBigGlow(0.60f, 0.60f, 0.8f, 1.0f, m_LightSourcePos, 16.0f);
            // Render the streaks
            RenderStreaks(0.60f, 0.60f, 0.8f, 1.0f, m_LightSourcePos, 16.0f);
            // Render the small Glow
            RenderGlow(0.8f, 0.8f, 1.0f, 0.5f, m_LightSourcePos, 3.5f);

            //绘制到光晕结束位置的0.1处的光晕
            pt = vLightSourceToIntersect * (Length * 0.1f);				// Lets compute a point that is 20%
            pt += m_LightSourcePos;										// away from the light source in the
                                                                    // direction of the intersection point.

            RenderGlow(0.9f, 0.6f, 0.4f, 0.5f, pt, 0.6f);				// Render the small Glow

            //绘制到光晕结束位置的0.15处的光晕
            pt = vLightSourceToIntersect * (Length * 0.15f);			// Lets compute a point that is 30%
            pt += m_LightSourcePos;										// away from the light source in the

            RenderHalo(0.8f, 0.5f, 0.6f, 0.5f, pt, 1.7f);				// Render the a Halo

            //绘制到光晕结束位置的0.175处的光晕
            pt = vLightSourceToIntersect * (Length * 0.175f);			// Lets compute a point that is 35%
            pt += m_LightSourcePos;										// away from the light source in the
                                                                    // direction of the intersection point.

            RenderHalo(0.9f, 0.2f, 0.1f, 0.5f, pt, 0.83f);				// Render the a Halo

            //绘制到光晕结束位置的0.285处的光晕
            pt = vLightSourceToIntersect * (Length * 0.285f);			// Lets compute a point that is 57%
            pt += m_LightSourcePos;										// away from the light source in the
                                                                    // direction of the intersection point.

            RenderHalo(0.7f, 0.7f, 0.4f, 0.5f, pt, 1.6f);				// Render the a Halo

            //绘制到光晕结束位置的0.2755处的光晕
            pt = vLightSourceToIntersect * (Length * 0.2755f);			// Lets compute a point that is 55.1%
            pt += m_LightSourcePos;										// away from the light source in the
                                                                    // direction of the intersection point.

            RenderGlow(0.9f, 0.9f, 0.2f, 0.5f, pt, 0.8f);				// Render the small Glow

            //绘制到光晕结束位置的0.4755处的光晕
            pt = vLightSourceToIntersect * (Length * 0.4775f);			// Lets compute a point that is 95.5%
            pt += m_LightSourcePos;										// away from the light source in the
                                                                    // direction of the intersection point.

            RenderGlow(0.93f, 0.82f, 0.73f, 0.5f, pt, 1.0f);			// Render the small Glow

            //绘制到光晕结束位置的0.49处的光晕
            pt = vLightSourceToIntersect * (Length * 0.49f);			// Lets compute a point that is 98%
            pt += m_LightSourcePos;										// away from the light source in the
                                                                    // direction of the intersection point.

            RenderHalo(0.7f, 0.6f, 0.5f, 0.5f, pt, 1.4f);				// Render the a Halo

            //绘制到光晕结束位置的0.65处的光晕
            pt = vLightSourceToIntersect * (Length * 0.65f);			// Lets compute a point that is 130%
            pt += m_LightSourcePos;										// away from the light source in the
                                                                    // direction of the intersection point.

            RenderGlow(0.7f, 0.8f, 0.3f, 0.5f, pt, 1.8f);				// Render the small Glow

            //绘制到光晕结束位置的0.63处的光晕
            pt = vLightSourceToIntersect * (Length * 0.63f);			// Lets compute a point that is 126%
            pt += m_LightSourcePos;										// away from the light source in the
                                                                    // direction of the intersection point.

            RenderGlow(0.4f, 0.3f, 0.2f, 0.5f, pt, 1.4f);				// Render the small Glow

            //绘制到光晕结束位置的0.8处的光晕
            pt = vLightSourceToIntersect * (Length * 0.8f);				// Lets compute a point that is 160%
            pt += m_LightSourcePos;										// away from the light source in the
                                                                    // direction of the intersection point.

            RenderHalo(0.7f, 0.5f, 0.5f, 0.5f, pt, 1.4f);				// Render the a Halo

            //绘制到光晕结束位置的0.7825处的光晕
            pt = vLightSourceToIntersect * (Length * 0.7825f);			// Lets compute a point that is 156.5%
            pt += m_LightSourcePos;										// away from the light source in the
                                                                    // direction of the intersection point.

            RenderGlow(0.8f, 0.5f, 0.1f, 0.5f, pt, 0.6f);				// Render the small Glow

            //绘制到光晕结束位置的1.0处的光晕
            pt = vLightSourceToIntersect * (Length * 1.0f);				// Lets compute a point that is 200%
            pt += m_LightSourcePos;										// away from the light source in the
                                                                    // direction of the intersection point.

            RenderHalo(0.5f, 0.5f, 0.7f, 0.5f, pt, 1.7f);				// Render the a Halo

            //绘制到光晕结束位置的0.975处的光晕
            pt = vLightSourceToIntersect * (Length * 0.975f);			// Lets compute a point that is 195%
            pt += m_LightSourcePos;										// away from the light source in the
                                                                    // direction of the intersection point.

            RenderGlow(0.4f, 0.1f, 0.9f, 0.5f, pt, 2.0f);				// Render the small Glow

        }
        glDisable(GL_BLEND );										// You should already know what this does
        glEnable(GL_DEPTH_TEST);									// You should already know what this does
        glDisable(GL_TEXTURE_2D);									// You should already know what this does
    }
}

//好了，下面的函数用来绘制四种不同的光晕
//绘制Halo形的光晕
void glCamera::RenderHalo(GLfloat r, GLfloat g, GLfloat b, GLfloat a, glPoint p, GLfloat scale)
{
    glPoint q[4];

    // Basically we are just going to make a 2D box
    // from four points we don't need a z coord because
    // we are rotating the camera by the inverse so the
    // texture mapped quads will always face us.
    q[0].x = (p.x - scale);											// Set the x coordinate -scale units from the center point.
    q[0].y = (p.y - scale);											// Set the y coordinate -scale units from the center point.

    q[1].x = (p.x - scale);											// Set the x coordinate -scale units from the center point.
    q[1].y = (p.y + scale);											// Set the y coordinate scale units from the center point.

    q[2].x = (p.x + scale);											// Set the x coordinate scale units from the center point.
    q[2].y = (p.y - scale);											// Set the y coordinate -scale units from the center point.

    q[3].x = (p.x + scale);											// Set the x coordinate scale units from the center point.
    q[3].y = (p.y + scale);											// Set the y coordinate scale units from the center point.

    glPushMatrix();													// Save the model view matrix
    glTranslatef(p.x, p.y, p.z);									// Translate to our point
    glRotatef(-m_HeadingDegrees, 0.0f, 1.0f, 0.0f);
    glRotatef(-m_PitchDegrees, 1.0f, 0.0f, 0.0f);
    glBindTexture(GL_TEXTURE_2D, m_HaloTexture);					// Bind to the Big Glow texture
    glColor4f(r, g, b, a);											// Set the color since the texture is a gray scale

    glBegin(GL_TRIANGLE_STRIP);										// Draw the Big Glow on a Triangle Strip
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(q[0].x, q[0].y);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(q[1].x, q[1].y);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(q[2].x, q[2].y);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(q[3].x, q[3].y);
    glEnd();
    glPopMatrix();													// Restore the model view matrix
}

//绘制Gloew形的光晕
void glCamera::RenderGlow(GLfloat r, GLfloat g, GLfloat b, GLfloat a, glPoint p, GLfloat scale)
{
    glPoint q[4];

    // Basically we are just going to make a 2D box
    // from four points we don't need a z coord because
    // we are rotating the camera by the inverse so the
    // texture mapped quads will always face us.
    q[0].x = (p.x - scale);											// Set the x coordinate -scale units from the center point.
    q[0].y = (p.y - scale);											// Set the y coordinate -scale units from the center point.

    q[1].x = (p.x - scale);											// Set the x coordinate -scale units from the center point.
    q[1].y = (p.y + scale);											// Set the y coordinate scale units from the center point.

    q[2].x = (p.x + scale);											// Set the x coordinate scale units from the center point.
    q[2].y = (p.y - scale);											// Set the y coordinate -scale units from the center point.

    q[3].x = (p.x + scale);											// Set the x coordinate scale units from the center point.
    q[3].y = (p.y + scale);											// Set the y coordinate scale units from the center point.

    glPushMatrix();													// Save the model view matrix
    glTranslatef(p.x, p.y, p.z);									// Translate to our point
    glRotatef(-m_HeadingDegrees, 0.0f, 1.0f, 0.0f);
    glRotatef(-m_PitchDegrees, 1.0f, 0.0f, 0.0f);
    glBindTexture(GL_TEXTURE_2D, m_GlowTexture);					// Bind to the Big Glow texture
    glColor4f(r, g, b, a);											// Set the color since the texture is a gray scale

    glBegin(GL_TRIANGLE_STRIP);										// Draw the Big Glow on a Triangle Strip
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(q[0].x, q[0].y);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(q[1].x, q[1].y);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(q[2].x, q[2].y);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(q[3].x, q[3].y);
    glEnd();
    glPopMatrix();													// Restore the model view matrix
}

//绘制BigGlow形的光晕
void glCamera::RenderBigGlow(GLfloat r, GLfloat g, GLfloat b, GLfloat a, glPoint p, GLfloat scale)
{
    glPoint q[4];

    // Basically we are just going to make a 2D box
    // from four points we don't need a z coord because
    // we are rotating the camera by the inverse so the
    // texture mapped quads will always face us.
    q[0].x = (p.x - scale);											// Set the x coordinate -scale units from the center point.
    q[0].y = (p.y - scale);											// Set the y coordinate -scale units from the center point.

    q[1].x = (p.x - scale);											// Set the x coordinate -scale units from the center point.
    q[1].y = (p.y + scale);											// Set the y coordinate scale units from the center point.

    q[2].x = (p.x + scale);											// Set the x coordinate scale units from the center point.
    q[2].y = (p.y - scale);											// Set the y coordinate -scale units from the center point.

    q[3].x = (p.x + scale);											// Set the x coordinate scale units from the center point.
    q[3].y = (p.y + scale);											// Set the y coordinate scale units from the center point.

    glPushMatrix();													// Save the model view matrix
    glTranslatef(p.x, p.y, p.z);									// Translate to our point
    glRotatef(-m_HeadingDegrees, 0.0f, 1.0f, 0.0f);
    glRotatef(-m_PitchDegrees, 1.0f, 0.0f, 0.0f);
    glBindTexture(GL_TEXTURE_2D, m_BigGlowTexture);					// Bind to the Big Glow texture
    glColor4f(r, g, b, a);											// Set the color since the texture is a gray scale

    glBegin(GL_TRIANGLE_STRIP);										// Draw the Big Glow on a Triangle Strip
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(q[0].x, q[0].y);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(q[1].x, q[1].y);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(q[2].x, q[2].y);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(q[3].x, q[3].y);
    glEnd();
    glPopMatrix();													// Restore the model view matrix
}

//绘制Streaks形的光晕
void glCamera::RenderStreaks(GLfloat r, GLfloat g, GLfloat b, GLfloat a, glPoint p, GLfloat scale)
{
    glPoint q[4];

    // Basically we are just going to make a 2D box
    // from four points we don't need a z coord because
    // we are rotating the camera by the inverse so the
    // texture mapped quads will always face us.
    q[0].x = (p.x - scale);											// Set the x coordinate -scale units from the center point.
    q[0].y = (p.y - scale);											// Set the y coordinate -scale units from the center point.

    q[1].x = (p.x - scale);											// Set the x coordinate -scale units from the center point.
    q[1].y = (p.y + scale);											// Set the y coordinate scale units from the center point.

    q[2].x = (p.x + scale);											// Set the x coordinate scale units from the center point.
    q[2].y = (p.y - scale);											// Set the y coordinate -scale units from the center point.

    q[3].x = (p.x + scale);											// Set the x coordinate scale units from the center point.
    q[3].y = (p.y + scale);											// Set the y coordinate scale units from the center point.

    glPushMatrix();													// Save the model view matrix
    glTranslatef(p.x, p.y, p.z);									// Translate to our point
    glRotatef(-m_HeadingDegrees, 0.0f, 1.0f, 0.0f);
    glRotatef(-m_PitchDegrees, 1.0f, 0.0f, 0.0f);
    glBindTexture(GL_TEXTURE_2D, m_StreakTexture);					// Bind to the Big Glow texture
    glColor4f(r, g, b, a);											// Set the color since the texture is a gray scale

    glBegin(GL_TRIANGLE_STRIP);										// Draw the Big Glow on a Triangle Strip
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(q[0].x, q[0].y);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(q[1].x, q[1].y);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(q[2].x, q[2].y);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(q[3].x, q[3].y);
    glEnd();
    glPopMatrix();													// Restore the model view matrix
}

glFont::glFont()
{
    m_FontTexture = 0;												// Initalize the texture to 0
    m_ListBase = 0;													// Initalize the List base to 0
}

glFont::~glFont()
{
    if(m_FontTexture != 0) {										// If the texture is valid.
        glDeleteTextures(1, &m_FontTexture);						// delete the texture.
    }

    if(m_ListBase != 0)	{											// If the Listbase is valid
        glDeleteLists(m_ListBase,256);								// delete the list
    }
}

void glFont::SetFontTexture(GLuint tex)
{
    if(tex != 0) {													// If the texture is valid
        m_FontTexture = tex;										// Set the font texture
    }
}

void glFont::BuildFont(GLfloat Scale)
{
    float	cx;														// Holds Our X Character Coord
    float	cy;														// Holds Our Y Character Coord
    GLuint  loop;

    m_ListBase=glGenLists(256);										// Creating 256 Display Lists
    if(m_FontTexture != 0)
    {
        glBindTexture(GL_TEXTURE_2D, m_FontTexture);				// Select Our Font Texture
        for (loop=0; loop<256; loop++)								// Loop Through All 256 Lists
        {
            cx=float(loop%16)/16.0f;								// X Position Of Current Character
            cy=float(loop/16)/16.0f;								// Y Position Of Current Character

            glNewList(m_ListBase+loop,GL_COMPILE);					// Start Building A List
                glBegin(GL_QUADS);									// Use A Quad For Each Character
                    glTexCoord2f(cx, 1 - cy - 0.0625f);				// Texture Coord (Bottom Left)
                    glVertex2f(0,0);								// Vertex Coord (Bottom Left)
                    glTexCoord2f(cx + 0.0625f, 1 - cy - 0.0625f);	// Texture Coord (Bottom Right)
                    glVertex2f(16 * Scale,0);						// Vertex Coord (Bottom Right)
                    glTexCoord2f(cx + 0.0625f, 1 - cy);				// Texture Coord (Top Right)
                    glVertex2f(16 * Scale, 16 * Scale);				// Vertex Coord (Top Right)
                    glTexCoord2f(cx, 1 - cy);						// Texture Coord (Top Left)
                    glVertex2f(0, 16 * Scale);						// Vertex Coord (Top Left)
                glEnd();											// Done Building Our Quad (Character)
                glTranslated(10*Scale,0,0);							// Move To The Right Of The Character
            glEndList();											// Done Building The Display List
        }															// Loop Until All 256 Are Built
    }
}

void glFont::glPrintf(GLint x, GLint y, GLint set, const char *Format, ...)
{
    char		text[256];											// Holds Our String
    va_list		ap;													// Pointer To List Of Arguments

    if (Format == NULL)												// If There's No Text
    {
        return;														// Do Nothing
    }

    va_start(ap, Format);											// Parses The String For Variables
        vsprintf(text, Format, ap);									// And Converts Symbols To Actual Numbers
    va_end(ap);														// Results Are Stored In Text

    if (set>1)
    {
        set=1;
    }

    glEnable(GL_TEXTURE_2D);										// Enable 2d Textures
    glEnable(GL_BLEND);												// Enable Blending
    glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
    glBindTexture(GL_TEXTURE_2D, m_FontTexture);					// Select Our Font Texture
    glDisable(GL_DEPTH_TEST);										// Disables Depth Testing
    glMatrixMode(GL_PROJECTION);									// Select The Projection Matrix
    glPushMatrix();													// Store The Projection Matrix
    glLoadIdentity();												// Reset The Projection Matrix
    glOrtho(0,m_WindowWidth,0,m_WindowHeight,-1,1);					// Set Up An Ortho Screen
    glMatrixMode(GL_MODELVIEW);										// Select The Modelview Matrix
    glPushMatrix();													// Store The Modelview Matrix
    glLoadIdentity();												// Reset The Modelview Matrix
    glTranslated(x,y,0);											// Position The Text (0,0 - Bottom Left)
    glListBase(m_ListBase-32+(128*set));							// Choose The Font Set (0 or 1)
    glCallLists(strlen(text),GL_BYTE,text);							// Write The Text To The Screen
    glMatrixMode(GL_PROJECTION);									// Select The Projection Matrix
    glPopMatrix();													// Restore The Old Projection Matrix
    glMatrixMode(GL_MODELVIEW);										// Select The Modelview Matrix
    glPopMatrix();													// Restore The Old Projection Matrix
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void glFont::SetWindowSize(GLint width, GLint height)
{
    m_WindowWidth = width;											// Set the window size width
    m_WindowHeight = height;										// Set the window size height
}

GLuint glFont::GetTexture()
{
    GLuint result = m_FontTexture;									// Return the currently set texture
    return(result);
}

GLuint glFont::GetListBase()
{
    GLuint result = m_ListBase;										// Return the curretnly set list base
    return(result);
}

MyGLWidget::MyGLWidget(QWidget *parent) :
    QGLWidget(parent), m_show_full_screen(false), m_timeCounter(0),
    m_qobj(NULL), m_gFrames(0), m_gFPS(0), m_infoOn(false)
{
    showNormal();
    startTimer(15);
}

MyGLWidget::~MyGLWidget()
{
    gluDeleteQuadric(m_qobj);							// Delete our quadric object
    glDeleteLists(m_cylList,1);						// Delete the cylinder list
}

//下面的代码的作用是重新设置OpenGL场景的大小，而不管窗口的大小是否已经改变(假定您没有使用全屏模式)。
//甚至您无法改变窗口的大小时(例如您在全屏模式下)，它至少仍将运行一次--在程序开始时设置我们的透视图。
//OpenGL场景的尺寸将被设置成它显示时所在窗口的大小。
void MyGLWidget::resizeGL(int w, int h)
{
    m_gCamera.m_WindowHeight = h;								// The camera needs to know the window height
    m_gCamera.m_WindowWidth = w;								// The camera needs to know the window width

    if (h==0)													// Prevent A Divide By Zero By
    {
        h=1;													// Making Height Equal One
    }

    glViewport(0,0,w,h);									// Reset The Current Viewport

    glMatrixMode(GL_PROJECTION);									// Select The Projection Matrix
    glLoadIdentity();												// Reset The Projection Matrix

    // Calculate The Aspect Ratio Of The Window
    gluPerspective(45.0f,(GLfloat)w/(GLfloat)h,1.0f,1000.0f);

    glMatrixMode(GL_MODELVIEW);										// Select The Modelview Matrix
    glLoadIdentity();												// Reset The Modelview Matrix
}

//接下的代码段中，我们将对OpenGL进行所有的设置。我们将设置清除屏幕所用的颜色，打开深度缓存，启用smooth shading(阴影平滑)，等等。
//这个例程直到OpenGL窗口创建之后才会被调用。此过程将有返回值。但我们此处的初始化没那么复杂，现在还用不着担心这个返回值。
void MyGLWidget::initializeGL()
{
    GLuint tex=0;

    glShadeModel(GL_SMOOTH);										// Enable Smooth Shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);							// Black Background
    glClearDepth(1.0f);												// Depth Buffer Setup
    glEnable(GL_DEPTH_TEST);										// Enables Depth Testing
    glDepthFunc(GL_LEQUAL);											// The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);				// Really Nice Perspective Calculations

    loadTexture(":/image/Font.bmp", tex);							// Load the font texture
    if(tex != 0)													// Make sure it was loaded
    {
        m_gFont.SetFontTexture(tex);									// Set the font texture
        m_gFont.SetWindowSize(1024, 768);								// The font class needs to know the window size
        m_gFont.BuildFont(1.0f);					                    // Build the font
    }
    else
    {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Failed to load font texture."));
    }

    m_gCamera.m_MaxHeadingRate = 1.0f;								// Set our Maximum rates for the camera
    m_gCamera.m_MaxPitchRate = 1.0f;									// Set our Maximum rates for the camera
    m_gCamera.m_HeadingDegrees = 0.0f;								// Set our Maximum rates for the camera

    // Try and load the HardGlow texture tell the user if we can't find it then quit
    loadTexture(":/image/HardGlow2.bmp", m_gCamera.m_GlowTexture);
    if(m_gCamera.m_GlowTexture == 0)
    {
        QMessageBox::warning(this, tr("Warning"), tr("Failed to load Hard Glow texture."));
    }

    // Try and load the BigGlow texture tell the user if we can't find it then quit
    loadTexture(":/image/BigGlow3.bmp", m_gCamera.m_BigGlowTexture);
    if(m_gCamera.m_BigGlowTexture == 0)
    {
        QMessageBox::warning(this, tr("Warning"), tr("Failed to load Big Glow texture."));
    }

    // Try and load the Halo texture tell the user if we can't find it then quit
    loadTexture(":/image/Halo3.bmp", m_gCamera.m_HaloTexture);
    if(m_gCamera.m_HaloTexture == 0)
    {
        QMessageBox::warning(this, tr("Warning"), tr("Failed to load Halo texture."));
    }

    // Try and load the Streaks texture tell the user if we can't find it then quit
    loadTexture(":/image/Streaks4.bmp", m_gCamera.m_StreakTexture);
    if(m_gCamera.m_StreakTexture == 0)
    {
        QMessageBox::warning(this, tr("Warning"), tr("Failed to load Streaks texture."));
    }

    //##################  NEW STUFF  ##################################

    // Just create a cylinder that will be used as occluder object
    m_cylList = glGenLists(1);
    m_qobj = gluNewQuadric();
    gluQuadricDrawStyle(m_qobj, GLU_FILL);
    gluQuadricNormals(m_qobj, GLU_SMOOTH);
    glNewList(m_cylList, GL_COMPILE);
        glEnable(GL_COLOR_MATERIAL);
        glColor3f(0.0f, 0.0f, 1.0f);
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHTING);
        glTranslatef(0.0f,0.0f,-2.0f);
        gluCylinder(m_qobj, 0.5, 0.5, 4.0, 15, 5);
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
        glDisable(GL_COLOR_MATERIAL);
    glEndList();

    m_gStartTime = m_timeCounter;
}

void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				// Clear Screen And Depth Buffer
    glLoadIdentity();												// Reset The Current Modelview Matrix

    // We want our light source to be 50 units if front
    // of the camera all the time to make it look like
    // it is infinately far away from the camera. We only
    // do this to the z coordinate because we want to see
    // the flares adjust if we fly in a straight line.
    m_gCamera.m_LightSourcePos.z = m_gCamera.m_Position.z - 50.0f;

    //##################### NEW STUFF ##########################
    // Draw our cylinder and make it "do something"
    // Of course we do that BEFORE testing for occlusion
    // We need our depth buffer to be filled to check against occluder objects
    glPushMatrix();
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -20.0f);
        glRotatef(m_timeCounter / 50.0f, 0.3f, 0.0f, 0.0f);
        glRotatef(m_timeCounter / 50.0f, 0.0f, 0.5f, 0.0f);
        glCallList(m_cylList);
    glPopMatrix();

    m_gCamera.SetPrespective();										// Set our perspective/oriention on the world
    m_gCamera.RenderLensFlare();									// Render the lens flare
    m_gCamera.UpdateFrustumFaster();								// Update the frustum as fast as possible.

    if(m_infoOn == true)
    {                                                               // Check to see if info has been toggled by 1,2
        drawGLInfo();												// Info is on so draw the GL information.
    }
}

void MyGLWidget::drawGLInfo()
{
    GLfloat modelMatrix[16];									// This will hold the model view matrix
    GLfloat projMatrix[16];										// This will hold the projection matrix
    GLfloat diffTime;											// This is will contain the difference in time
    char string[64];											// A temporary string to use to format information
                                                                // that will be printed to the screen.

    glGetFloatv(GL_PROJECTION_MATRIX, projMatrix);				// Grab the projection matrix
    glGetFloatv(GL_MODELVIEW_MATRIX, modelMatrix);				// Grab the modelview matrix

    // Print out the cameras position
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    sprintf(string, "m_Position............. = %.02f, %.02f, %.02f", m_gCamera.m_Position.x, m_gCamera.m_Position.y, m_gCamera.m_Position.z);
    m_gFont.glPrintf(10, 720, 1, string);

    // Print out the cameras direction
    sprintf(string, "m_DirectionVector...... = %.02f, %.02f, %.02f", m_gCamera.m_DirectionVector.i, m_gCamera.m_DirectionVector.j, m_gCamera.m_DirectionVector.k);
    m_gFont.glPrintf(10, 700, 1, string);

    // Print out the light sources position
    sprintf(string, "m_LightSourcePos....... = %.02f, %.02f, %.02f", m_gCamera.m_LightSourcePos.x, m_gCamera.m_LightSourcePos.y, m_gCamera.m_LightSourcePos.z);
    m_gFont.glPrintf(10, 680, 1, string);

    // Print out the intersection point
    sprintf(string, "ptIntersect............ = %.02f, %.02f, %.02f", m_gCamera.ptIntersect.x, m_gCamera.ptIntersect.y, m_gCamera.ptIntersect.x);
    m_gFont.glPrintf(10, 660, 1, string);

    // Print out the vector that points from the light source to the camera
    sprintf(string, "vLightSourceToCamera... = %.02f, %.02f, %.02f", m_gCamera.vLightSourceToCamera.i, m_gCamera.vLightSourceToCamera.j, m_gCamera.vLightSourceToCamera.k);
    m_gFont.glPrintf(10, 640, 1, string);

    // Print out the vector that points from the light source to the intersection point.
    sprintf(string, "vLightSourceToIntersect = %.02f, %.02f, %.02f", m_gCamera.vLightSourceToIntersect.i, m_gCamera.vLightSourceToIntersect.j, m_gCamera.vLightSourceToIntersect.k);
    m_gFont.glPrintf(10, 620, 1, string);

    // Let everyone know the below matrix is the model view matrix
    sprintf(string, "GL_MODELVIEW_MATRIX");
    m_gFont.glPrintf(10, 580, 1, string);

    // Print out row 1 of the model view matrix
    sprintf(string, "%.02f, %.02f, %.02f, %.02f", modelMatrix[0], modelMatrix[1], modelMatrix[2], modelMatrix[3]);
    m_gFont.glPrintf(10, 560, 1, string);

    // Print out row 2 of the model view matrix
    sprintf(string, "%.02f, %.02f, %.02f, %.02f", modelMatrix[4], modelMatrix[5], modelMatrix[6], modelMatrix[7]);
    m_gFont.glPrintf(10, 540, 1, string);

    // Print out row 3 of the model view matrix
    sprintf(string, "%.02f, %.02f, %.02f, %.02f", modelMatrix[8], modelMatrix[9], modelMatrix[10], modelMatrix[11]);
    m_gFont.glPrintf(10, 520, 1, string);

    // Print out row 4 of the model view matrix
    sprintf(string, "%.02f, %.02f, %.02f, %.02f", modelMatrix[12], modelMatrix[13], modelMatrix[14], modelMatrix[15]);
    m_gFont.glPrintf(10, 500, 1, string);

    // Let everyone know the below matrix is the projection matrix
    sprintf(string, "GL_PROJECTION_MATRIX");
    m_gFont.glPrintf(10, 460, 1, string);

    // Print out row 1 of the projection view matrix
    sprintf(string, "%.02f, %.02f, %.02f, %.02f", projMatrix[0], projMatrix[1], projMatrix[2], projMatrix[3]);
    m_gFont.glPrintf(10, 440, 1, string);

    // Print out row 2 of the projection view matrix
    sprintf(string, "%.02f, %.02f, %.02f, %.02f", projMatrix[4], projMatrix[5], projMatrix[6], projMatrix[7]);
    m_gFont.glPrintf(10, 420, 1, string);

    // Print out row 3 of the projection view matrix
    sprintf(string, "%.02f, %.02f, %.03f, %.03f", projMatrix[8], projMatrix[9], projMatrix[10], projMatrix[11]);
    m_gFont.glPrintf(10, 400, 1, string);

    // Print out row 4 of the projection view matrix
    sprintf(string, "%.02f, %.02f, %.03f, %.03f", projMatrix[12], projMatrix[13], projMatrix[14], projMatrix[15]);
    m_gFont.glPrintf(10, 380, 1, string);

    // Let everyone know the below values are the Frustum clipping planes
    m_gFont.glPrintf(10, 320, 1, "FRUSTUM CLIPPING PLANES");

    // Print out the right clipping plane
    sprintf(string, "%.02f, %.02f, %.02f, %.02f", m_gCamera.m_Frustum[0][0], m_gCamera.m_Frustum[0][1], m_gCamera.m_Frustum[0][2], m_gCamera.m_Frustum[0][3]);
    m_gFont.glPrintf(10, 300, 1, string);

    // Print out the left clipping plane
    sprintf(string, "%.02f, %.02f, %.02f, %.02f", m_gCamera.m_Frustum[1][0], m_gCamera.m_Frustum[1][1], m_gCamera.m_Frustum[1][2], m_gCamera.m_Frustum[1][3]);
    m_gFont.glPrintf(10, 280, 1, string);

    // Print out the bottom clipping plane
    sprintf(string, "%.02f, %.02f, %.02f, %.02f", m_gCamera.m_Frustum[2][0], m_gCamera.m_Frustum[2][1], m_gCamera.m_Frustum[2][2], m_gCamera.m_Frustum[2][3]);
    m_gFont.glPrintf(10, 260, 1, string);

    // Print out the top clipping plane
    sprintf(string, "%.02f, %.02f, %.02f, %.02f", m_gCamera.m_Frustum[3][0], m_gCamera.m_Frustum[3][1], m_gCamera.m_Frustum[3][2], m_gCamera.m_Frustum[3][3]);
    m_gFont.glPrintf(10, 240, 1, string);

    // Print out the far clipping plane
    sprintf(string, "%.02f, %.02f, %.02f, %.02f", m_gCamera.m_Frustum[4][0], m_gCamera.m_Frustum[4][1], m_gCamera.m_Frustum[4][2], m_gCamera.m_Frustum[4][3]);
    m_gFont.glPrintf(10, 220, 1, string);

    // Print out the near clipping plane
    sprintf(string, "%.02f, %.02f, %.02f, %.02f", m_gCamera.m_Frustum[5][0], m_gCamera.m_Frustum[5][1], m_gCamera.m_Frustum[5][2], m_gCamera.m_Frustum[5][3]);
    m_gFont.glPrintf(10, 200, 1, string);

    if(m_gFrames >= 100)											// if we are due for another FPS update
    {
        qlonglong gCurrentTime = m_timeCounter;// Get the current time
        diffTime = GLfloat(gCurrentTime - m_gStartTime);			// Find the difference between the start and end times
        m_gFPS = (m_gFrames / diffTime) * 1000.0f;					// Compute the FPS
        m_gStartTime = gCurrentTime;							// Set the current start time to the current time
        m_gFrames = 1;											// Set the number of frames to 1
    }
    else
    {
        m_gFrames++;												// We are not due to for another update so add one to the frame count
    }

    // Print out the FPS
    sprintf(string, "FPS %.02f", m_gFPS);
    m_gFont.glPrintf(10, 160, 1, string);
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
        case Qt::Key_W:
        {
            m_gCamera.ChangePitch(-0.2f);								// Pitch the camera up 0.2 degrees
            break;
        }
        case Qt::Key_S:
        {
            m_gCamera.ChangePitch(0.2f);								// Pitch the camera down 0.2 degrees
            break;
        }
        case Qt::Key_D:
        {
            m_gCamera.ChangeHeading(0.2f);							// Yaw the camera to the left
            break;
        }
        case Qt::Key_A:
        {
            m_gCamera.ChangeHeading(-0.2f);							// Yaw the camera to the right
            break;
        }
        case Qt::Key_Z:
        {
            m_gCamera.m_ForwardVelocity = 0.01f;						// Start moving the camera forward 0.01 units every frame
            break;
        }
        case Qt::Key_C:
        {
            m_gCamera.m_ForwardVelocity = -0.01f;						// Start moving the camera backwards 0.01 units every frame
            break;
        }
        case Qt::Key_X:
        {
            m_gCamera.m_ForwardVelocity = 0.0f;						// Stop the camera from moving.
            break;
        }
        case Qt::Key_1:
        {
            m_infoOn = true;											// Toggle info on
            break;
        }
        case Qt::Key_2:
        {
            m_infoOn = false;
            break;
        }
    }
    QGLWidget::keyPressEvent(event);
}

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    updateGL();
    m_timeCounter += 15;
    QGLWidget::timerEvent(event);
}

void MyGLWidget::loadTexture(const QString &filePath, GLuint &texId)
{
    QImage image(filePath);
    image = image.convertToFormat(QImage::Format_RGB888);
    image = image.mirrored();

    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width(), image.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, image.bits());
}

//可以使用w,s,a,d变换摄像机的方向，1，2显示/关闭各种信息参数。C给摄像机一个固定的速度，X停止它。
//上面就是这个教程的全部了，所有的问题，评论和抱怨都欢迎。当然我不是第一个作这个效果的人，下面是其他方面相关的文章:
//http://www.gamedev.net/reference/articles/article874.asp
//http://www.gamedev.net/reference/articles/article813.asp
//http://www.opengl.org/developers/code/mjktips/lensflare/
//http://www.markmorley.com/opengl/frustumculling.html
//http://oss.sgi.com/projects/ogl-sample/registry/HP/occlusion_test.txt
//http://oss.sgi.com/projects/ogl-sample/registry/NV/occlusion_query.txt
