
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include "SOIL.h"
#include "pkg.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <glm/gtx/euler_angles.hpp>
#include <unistd.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>

#include <emscripten.h>

// GL includes
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "shader.h"
#include "camera.h"
#include <emscripten/html5.h>

// Nice tutorials
// http://duriansoftware.com/joe/An-intro-to-modern-OpenGL.-Chapter-2.2:-Shaders.html
//
// http://blog.db-in.com/all-about-opengl-es-2-x-part-1/
//
// Builtin GL ES functions
// http://www.shaderific.com/glsl-functions/
//

////////////////// Default shaders ///////////
#if 0

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
#endif
#define GLSL(src) "#version 100\n" #src
//#define GLSL(src)  #src

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
            //texcoord = vec2(vtex.x , vtex.y)  + vec2(0.5);
            //texcoord  =  vtex;
            texcoord = vec2(vtex.x, 1.0 - vtex.y);
        });


const char* fs = R"(
precision mediump float;
varying vec2 texcoord;
uniform sampler2D tex;
uniform vec4 colour;
void main() {
  vec2 coord = clamp(texcoord, 0.0, 1.0);
  //gl_FragColor = texture2D(tex, coord) * colour;
  gl_FragColor = vec4(0.0,1.0,0.0,1.0);
}
)";

        #if 0
const char* fs = GLSL(
            precision highp float;
            varying vec2 texcoord;
            uniform sampler2D tex;
            uniform vec4 colour;

            void main()
            {

             vec2 coord;\n
             coord.x=clamp(texcoord.x,0.0,1.0);
             coord.y=clamp(texcoord.y,0.0,1.0);
             //gl_FragColor = texture2D(tex, coord);
                // TODO! we want colour 
                //gl_FragColor = texture2D(tex, texcoord);
                //gl_FragColor = vec4(0.0,1.0,0.0,1.0);
                gl_FragColor = vec4(0.0,0.0,coord.y,1.0) ;
                //gl_FragColor = texture2D(tex, coord) ;
            });
            #endif

            /*
            const char* fs = GLSL(
            varying vec2 texcoord;
            uniform sampler2D tex;

            void main()
            {
                gl_FragColor = texture2D(tex, texcoord);
                //gl_FragColor = vec4(0.0,1.0,0.0,1.0);
                //gl_FragColor = vec4(texcoord.x,texcoord.y,0.0,1.0);
            });
            */
#if 0
const char* vs = GLSL(
    attribute vec4 position;                     
     void main()                                  
     {                                            
       gl_Position = vec4(position.xyz, 1.0);    
     });

const char* fs = GLSL(
    precision mediump float;
    void main()                            
    {                                         
      gl_FragColor[0] = gl_FragCoord.x/640.0; 
      gl_FragColor[1] = gl_FragCoord.y/480.0; 
      gl_FragColor[2] = 0.5;                  
    });  
#endif
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
int mdl_index_count=0;


GLfloat offsetAngleX=0.0;
GLfloat offsetAngleY=0.0;


GLfloat angleX=0.0;
GLfloat angleY=0.0;
GLfloat angleZ=0.0;

bool rotating=true;

// This connects to a ESP8266 with the sketch loaded in the sketch direcory
// https://learn.adafruit.com/bno055-absolute-orientation-sensor-with-raspberry-pi-and-beaglebone-black/hardware?view=all
void *connection_handler(void *dummy);


char *sensor_adress;

char buffer[1024];


static void enable_webgl_extensions() {
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_get_current_context();
  emscripten_webgl_enable_extension(ctx, "WEBGL_compressed_texture_s3tc");
  emscripten_webgl_enable_extension(ctx, "WEBKIT_WEBGL_compressed_texture_s3tc");
  emscripten_webgl_enable_extension(ctx, "MOZ_WEBGL_compressed_texture_s3tc");
  emscripten_webgl_enable_extension(ctx, "WEBGL_compressed_texture_s3tc_srgb");
}
#if 0
GLint TextureFromFile(const char* path, std::string directory, bool gamma)
{
     //Generate texture ID and load texture data
    std::string filename = std::string(path);
    filename = directory + '/' + filename;
    GLuint textureID;
    glGenTextures(1, &textureID);
    int width,height;
    int numChannels=0;

    unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, &numChannels, SOIL_LOAD_RGB);
    if (image==NULL)
    {
        printf("Failed to load texture %s",SOIL_last_result());
    }
    // Assign texture to ID
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, gamma ? GL_SRGB : GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
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
#endif

