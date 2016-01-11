#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "camera.h"
#include "SOIL.h"
#include <vector>
#include <string>
#include <stdlib.h>
#include "hash.h"


#include <iostream>
#include <string>
#include <vector>
#include <memory>

using std::vector;
using std::string;


#include "EGL/egl.h"
//#include "GLES/gl.h"
#include "GLES2/gl2.h"

// Should be defined somewhere
#ifndef GLAPIENTRY
#define GLAPIENTRY
#endif

extern "C" {
#include "eglstate.h"
}


//#include "GLES2/gl2ext.h"

#include "pkg.h"

extern int mdl_index_count;

typedef struct {
    GLuint    textureID;
    uint32_t  hash;
} TextureHashMap;

std::vector<TextureHashMap>  loadedImages;


void loadMdl(unsigned char*read_pos,unsigned int length);


GLint TextureFromData(const unsigned char* data,unsigned int length)
{
    //Generate texture ID and load texture data
    GLuint textureID;

    glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );

    glGenTextures(1, &textureID);
    int width,height;
    int numChannels=0;

    unsigned char* image = SOIL_load_image_from_memory(data,length, &width, &height, &numChannels, SOIL_LOAD_RGB);
    if (image==NULL)
    {
        printf("Failed to load texture %s",SOIL_last_result());
    }
    // Assign texture to ID
    glBindTexture(GL_TEXTURE_2D, textureID);
    //glTexImage2D(GL_TEXTURE_2D, 0, gamma ? GL_SRGB : GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Parameters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);
    return textureID;
}

struct SlumEntry
{
    char file[32];
    uint32_t size;
    uint32_t offset;
} ;


struct IndexVertex
{
    float pos[3];
    float norm[3];
    float tex[2];
    int32_t index;
};


// Custom format (mdl file)
void loadPkgMdl(unsigned char*read_pos,unsigned int length,mdlGLData *GLdata)
{
  mdl_lod1Header_t *header=(mdl_lod1Header_t *)read_pos;


  printf("X  %.2f - %.2f\n",header->_abb[0].x,header->_abb[1].x);
  printf("Y  %.2f - %.2f\n",header->_abb[0].y,header->_abb[1].y);
  printf("Z  %.2f - %.2f\n",header->_abb[0].z,header->_abb[1].z);
  //glGenVertexArraysOES(1, &VAO);
  //glBindVertexArrayOES(VAO);

  read_pos+=sizeof(mdl_lod1Header_t);

  printf("rend_hash=%d\n",header->render_hash);
  printf("text_hash=%d\n",header->texture_hash);

  uint32_t buffer_size;
  buffer_size=*(uint32_t *)read_pos;
  //fread(&buffer_size, 4,1,file);

  buffer_size=header->texture_hash;
  printf("buffer %d size/sizeof(pixel_data) %d\n",buffer_size,buffer_size/sizeof(Vertex_t));

  for (int i = 0; i < 16; i ++) {
          printf(" %2x", read_pos[i]);
  }

  printf("length=%d\n",length);

  //exit(1);

  //read_pos+=4;


  GLuint vb;
  glGenBuffers(1, &vb);
  glBindBuffer(GL_ARRAY_BUFFER, vb);
  glBufferData(GL_ARRAY_BUFFER, buffer_size, read_pos, GL_STATIC_DRAW);

  //fseek(file,pos + buffer_size,SEEK_SET);
  //fread(&buffer_size, 4,1,file);
  read_pos+=buffer_size;

  buffer_size=*(uint32_t *)read_pos;

  mdl_index_count = buffer_size/2 ;

  printf("index count %d\n",mdl_index_count);

  //pos=ftell(file);
  read_pos+=4;

  GLuint ib;
  glGenBuffers(1, &ib);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer_size,  read_pos, GL_STATIC_DRAW);

  //fseek(file,pos + buffer_size,SEEK_SET);
  //s->seek(s->offset() + buffer_size);



  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(IndexVertex), 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(IndexVertex), (char*)0 + 12);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(IndexVertex), (char*)0 + 24);
  glVertexAttribPointer(3, 1, GL_INT, GL_FALSE, sizeof(IndexVertex), (char*)0 + 32);

  //glBindVertexArrayOES(0);

}


