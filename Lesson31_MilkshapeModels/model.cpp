#include "model.h"

GLuint LoadGLTexture( char *filename )	// Load Bitmaps And Convert To Textures
{
    QString filePath = QApplication::applicationDirPath() + "/" + QString(filename);
    QImage image(filePath);// // Loads The Bitmap Specified By filename
    image = image.convertToFormat(QImage::Format_RGB888);
    image = image.mirrored();
    GLuint texture = 0;//Texture ID
    if(!image.isNull())//Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit
    {
        glGenTextures(1, &texture);

        // Typical Texture Generation Using Data From The Bitmap
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width(), image.height(),
                     0, GL_RGB, GL_UNSIGNED_BYTE, image.bits());
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    }
    return texture;// Return The Status
}

Model::Model()
{
	m_numMeshes = 0;
	m_pMeshes = NULL;
	m_numMaterials = 0;
	m_pMaterials = NULL;
	m_numTriangles = 0;
	m_pTriangles = NULL;
	m_numVertices = 0;
	m_pVertices = NULL;
}

Model::~Model()
{
	for (int i = 0; i < m_numMeshes; i++ )
    {
        delete[] m_pMeshes[i].m_pTriangleIndices;
    }
	for (int i = 0; i < m_numMaterials; i++ )
    {
        delete[] m_pMaterials[i].m_pTextureFilename;
    }
	m_numMeshes = 0;
	if ( m_pMeshes != NULL )
	{
		delete[] m_pMeshes;
		m_pMeshes = NULL;
	}
	m_numMaterials = 0;
	if ( m_pMaterials != NULL )
	{
		delete[] m_pMaterials;
		m_pMaterials = NULL;
	}
	m_numTriangles = 0;
	if ( m_pTriangles != NULL )
	{
		delete[] m_pTriangles;
		m_pTriangles = NULL;
	}
	m_numVertices = 0;
	if ( m_pVertices != NULL )
	{
		delete[] m_pVertices;
		m_pVertices = NULL;
	}
}

//有了数据，就可以写出绘制函数了，下面的函数根据模型的信息，按网格分组，分别绘制每一组的数据。
void Model::draw() 
{
	GLboolean texEnabled = glIsEnabled( GL_TEXTURE_2D );
    // 按网格分组绘制
	for ( int i = 0; i < m_numMeshes; i++ )
	{
		int materialIndex = m_pMeshes[i].m_materialIndex;
		if ( materialIndex >= 0 )
		{
			glMaterialfv( GL_FRONT, GL_AMBIENT, m_pMaterials[materialIndex].m_ambient );
			glMaterialfv( GL_FRONT, GL_DIFFUSE, m_pMaterials[materialIndex].m_diffuse );
			glMaterialfv( GL_FRONT, GL_SPECULAR, m_pMaterials[materialIndex].m_specular );
			glMaterialfv( GL_FRONT, GL_EMISSION, m_pMaterials[materialIndex].m_emissive );
			glMaterialf( GL_FRONT, GL_SHININESS, m_pMaterials[materialIndex].m_shininess );
			if ( m_pMaterials[materialIndex].m_texture > 0 )
			{
				glBindTexture( GL_TEXTURE_2D, m_pMaterials[materialIndex].m_texture );
				glEnable( GL_TEXTURE_2D );
			}
			else
            {
                glDisable( GL_TEXTURE_2D );
            }
		}
		else
		{
			// Material properties?
			glDisable( GL_TEXTURE_2D );
		}

		glBegin( GL_TRIANGLES );
		{
			for ( int j = 0; j < m_pMeshes[i].m_numTriangles; j++ )
			{
				int triangleIndex = m_pMeshes[i].m_pTriangleIndices[j];
				const Triangle* pTri = &m_pTriangles[triangleIndex];

				for ( int k = 0; k < 3; k++ )
                {
					int index = pTri->m_vertexIndices[k];
					glNormal3fv( pTri->m_vertexNormals[k] );
					glTexCoord2f( pTri->m_s[k], pTri->m_t[k] );
					glVertex3fv( m_pVertices[index].m_location );
				}
			}
		}
		glEnd();
	}
    
	if ( texEnabled )
    {
        glEnable( GL_TEXTURE_2D );
    }
	else
    {
        glDisable( GL_TEXTURE_2D );
    }
}

void Model::reloadTextures()
{
    //设置好了一切参数，但纹理还没有载入内存，下面的代码完成这个功能
	for ( int i = 0; i < m_numMaterials; i++ )
    {
        if ( strlen( m_pMaterials[i].m_pTextureFilename ) > 0 )
        {
            m_pMaterials[i].m_texture = LoadGLTexture( m_pMaterials[i].m_pTextureFilename );
        }
        else
        {
            m_pMaterials[i].m_texture = 0;
        }
    }
}