void printHelp(int argc, char *argv[]) {

    printf("Usage, %s [-t texture] [-s shader_base] modelname.*\n",argv[0]);

    printf("     -t   Texture to force load when loading a mdl file \n");
    printf("     -s   shaderfile will load and compile shaderfile.vert & shaderfile.frag\n");
    printf("     -h   This helptext\n");

}


// Only used by pi3d.cpp
static STATE_T _state, *state = &_state;	// global graphics state


#define MAX_MDLS 100
mdlGLData staticData[MAX_MDLS];
int gMaxMdl=0;

GLFWwindow* window;
Shader *shader;


void do_frame(){
    // Set frame time
      GLfloat currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;



      // Bind Textures using texture units, Mesh might have loaded a new texture
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture1);
      glUniform1i(glGetUniformLocation(shader->Program, "tex"), 0);

      shader->Use();



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

      GLint modelLoc = glGetUniformLocation(shader->Program, "model");
      GLint viewLoc = glGetUniformLocation(shader->Program, "view");
      GLint projLoc = glGetUniformLocation(shader->Program, "projection");
      GLint mvpLoc = glGetUniformLocation(shader->Program, "mvp");

      GLint maxAttr = 0; glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttr);
        auto enable_attr = [&](GLint loc, GLint size, GLenum type, GLsizei stride, const void* off, GLuint vbo){
            if (loc < 0 || loc >= maxAttr) return;           // <-- critical guard
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glEnableVertexAttribArray((GLuint)loc);
            glVertexAttribPointer((GLuint)loc, size, type, GL_FALSE, stride, off);
        };


      glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
      // Note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
      glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

     // Transformation matrices
     //glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth/(float)screenHeigth, 0.1f, 100.0f);
     //glm::mat4 view = camera.GetViewMatrix();
     //glUniformMatrix4fv(glGetUniformLocation(shader->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
     //glUniformMatrix4fv(glGetUniformLocation(shader->Program, "view"), 1, GL_FALSE, glm::value_ptr(view));


      glm::mat4 model;
      model = glm::translate(model, center);

      if (rotating)
      {
          angleX+=0.02;
          angleY+=0.04;
      }

      model=model*glm::eulerAngleYXZ(1*angleX+offsetAngleX,angleZ,-1*(angleY+offsetAngleY));


      //model = glm::rotate(model, angleZ, glm::vec3(1.0f, 0.0f, 0.0f));
      //model = glm::rotate(model, -1*angleX+offsetAngleX, glm::vec3(0.0f, 1.0f, 0.0f));
      //model = glm::rotate(model, -angleY+offsetAngleY, glm::vec3(0.0f, 0.0f, 1.0f));