// Loads pkg file, returns number of loaded elements
int loadPkg(char *filename,Camera &camera,mdlGLData *GLdata,int numElem)
{
   PKG_content *my_content;
   int ret=0;

   FILE *file = fopen(filename, "rb");

   if (file==NULL)
   {
       printf("Failed open file %s for reading ",filename);
       exit(1);
   }


   fseek(file, 0, SEEK_END);
   uint32_t size = ftell(file);
   fseek(file, 0, SEEK_SET);
   std::vector<uint8_t> data;
   data.resize(size);
   fread(&data[0], size, 1, file);

   my_content=(PKG_content *) &data[0];

   printf("tot_siz=%d\n",size);



   printf("siz=%d\n",4+ my_content->n_files* sizeof(SlumEntry));

   int numF=my_content->n_files;
   for (int ix=0;ix<numF;ix++)
   {
       //printf("%s\n",my_content->files[ix].file);
       //printf("off=%d\n",my_content->files[ix].offset);
       //printf("siz=%d\n",my_content->files[ix].size);
       //printf("nxt=%d\n",my_content->files[ix].offset + my_content->files[ix].size);

       char *pExt = strrchr(my_content->files[ix].file, '.');
       if (pExt != NULL)
       {
          if (strcmp(pExt,".mdl")==0)
          {
              loadPkgMdl(&data[my_content->files[ix].offset],my_content->files[ix].size,GLdata);
              GLdata++;
              ret++;
          }
          if (strcmp(pExt,".dds")==0)
          {
              GLuint id=TextureFromData(&data[my_content->files[ix].offset],my_content->files[ix].size);
              TextureHashMap tmp;

              Hash_key namehash(my_content->files[ix].file);
              tmp.hash=namehash;
              tmp.textureID=id;

              printf("Stored image %s hash %u as %d\n",my_content->files[ix].file,tmp.hash,tmp.textureID);

              loadedImages.push_back(tmp);


          }
      }
   }

   return ret;
}





// Custom format (mdl file)
void loadMdl(unsigned char*read_pos,unsigned int length)
{
  mdl_lod1Header_t *header=(mdl_lod1Header_t *)read_pos;


  printf("X  %.2f - %.2f\n",header->_abb[0].x,header->_abb[1].x);
  printf("Y  %.2f - %.2f\n",header->_abb[0].y,header->_abb[1].y);
  printf("Z  %.2f - %.2f\n",header->_abb[0].z,header->_abb[1].z);
  //glGenVertexArraysOES(1, &VAO);
  //glBindVertexArrayOES(VAO);

  //read_pos+=sizeof(mdl_lod1Header_t);

  printf("rend_hash=%d\n",header->render_hash);
  printf("text_hash=%d\n",header->texture_hash);

  uint32_t buffer_size;
  buffer_size=*(uint32_t *)read_pos;
  //fread(&buffer_size, 4,1,file);
  printf("buffer %d size/sizeof(pixel_data) %d\n",buffer_size,buffer_size/sizeof(Vertex_t));


  for (int i = 0; i < 16; i ++) {
          printf(" %2x", read_pos[i]);
  }

  printf("length=%d\n",length);

   exit(1);

  //int pos=ftell(file);
  read_pos+=4;


  //buffer_size=*(uint32_t *)read_pos;


  GLuint vb;
  glGenBuffers(1, &vb);
  glBindBuffer(GL_ARRAY_BUFFER, vb);
  glBufferData(GL_ARRAY_BUFFER, buffer_size, read_pos, GL_STATIC_DRAW);

  //fseek(file,pos + buffer_size,SEEK_SET);
  //fread(&buffer_size, 4,1,file);
  read_pos+=buffer_size;

  buffer_size=*(uint32_t *)read_pos;

  mdl_index_count = buffer_size/2 ;

  printf("index count %d\n",mdl_index_count);

  //pos=ftell(file);
  read_pos+=4;

  GLuint ib;
  glGenBuffers(1, &ib);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer_size,  read_pos, GL_STATIC_DRAW);

  //fseek(file,pos + buffer_size,SEEK_SET);
  //s->seek(s->offset() + buffer_size);



  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char*)0 + 12);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char*)0 + 24);

  //glBindVertexArrayOES(0);



}



