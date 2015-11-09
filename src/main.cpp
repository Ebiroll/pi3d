
#include "Mesh.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include "SOIL.h"


// GL includes
#include "shader.h"
#include "camera.h"
////////////////// Default shaders ///////////
// Nice 2.2 tutorial
// http://duriansoftware.com/joe/An-intro-to-modern-OpenGL.-Chapter-2.2:-Shaders.html


#define GLSL(src) "#version 120\n" #src

/*
 *

   layout(location = 0) in vec3 vpos;
   layout(location = 1) in vec3 vnorm;
   layout(location = 2) in vec2 vtex;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(vpos, 1.0f);
    }


#version 130
layout(location = 0) in vec3 vertexPosition_modelspace;

// Setup Vertex Attributes  [NEW]
glBindAttribLocation (ProgramID, 0, "vertexPosition_modelspace");

//////////////////////////////////////

    layout(std140, column_major) uniform;

    in vec2 uv;
    in vec3 norm;
    uniform sampler2D tex;
    uniform float light;
    out vec4 frag_color;

    void main() {
        vec4 color = texture(tex, uv);
        vec3 l = vec3(1, 1, 0.7) * 1.0 * (1.0 - color.a);
        frag_color = vec4(clamp(color.rgb + l, 0, 1), 1.0);
        //frag_color = textureLod(tex, uv, 0);
        //frag_color = vec4(1);
    }


*/


const char* vs = GLSL(
        attribute vec3 position;
        attribute vec3 normal;
        attribute vec2 vtex;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        varying vec2 texcoord;

        void main()
        {
            gl_Position = projection * view * model * vec4(position, 1.0);
            texcoord = vec2(position.x , position.y)  + vec2(0.5);
            //texcoord  =  vtex;
        });

const char* fs = GLSL(
            varying vec2 texcoord;
            uniform sampler2D tex;

            void main()
            {
                gl_FragColor = texture2D(tex, texcoord);
                //gl_FragColor = vec4(0.0,1.0,0.0,1.0);
                //gl_FragColor = vec4(texcoord.x,texcoord.y,0.0,1.0);
            });




/////////////////////////////////////////////
#pragma pack(1)
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
   //uint16_t  nlods;
   uint16_t  lod[4];
   uint32_t  render_type;
   uint32_t  texture_hash;
   //uint32_t  buffer_size;
} mdl_lod1Header_t;


glm::vec3 center(0.0f, 0.0f, 0.0f);


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 80.0f));

bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
int screenWidth=800;
int screenHeigth=600;

// Backup texture
GLuint texture1;
GLuint VBO, VAO;
int mdl_index_count=-1;


GLfloat angleX=0.0;
GLfloat angleY=0.0;

bool rotating=true;


void loadSimple(char *filename)
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

    // This only works when nlod=1!!
    //assert(test_header->nlods==1);

    fclose(file);
    file=fopen(filename, "rb");

    printf("Render type %d\n",test_header->render_type);


    // printf("Heder size= %d",sizeof(mdl_lod1Header_t));

    unsigned char*read_pos=&data[0];


    mdl_lod1Header_t header;
    fread(&header,sizeof(mdl_lod1Header_t),1,file);

    if (header.render_type!=1) {
        printf("Only RT_SIMPLE is supported\n");
        exit(-1);
    }

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


//    layout (location = 0) in vec3 position;
//    layout (location = 1) in vec3 normal;
//   layout (location = 2) in vec2 texCoords;



    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char*)0 + 12);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char*)0 + 24);

    glBindVertexArray(0);


}



void printHelp(int argc, char *argv[]) {

    printf("Usage, %s [-t texture] [-s shader_base] modelname.*\n",argv[0]);

    printf("     -t   Texture to force load when loading a mdl file \n");
    printf("     -s   shaderfile will load and compile shaderfile.vert & shaderfile.frag\n");
    printf("     -h   This helptext\n");

}



int main(int argc, char* argv[])
{


  GLFWwindow* window;
  char *shader_base="shader";
  char vertex_shader[512];
  char fragment_shader[512];
  char geometry_shader[512];


  if (argc<2)
  {
      printHelp(argc,argv);
  }

  int ret=glfwInit();
   if (!ret)
   {
      std::cerr << "Init failed"  << ret << std::endl;
   }

   glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); // We want OpenGL 2.1 ??
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
   //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
   //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // If we don't want the old OpenGL
   //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


  window = glfwCreateWindow(screenWidth, screenHeigth, "Viewer", NULL, NULL);


#ifdef FULL
    GLFWmonitor* monitor=glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    window = glfwCreateWindow(mode->width, mode->height, "assviewer 0.1", monitor, NULL);
