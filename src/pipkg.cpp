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


#include <iostream>
#include <string>
#include <vector>
#include <memory>

using std::vector;
using std::string;


#include "EGL/egl.h"
#include "GLES/gl.h"
#include "GLES2/gl2.h"

// Should be defined somewhere
#ifndef GLAPIENTRY
#define GLAPIENTRY
#endif

//#include "GLES2/gl2ext.h"

#include "pipkg.h"

extern GLuint VBO, VAO;
extern int mdl_index_count;



void loadMdl(unsigned char*read_pos,unsigned int length);


GLint TextureFromData(const unsigned char* data,unsigned int length)
{
    //Generate texture ID and load texture data
    GLuint textureID;
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
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
void loadPkgMdl(unsigned char*read_pos,unsigned int length)
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


// Loads pkg file
int loadPkg(char *filename,Camera &camera)
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
              if (strcmp(my_content->files[ix].file,"b733.mdl")==0)
              {
                  loadPkgMdl(&data[my_content->files[ix].offset],my_content->files[ix].size);
              }
          }
          if (strcmp(pExt,".dds")==0)
          {
              ret=TextureFromData(&data[my_content->files[ix].offset],my_content->files[ix].size);
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
void loadSimple(char *filename,Camera &camera)
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


    loadMdl(&data[0],size);
#if 0

    // This only works when nlod=1!!
    //assert(test_header->nlods==1);

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


    int pos=ftell(file);

    GLuint vb;
    glGenBuffers(1, &vb);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glBufferData(GL_ARRAY_BUFFER, buffer_size, &data[pos], GL_STATIC_DRAW);

    fseek(file,pos + buffer_size,SEEK_SET);
    fread(&buffer_size, 4,1,file);

    mdl_index_count = buffer_size/2 ;

    printf("index count %d\n",mdl_index_count);

    pos=ftell(file);

    GLuint ib;
    glGenBuffers(1, &ib);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer_size,  &data[pos], GL_STATIC_DRAW);

    fseek(file,pos + buffer_size,SEEK_SET);
    //s->seek(s->offset() + buffer_size);


    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char*)0 + 12);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char*)0 + 24);

    //glBindVertexArrayOES(0);
#endif

}
