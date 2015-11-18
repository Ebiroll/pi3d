#ifndef PKG_H
#define PKG_H
#include "camera.h"

// Loads mdl file
void loadSimple(char *filename,Camera &camera);

// Loads pkg file
void loadPkg(char *filename,Camera &camera);


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


typedef struct Vector3
{
    float x, y, z;
} Vector3_f;


#pragma pack(2)
typedef struct lod1
{
    Vector3_f _abb[2];
    uint16_t  lod[4];
    char      name[32];
    uint32_t  render_hash;
    uint32_t  texture_hash;
} mdl_lod1Header_t;






#endif // PKG_H
