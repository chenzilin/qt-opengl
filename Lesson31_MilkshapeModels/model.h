#ifndef MODEL_H
#define MODEL_H

#include <GL/gl.h>
#include <QImage>
#include <QApplication>

// 网格结构
struct Mesh
{
    int m_materialIndex;
    int m_numTriangles;
    int *m_pTriangleIndices;
};
// 材质属性
struct Material
{
    float m_ambient[4], m_diffuse[4], m_specular[4], m_emissive[4];
    float m_shininess;
    GLuint m_texture;
    char *m_pTextureFilename;
};
// 三角形结构
struct Triangle
{
    float m_vertexNormals[3][3];
    float m_s[3], m_t[3];
    int m_vertexIndices[3];
};
// 顶点结构
struct Vertex
{
    char m_boneID;	// 顶点所在的骨骼
    float m_location[3];
};

class Model
{
public:
    /*	Constructor. */
    Model();
    /*	Destructor. */
    virtual ~Model();
    /*
        Load the model data into the private variables.
            filename			Model filename
    */
    virtual bool loadModelData( char *filename ) = 0;
    /*
        Draw the model.
    */
    void draw();
    /*
        Called if OpenGL context was lost and we need to reload textures, display lists, etc.
    */
    void reloadTextures();
protected:
    // 使用的网格
    int m_numMeshes;
    Mesh *m_pMeshes;

    // 使用的纹理
    int m_numMaterials;
    Material *m_pMaterials;

    // 使用的三角形
    int m_numTriangles;
    Triangle *m_pTriangles;

    // 顶点的个数和数据
    int m_numVertices;
    Vertex *m_pVertices;
};

#endif // MODEL_H