/*
      model = glm::rotate(model, angleX, glm::vec3(1.0f, 0.0f, 0.0f));
      model = glm::rotate(model, angleY, glm::vec3(0.0f, 1.0f, 0.0f));
      model = glm::rotate(model, angleZ, glm::vec3(0.0f, 0.0f, 1.0f));
*/

      GLuint attr_position = glGetAttribLocation(shader->Program, "position");
      GLuint attr_vtex = glGetAttribLocation(shader->Program, "vtex");
      GLint my_maxAttr; glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &my_maxAttr);
     printf("attr_position=%d attr_vtex=%d max=%d\n", attr_position, attr_vtex, my_maxAttr);


      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

      glm::mat4 mvp=projection * view * model;

      glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

      //printf("max mdl %d\n",gMaxMdl);
      if (gMaxMdl>0)
      {
        //printf("-");

          for (int q=0;q<gMaxMdl;q++)
          {
              //printf("%d\n",q);
              enable_attr(attr_position, 3, GL_FLOAT, 0, (void*)0, staticData[q].dataVBO);
              enable_attr(attr_vtex,  2, GL_FLOAT, 0, (void*)0, staticData[q].uvVBO);

                // bind & set only if valid
                if (attr_position >= 0) {
                glBindBuffer(GL_ARRAY_BUFFER, staticData[q].dataVBO);
                glEnableVertexAttribArray(attr_position);
                glVertexAttribPointer(attr_position, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                }
                if (attr_vtex >= 0) {
                glBindBuffer(GL_ARRAY_BUFFER, staticData[q].uvVBO);
                glEnableVertexAttribArray(attr_vtex);
                glVertexAttribPointer(attr_vtex, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
                }


#if 1
            //glEnableVertexAttribArray(attr_position);
            //printf("%d\n",staticData[q].dataVBO);
            glBindBuffer(GL_ARRAY_BUFFER, staticData[q].dataVBO);
            
            glVertexAttribPointer(
            attr_position, // The attribute we want to configure
            3, // size : X+Y+Z => 3
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            0, // stride
            (void*)0 // array buffer offset
            );
#endif

              //glUniform1i(glGetUniformLocation(shader->Program, "tex"), 0);  // Texture unit 0 is for base images.
            //glTexImage2D();

#if 1

    //glVertexAttribPointer(Effect::TEXCOORD0_ATTR, 2, GL_FLOAT, GL_FALSE, 0, ms_texCoords);


          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, staticData[q].textureIx);


          glUniform1i(glGetUniformLocation(shader->Program, "vtex"), 0);

          //glEnableVertexAttribArray(attr_vtex);
          //glBindBuffer(GL_ARRAY_BUFFER, staticData[q].uvVBO);
          //glVertexAttribPointer(
          //attr_vtex, // The attribute we want to configure
          //2, // size : U+V => 2
          //GL_FLOAT, // type
          //GL_FALSE, // normalized?
          //0, // stride
          //(void*)0 // array buffer offset
          //);

#endif

              //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,staticData[q].indexVAO);
              //glBindVertexArray(staticData[q].indexVAO);

              glBindBuffer(GL_ARRAY_BUFFER, staticData[q].dataVBO);
              glEnableVertexAttribArray(attr_position);
              glVertexAttribPointer(attr_position, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

              //glActiveTexture(GL_TEXTURE0);
              //glBindTexture(GL_TEXTURE_2D, staticData[q].textureIx);   
              //glUniform1i(glGetUniformLocation(shader->Program, "vtex"), 0);
              glBindBuffer(GL_ARRAY_BUFFER, staticData[q].uvVBO);
              if (attr_vtex >= 0) {
                glEnableVertexAttribArray((GLuint)attr_vtex);
                glVertexAttribPointer((GLuint)attr_vtex, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
              }

              glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, staticData[q].indexVAO);
              glDrawElements(GL_TRIANGLES, staticData[q].numIndexes, GL_UNSIGNED_SHORT, 0);

              
          }
      }
      
      //printf("mdl_index_count %d",mdl_index_count);
      if (mdl_index_count>0)      
      {
          printf(".");
          // Draw mdl :-P
          //glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D, texture1);
          //glUniform1i(glGetUniformLocation(shader->Program, "tex"), 0);
          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, texture1);
          glUniform1i(glGetUniformLocation(shader->Program, "tex"), 0);  // Texture unit 0 is for base images.

          //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,staticData[0].indexVAO);
          //glBindVertexArray(VAO);
          //glDrawElements(GL_TRIANGLES, mdl_index_count, GL_UNSIGNED_SHORT, 0);
      }
      
      //if (mesh) mesh->render(shader->Program);
      glfwSwapBuffers(window);
}


int main(int argc, char* argv[])
{


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

   //glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
   //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); // We want OpenGL 2.1 ??
   //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
   //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
   //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // If we don't want the old OpenGL
   //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // OpenGL ES 2.0
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


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
          printf("No window\n");
         glfwTerminate();
     }


   //glfwSetWindowFocusCallback(window, window_focus_callback);
   glfwMakeContextCurrent(window);


  
#if 0
   printf("GLEW init %d,%d\n",screenWidth, screenHeigth);


   //glewExperimental=true; // Needed in core profile
   if (glewInit() != GLEW_OK) {
       std::cout << "System::init() , Failed to initiate glew, check GLFW_CONTEXT_VERSION_MAJOR & MINOR in main.cpp" << std::endl;
       exit(0);
       //TRACE("Failed to initiate glew");
   }
