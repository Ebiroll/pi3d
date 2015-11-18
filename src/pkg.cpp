
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "pkg.h"
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "camera.h"

extern GLuint VBO, VAO;
extern int mdl_index_count;



// Loads pkg file
void loadPkg(char *filename,Camera &camera)
{


}


// Custom format (mdl file)
void loadMdl(unsigned char*read_pos,unsigned int length)
{
  mdl_lod1Header_t *header=(mdl_lod1Header_t *)read_pos;

  //}

  printf("X  %.2f - %.2f\n",header->_abb[0].x,header->_abb[1].x);
  printf("Y  %.2f - %.2f\n",header->_abb[0].y,header->_abb[1].y);
  printf("Z  %.2f - %.2f\n",header->_abb[0].z,header->_abb[1].z);
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  read_pos+=sizeof(mdl_lod1Header_t);

  uint32_t buffer_size;
  buffer_size=*(uint32_t *)read_pos;
  //fread(&buffer_size, 4,1,file);


  printf("buffer size/sizeof(pixel_data) %d\n",buffer_size/sizeof(Vertex_t));

  //int pos=ftell(file);
  read_pos+=4;

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

  glBindVertexArray(0);



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


    //loadMdl(&data[0],size);


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

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

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

    glBindVertexArray(0);

}

