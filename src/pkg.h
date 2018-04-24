#ifndef PKG_H
#define PKG_H
#include "camera.h"

extern "C" {
#include "eglstate.h"
}

// These are Opengl data loaded for each model/mesh
struct mdlGLData {
    GLuint dataVBO;
    GLuint uvVBO;
    GLuint indexVAO;
    GLuint numIndexes;
    GLuint textureIx;
};

// Loads mdl file, GLdata should contain pointer to data to be returned
GLuint loadSimple(char *filename,Camera &camera,STATE_T* state,mdlGLData *GLdata);

// Loads pkg file
int loadPkg(char *filename,Camera &camera,mdlGLData *GLdata,int numElem);

#define SIMPLE_HASH      2031172703
                       //2031172703
#define GENERAL_HASH       1929756412                          
#define BUILDING_HASH      955201417
#define AV_MODEL_HASH      3915210767
#define AV_CS_HASH         187990613
#define STATIC_DECAL_HASH  1181308877
#define TREE_HASH          2656397992



/////////////////////////////////////////////
#pragma pack(1)
struct PKG_content
{
    uint32_t n_files;
    struct Entry
    {
        char file[32];
        uint32_t size;
        uint32_t offset;
    } files[1];
};

typedef struct Vertex
{
        float pos[3];
        float norm[3];
        float tex[2];
} Vertex_t;


typedef struct SkinnedVertex
{
    float pos[3];
    float norm[3];
    float tex[2];
    int32_t index;
} Skinned_Vertex_t;


typedef struct Vector3
{
    float x, y, z;
} Vector3_f;


#pragma pack(1)
typedef struct lod1
{
    uint32_t  version;
    uint32_t  flags;
    Vector3_f _abb[2];
    uint16_t  lod[4];
    char      name[32];
    uint32_t  render_hash;
    uint32_t  texture_hash;
    uint32_t  attribs;
} mdl_lod1Header_t;



typedef struct avmod
{
    uint32_t  version;
    uint32_t  flags;
    Vector3_f _abb[2];
    uint16_t  lod[4];
    char      name[32];
    uint32_t  render_hash;
    uint32_t  attribs;
    uint32_t  texture_hash0;
    uint32_t  texture_hash1;
} av_mdlHeader_t;


typedef struct general
{
    uint32_t  version;
    uint32_t  flags;
    Vector3_f _abb[2];
    uint16_t  lod[4];
    char      name[32];
    uint32_t  render_hash;
    uint32_t  attribs;
    float     z_bias;
    uint32_t  texture_hash0;
    uint32_t  texture_hash1;
    uint32_t  texture_hash2;   
} General_t;


#endif // PKG_H