#endif

   // Set the required callback functions
   glfwSetKeyCallback(window, key_callback);
   glfwSetCursorPosCallback(window, mouse_callback);
   glfwSetScrollCallback(window, scroll_callback);


   printf("Viewport %d,%d\n",screenWidth, screenHeigth);

   // Define the viewport dimensions
   glViewport(0, 0, screenWidth, screenHeigth);

   // Setup some OpenGL options
   //glEnable(GL_DEPTH_TEST);

    // Not 
   //glEnable(GL_TEXTURE_2D);
    enable_webgl_extensions();


   for (int i=1; i< argc; i++) {
     if (!strcmp(argv[i],"-h"))
     {
         printHelp(argc,argv);
         exit(1);
     }

     if (!strcmp(argv[i],"-s"))
     {
        printf("SHAD %s\n",argv[i+1]);
        shader_base=argv[i+1];
     }

/*
     if (!strcmp(argv[i],"-t"))
     {
         texture1=TextureFromFile(argv[i+1],".",false);
         staticData[0].textureIx=texture1;
     }
*/
   }

   sprintf(vertex_shader,"%s.vert",shader_base);
   sprintf(fragment_shader,"%s.frag",shader_base);
   sprintf(geometry_shader,"%s.geom",shader_base);



   printf("Init shaders\n");

   // Setup and compile our shaders
    shader=new Shader(vertex_shader, fragment_shader,NULL);
   //Shader shader("shader->vert", "shader->frag");

   printf("Shaders created\n");



   // This binds the attrib opengl 2.1 stuff
   
   glBindAttribLocation (shader->Program, 0, "position");
   glBindAttribLocation (shader->Program, 1, "normal");
   glBindAttribLocation (shader->Program, 2, "vtex");
   glBindAttribLocation (shader->Program, 3, "index");
   glEnableVertexAttribArray(0);


   //strcpy(out_filename,filename);


   printf("Loading ----------  pkg\n");

   gMaxMdl=loadPkg("data/a320_ein.pkg",camera,&staticData[0],MAX_MDLS);

   printf("Loaded ---------- %d pkg\n",gMaxMdl);



   char *pExt = strrchr(argv[argc-1], '.');


   if (pExt != NULL)
   {
      if (strcmp(pExt,".mdl")==0)
      {
          loadSimple(argv[argc-1],camera,state,&staticData[0]);
      } else if (strcmp(pExt,".pkg")==0) {
          gMaxMdl=loadPkg(argv[argc-1],camera,&staticData[0],MAX_MDLS);
      }
      /*
      else
      {
          mesh = new Mesh(argv[argc-1]);
      }
     */
   }
   else
   {
              printf("Please specify model to load \n");
              //exit(1);

   }

/*
   if (mesh)
   {
       for(int j=0;j<mesh->textures_loaded.size();j++)
       {
           printf("Loaded texture type %s from %s id %d\n",mesh->textures_loaded[j].type.c_str(),mesh->textures_loaded[j].path.data,mesh->textures_loaded[j].id);
           texture1=mesh->textures_loaded[j].id;
       }
   }
*/

   //setupTestData();
   //glColor3f(0.8f, 0.1f, 0.1f);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

   shader->linkProg();
   std::cout << "Use Shader" << std::endl; 

   shader->Use();


   std::cout << "Main loop" << std::endl; 

    // Clear the colorbuffer
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    emscripten_set_main_loop(do_frame, 20, 0);

//while (!glfwWindowShouldClose(window)) {
//}
//delete mesh;

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

    if(action == GLFW_PRESS) {
        keys[key] = true;
        if (key==GLFW_KEY_A) {
            offsetAngleX+=5;
        }
        if (key==GLFW_KEY_D) {
            offsetAngleX-=5;
        }
    }
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

    offsetAngleX+=xoffset*M_PI/180.f;
    offsetAngleY+=yoffset*M_PI/180.f;

    lastX = xpos;
    lastY = ypos;


    rotating=false;
    //angleX+=yoffset/45.0f;
    //angleY+=xoffset/45.0f;

    //camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
    //printf("scroll\n");
}

#pragma endregion