// Custom format (mdl file)
GLuint loadSimple(char *filename,Camera &camera,STATE_T* state,mdlGLData *GLdata)
{

    //std::string totfilename=std::string(filename);

    FILE *file = fopen(filename, "rb");

    if (file==NULL)
    {
        printf("Failed open file %s for reading ",filename);
        exit(1);
    }


    fseek(file, 0, SEEK_END);
    uint32_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    std::vector<uint8_t> data;
    data.resize(size);
    fread(&data[0], size, 1, file);
    mdl_lod1Header_t *test_header=(mdl_lod1Header_t *)&data[0];


    fclose(file);
    file=fopen(filename, "rb");


    // printf("Heder size= %d",sizeof(mdl_lod1Header_t));

    unsigned char*read_pos=&data[0];


    mdl_lod1Header_t header;
    fread(&header,sizeof(mdl_lod1Header_t),1,file);

    //if (header.nlods!=1) {
    //    printf("Only nlods ==1 supported\n");
    //    exit(-1);
    //}

    printf("X  %.2f - %.2f\n",header._abb[0].x,header._abb[1].x);
    printf("Y  %.2f - %.2f\n",header._abb[0].y,header._abb[1].y);
    printf("Z  %.2f - %.2f\n",header._abb[0].z,header._abb[1].z);

    glm::vec3 tmp(0.0f-(header._abb[0].x+header._abb[1].x)/2,
                  0.0f-(header._abb[0].y+header._abb[1].y)/2,
                  0.0f-(header._abb[0].z+header._abb[1].z)/2);

    //center=tmp;

    Camera ca(glm::vec3(0.0f,0.0f,80.0f));
    camera=ca;


    // Part of header
    //uint32_t texture_hash = 0;
    //fread(&texture_hash, 4,1,file);
    //std::shared_ptr<Texture> sp_texture = engine::texture_manager->fetch_texture(texture_hash);
    //re_simple->texture = sp_texture->texture_id();

    //glGenVertexArraysOES(1, &VAO);
    //glBindVertexArrayOES(VAO);

    uint32_t buffer_size;
    fread(&buffer_size, 4,1,file);

    printf("buffer size/sizeof(pixel_data) %d\n",buffer_size/sizeof(Vertex_t));

    int numVertexes=buffer_size/sizeof(Vertex_t);

    int pos=ftell(file);


    fseek(file,pos + buffer_size,SEEK_SET);
    fread(&buffer_size, 4,1,file);

    mdl_index_count = buffer_size/2 ;
    numVertexes=mdl_index_count;

    printf("index count %d\n",mdl_index_count);

    int pos_indexes=ftell(file);


    float *buff_data= (float *)malloc(sizeof(float)*3*numVertexes+1);
    float *uv_data= (float *)malloc(sizeof(float)*2*numVertexes+1);
    GLushort *indexData=(GLushort *)malloc(sizeof(GLushort)*numVertexes+1);

    Vertex_t *ptr= (Vertex_t *) &data[pos];
    unsigned short int *indexes = (unsigned short int *) &data[pos_indexes];
    int j=0;
    for (j=0;j<(mdl_index_count*3);j+=3)
    {
        //printf("%d,",*indexes/3);
        //int myix=*indexes;
        int myix=j/3;
        indexData[myix]=*indexes;

        printf("X,Y,Z U,V = %.2f , %.2f , %.2f     %.2f,%.2f\n",ptr[myix].pos[0],ptr[myix].pos[1],ptr[myix].pos[2],ptr[myix].tex[0],ptr[myix].tex[1]);
        buff_data[j]=ptr[myix].pos[0];
        buff_data[j+1]=ptr[myix].pos[1];
        buff_data[j+2]=ptr[myix].pos[2];
        indexes++;
    }

    indexes = (unsigned short int *) &data[pos_indexes];
    for (j=0;j<(mdl_index_count*2);j+=2)
    {
        int myix=*indexes;
        uv_data[j]=ptr[myix].tex[0];
        uv_data[j+1]=ptr[myix].tex[1];
        indexes++;
    }

    //GLuint vb;
    glGenBuffers(1, &GLdata->dataVBO);
    glBindBuffer(GL_ARRAY_BUFFER, GLdata->dataVBO);
    glBufferData(GL_ARRAY_BUFFER, (numVertexes*sizeof(float)*3), buff_data, GL_STATIC_DRAW);


    glGenBuffers(1, &GLdata->uvVBO);
    glBindBuffer(GL_ARRAY_BUFFER, GLdata->uvVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*numVertexes, uv_data, GL_STATIC_DRAW);


    glGenBuffers(1, &GLdata->indexVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GLdata->indexVAO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*numVertexes*3, indexData, GL_STATIC_DRAW);



    return 1;

}

