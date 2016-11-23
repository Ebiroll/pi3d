
#include "Mesh.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
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


#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>

// GL includes
#include "shader.h"
#include "camera.h"
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
#define GLSL(src) "#version 120\n" #src


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

const char* fs = GLSL(
            varying vec2 texcoord;
            uniform sampler2D tex;

            void main()
            {
                gl_FragColor = texture2D(tex, texcoord);
                //gl_FragColor = vec4(0.0,1.0,0.0,1.0);
                //gl_FragColor = vec4(texcoord.x,texcoord.y,0.0,1.0);
            });

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
GLfloat angleZ=0.0;

bool rotating=true;

// This connects to a ESP8266 with the sketch loaded in the sketch direcory
// https://learn.adafruit.com/bno055-absolute-orientation-sensor-with-raspberry-pi-and-beaglebone-black/hardware?view=all
void *connection_handler(void *dummy);


char *sensor_adress;

char buffer[1024];

// This connects to the 
void *connection_handler(void *dummy)
{
    int socket_desc  , c , *new_sock;
    struct sockaddr_in server , client;
    int ret;
    int n;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    } else {
        int enable = 1;
        if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
            printf("setsockopt(SO_REUSEADDR) failed");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("192.168.1.118");
    server.sin_port = htons( 10023 );

    ret=-1;
    while (ret<0)
    {
        ret = connect(socket_desc, (struct sockaddr*)&server, sizeof(struct sockaddr));

        if(ret < 0){
            //printf("connectnr %d: \n", ret);
            fprintf(stderr, "Error: Can't connect to the server.\n");
            //return 1;
        }
    }

    rotating=false;
    float x;
    float y;
    float z;




   bzero(buffer,256);
   n = read(socket_desc, buffer, 255);
   while (n>=0) {
       //printf("%s",buffer);
       sscanf(buffer,"%f,%f,%f",&x,&y,&z);
       angleX=M_PI*(x+90.0f)/180.0f;
       angleY=M_PI*y/180.0f;
       angleZ=M_PI*z/180.0f;
 
       bzero(buffer,256);
       n = read(socket_desc, buffer, 255);
   }


}


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

  // Start connection thread
  pthread_t pconnection_thread;
    
  printf("creating socket thread\n");
  if( pthread_create( &pconnection_thread , NULL ,  connection_handler , (void*) NULL) < 0)
  {
    printf("Failed to create connection thread\n");
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


   //glewExperimental=true; // Needed in core profile
   if (glewInit() != GLEW_OK) {
       std::cout << "System::init() , Failed to initiate glew, check GLFW_CONTEXT_VERSION_MAJOR & MINOR in main.cpp" << std::endl;
       exit(0);
       //TRACE("Failed to initiate glew");
   }


   // Set the required callback functions
   glfwSetKeyCallback(window, key_callback);
   glfwSetCursorPosCallback(window, mouse_callback);
   glfwSetScrollCallback(window, scroll_callback);


   // Define the viewport dimensions
   glViewport(0, 0, screenWidth, screenHeigth);

   // Setup some OpenGL options
   glEnable(GL_DEPTH_TEST);

   glEnable(GL_TEXTURE_2D);


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

     if (!strcmp(argv[i],"-t"))
     {
         texture1=TextureFromFile(argv[i+1],".",false);
         staticData[0].textureIx=texture1;
     }

   }

   sprintf(vertex_shader,"%s.vert",shader_base);
   sprintf(fragment_shader,"%s.frag",shader_base);
   sprintf(geometry_shader,"%s.geom",shader_base);

   // Setup and compile our shaders
   Shader shader(vertex_shader, fragment_shader,geometry_shader);
   //Shader shader("shader.vert", "shader.frag");

   // This binds the attrib opengl 2.1 stuff
   glBindAttribLocation (shader.Program, 0, "position");
   glBindAttribLocation (shader.Program, 1, "normal");
   glBindAttribLocation (shader.Program, 2, "vtex");
   glBindAttribLocation (shader.Program, 3, "index");


   Mesh *mesh=NULL;

   //strcpy(out_filename,filename);

   char *pExt = strrchr(argv[argc-1], '.');


   if (pExt != NULL)
   {
      if (strcmp(pExt,".mdl")==0)
      {
          loadSimple(argv[argc-1],camera,state,&staticData[0]);
      } else if (strcmp(pExt,".pkg")==0) {
          gMaxMdl=loadPkg(argv[argc-1],camera,&staticData[0],MAX_MDLS);
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

   shader.linkProg();
   shader.Use();


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
          //angleX+=0.02;
          //angleY+=0.04;
      }


      model = glm::rotate(model, angleZ, glm::vec3(1.0f, 0.0f, 0.0f));
      model = glm::rotate(model, -angleX, glm::vec3(0.0f, 1.0f, 0.0f));
      model = glm::rotate(model, -angleY, glm::vec3(0.0f, 0.0f, 1.0f));


/*
      model = glm::rotate(model, angleX, glm::vec3(1.0f, 0.0f, 0.0f));
      model = glm::rotate(model, angleY, glm::vec3(0.0f, 1.0f, 0.0f));
      model = glm::rotate(model, angleZ, glm::vec3(0.0f, 0.0f, 1.0f));
*/
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

      glm::mat4 mvp=projection * view * model;

      glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

      if (gMaxMdl>0)
      {

          for (int q=0;q<gMaxMdl;q++)
          {
              //printf("%d\n",q);
              glActiveTexture(GL_TEXTURE0);
              glBindTexture(GL_TEXTURE_2D, staticData[q].textureIx);
              //glUniform1i(glGetUniformLocation(shader.Program, "tex"), 0);  // Texture unit 0 is for base images.

              glBindVertexArray(staticData[q].indexVAO);
              glDrawElements(GL_TRIANGLES, staticData[q].numIndexes, GL_UNSIGNED_SHORT, 0);

          }
      }
      if (mdl_index_count>0)
      {
          // Draw mdl :-P
          //glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D, texture1);
          //glUniform1i(glGetUniformLocation(shader.Program, "tex"), 0);
          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, texture1);
          glUniform1i(glGetUniformLocation(shader.Program, "tex"), 0);  // Texture unit 0 is for base images.

          glBindVertexArray(VAO);
          glDrawElements(GL_TRIANGLES, mdl_index_count, GL_UNSIGNED_SHORT, 0);
      }
      if (mesh) mesh->render(shader.Program);
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