#endif


     if (!window)
     {
         glfwTerminate();
     }


   //glfwSetWindowFocusCallback(window, window_focus_callback);
   glfwMakeContextCurrent(window);


   glewExperimental=true; // Needed in core profile
   if (glewInit() != GLEW_OK) {
       std::cout << "System::init() , Failed to initiate glew" << std::endl;
       //TRACE("Failed to initiate glew");
   }


   for (int i=1; i< argc; i++) {
     if (!strcmp(argv[i],"-h"))
     {
         printHelp(argc,argv);
         exit(1);
     }

     if (!strcmp(argv[i],"-s"))
     {
        shader_base=argv[i+1];
     }

     if (!strcmp(argv[i],"-t"))
     {
         texture1=TextureFromFile(argv[i+1],".",false);
     }



   }

   sprintf(vertex_shader,"%s.vert",shader_base);
   sprintf(fragment_shader,"%s.frag",shader_base);
   sprintf(geometry_shader,"%s.geom",shader_base);


   // Set the required callback functions
   glfwSetKeyCallback(window, key_callback);
   glfwSetCursorPosCallback(window, mouse_callback);
   glfwSetScrollCallback(window, scroll_callback);


   // Define the viewport dimensions
   glViewport(0, 0, screenWidth, screenHeigth);

   // Setup some OpenGL options
   glEnable(GL_DEPTH_TEST);

   // Setup and compile our shaders
   Shader shader(vertex_shader, fragment_shader,geometry_shader);
   //Shader shader("shader.vert", "shader.frag");


   Mesh *mesh=NULL;


   //strcpy(out_filename,filename);

   char *pExt = strrchr(argv[argc-1], '.');


   if (pExt != NULL)
   {
      if (strcmp(pExt,".mdl")==0)
      {
          loadSimple(argv[argc-1]);
      }
      else
      {
          mesh = new Mesh(argv[argc-1]);
      }

   }
   else
   {
              printf("Please specify model to load \n");
              exit(1);

   }



   if (mesh)
   {
       for(int j=0;j<mesh->textures_loaded.size();j++)
       {
           printf("Loaded texture type %s from %s id %d\n",mesh->textures_loaded[j].type.c_str(),mesh->textures_loaded[j].path.data,mesh->textures_loaded[j].id);
           texture1=mesh->textures_loaded[j].id;
       }
   }




   //setupTestData();


   glColor3f(0.8f, 0.1f, 0.1f);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

   shader.Use();
   // This binds the attrib opengl 2.1 stuff
   glBindAttribLocation (shader.Program, 0, "position");
   glBindAttribLocation (shader.Program, 1, "normal");
   glBindAttribLocation (shader.Program, 2, "vtex");


while (!glfwWindowShouldClose(window)) {

    // Set frame time
      GLfloat currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      // Clear the colorbuffer
      glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


      // Bind Textures using texture units, Mesh might have loaded a new texture
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture1);
      glUniform1i(glGetUniformLocation(shader.Program, "tex"), 0);

      shader.Use();



      // Check and call events
      glfwPollEvents();
      Do_Movement();


      // Create transformations
      glm::mat4 view;
      glm::mat4 projection;
      //view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
      //projection = glm::perspective(45.0f, (GLfloat)screenWidth / (GLfloat)screenHeigth, 0.1f, 100.0f);
      // Get their uniform location
      projection = glm::perspective(camera.Zoom, (float)screenWidth/(float)screenHeigth, 0.1f, 100.0f);
      view = camera.GetViewMatrix();

      GLint modelLoc = glGetUniformLocation(shader.Program, "model");
      GLint viewLoc = glGetUniformLocation(shader.Program, "view");
      GLint projLoc = glGetUniformLocation(shader.Program, "projection");
      GLint mvpLoc = glGetUniformLocation(shader.Program, "mvp");



      glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
      // Note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
      glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

     // Transformation matrices
     //glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth/(float)screenHeigth, 0.1f, 100.0f);
     //glm::mat4 view = camera.GetViewMatrix();
     //glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
     //glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));


      glm::mat4 model;
      model = glm::translate(model, center);

      if (rotating)
      {
          angleX+=0.02;
          angleY+=0.04;
      }

      model = glm::rotate(model, angleX, glm::vec3(1.0f, 0.0f, 0.1f));
      model = glm::rotate(model, angleY, glm::vec3(0.0f, 1.0f, 0.1f));

      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

      glm::mat4 mvp=projection * view * model;

      glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));



      if (mdl_index_count>0)
      {
          // Draw mdl :-P
          //glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D, texture1);
          //glUniform1i(glGetUniformLocation(shader.Program, "tex"), 0);


          glBindVertexArray(VAO);
          glDrawElements(GL_TRIANGLES, mdl_index_count, GL_UNSIGNED_SHORT, 0);
      }

      if (mesh) mesh->render();
      glfwSwapBuffers(window);
}
delete mesh;

return 0;
}

#pragma region "User input"

// Moves/alters the camera positions based on user input
void Do_Movement()
{
    // Camera controls
    if(keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;


    rotating=false;
    angleX+=yoffset/45.0f;
    angleY+=xoffset/45.0f;

    //camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
    //printf("scroll\n");
}

#pragma endregion



