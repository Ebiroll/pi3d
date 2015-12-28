
#include "Mesh.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include "SOIL.h"
#include "pkg.h"


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
#define GLSL(src) "#version 120\n" #src

const char* vs = GLSL(
            attribute vec3 position;
            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;
            void main()
            {
              gl_Position =  projection * view  * vec4(position.x,position.y,position.z,1.0);
            });


const char* fs =
        "uniform vec3      iResolution;\n"           // viewport resolution (in pixels)
        "uniform float     time;\n"                  // shader playback time (in seconds)

        "void main()\n"
        "{\n"
          "vec3 c;\n"
          "float l,z=time;\n"
          "for(int i=0;i<3;i++)\n"
          "{\n"
                "vec2 uv,p=gl_FragCoord.xy/iResolution.xy;\n"
                "uv=p;\n"
                "p-=.5;\n"
                "p.x*=iResolution.x/iResolution.y;\n"
                "z+=.07;\n"
                "l=length(p);\n"
                "uv+=p/l*(sin(z)+1.)*abs(sin(l*9.-z*2.));\n"
                "c[i]=.01/length(abs(mod(uv,1.)-.5));\n"
          "}\n"
          "gl_FragColor=vec4(c/l,time);\n"
          "//gl_FragColor=vec4(1.0,0.0,1.0,1.0);\n"
        "}\n";



glm::vec3 center(0.0f, 0.0f, 0.0f);


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 20.0f));

bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
int screenWidth=1280;
int screenHeigth=1024;

// Backup texture
GLuint texture1=-1;
GLuint VBO, VAO;
int mdl_index_count=-1;


GLfloat angleX=0.0;
GLfloat angleY=0.0;

bool rotating=true;



float vertices[] = {
     -10.0f,  -10.0f, 0.0f , // Vertex 0 (X, Y, Z)
     10.0f, -10.0f, 0.0f , // Vertex 1 (X, Y, Z)
     10.0f, 10.0f, 0.0f,   // Vertex 2 (X, Y ,Z)
     -10.0f,  10.0f, 0.0f , // Vertex 3 (X, Y, Z)
     -10.0f, -10.0f, 0.0f   // Vertex 4 (X, Y ,Z)

};

GLushort  indexes[] = {0,1,2,2,3,0};


void printHelp(int argc, char *argv[]) {

    printf("Usage, %s [-t texture] [-s shader_base] *\n",argv[0]);

    printf("     -t   Texture1 to use \n");
    printf("     -s   shaderfile will load and compile shaderfile.vert & shaderfile.frag\n");
    printf("     -h   This helptext\n");

}


//void loadSimple(char *filename,Camera &camera);


int main(int argc, char* argv[])
{


  GLFWwindow* window;
  char *shader_base="shader";
  char vertex_shader[512];
  char fragment_shader[512];
  char geometry_shader[512];


  if (argc<2)
  {
      //printHelp(argc,argv);
  }

  int ret=glfwInit();
   if (!ret)
   {
      std::cerr << "Init failed"  << ret << std::endl;
   }

   //glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); // We want OpenGL 2.1 ??
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
   //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
   //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // If we don't want the old OpenGL
   //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

   //#define FULL 1

#ifdef FULL
    GLFWmonitor* monitor=glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    window = glfwCreateWindow(mode->width, mode->height, "assviewer 0.1", monitor, NULL);
#else
   window = glfwCreateWindow(screenWidth, screenHeigth, "Viewer", NULL, NULL);
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

   //shader_base="../shader/creation";

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

   glEnable(GL_TEXTURE_2D);

   // Setup and compile our shaders
   Shader shader(vertex_shader, fragment_shader,geometry_shader);
   //Shader shader("shader.vert", "shader.frag");

   Mesh *mesh=NULL;

   // Use square flat area as mesh

   glGenVertexArrays(1, &VAO);
   glBindVertexArray(VAO);

   GLuint vb;
   glGenBuffers(1, &vb);
   glBindBuffer(GL_ARRAY_BUFFER, vb);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

   GLuint ib;
   glGenBuffers(1, &ib);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), &indexes[0], GL_STATIC_DRAW);
   mdl_index_count=6;


   glEnableVertexAttribArray(0);
   //glEnableVertexAttribArray(1);
   //glEnableVertexAttribArray(2);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
   //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char*)0 + 12);
   //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char*)0 + 24);

   glBindVertexArray(0);



   Camera ca(glm::vec3(0.0f,0.0f,10.0f));
   camera=ca;


   //strcpy(out_filename,filename);

   char *pExt = strrchr(argv[argc-1], '.');

#if 0
   if (pExt != NULL)
   {
      if (strcmp(pExt,".mdl")==0)
      {
          loadSimple(argv[argc-1],camera);
      }
      else
      {
          mesh = new Mesh(argv[argc-1]);
      }
   }
   else
   {
          //printf("Please specify model to load \n");
          //exit(1);

   }
#endif


   //if (mesh)
   //{
   //   for(int j=0;j<mesh->textures_loaded.size();j++)
   //    {
   //        printf("Loaded texture type %s from %s id %d\n",mesh->textures_loaded[j].type.c_str(),mesh->textures_loaded[j].path.data,mesh->textures_loaded[j].id);
   //        texture1=mesh->textures_loaded[j].id;
   //    }
   //}



   //setupTestData();


   glColor3f(0.8f, 0.1f, 0.1f);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

   shader.Use();
   // This binds the attrib opengl 2.1 stuff
   glBindAttribLocation (shader.Program, 0, "position");
   //glBindAttribLocation (shader.Program, 1, "normal");
   //glBindAttribLocation (shader.Program, 2, "vtex");


   shader.linkProg();


   glm::vec3 resolution((float)screenWidth,(float)screenHeigth,0.0f);


   /////////////////////////// Cloud shader
   //uniform vec3 fogColor;
   //uniform float fogNear;
   //uniform float fogFar;
   //uniform float opacity;

   glm::vec3  fogColor(1.0f,0.0f,0.5f);
   float fogNear=0.1f;
   float fogFar=0.8f;
   float opacity=0.9f;

   GLint fogColorLoc = glGetUniformLocation(shader.Program, "fogColor");
   glUniform3f(fogColorLoc,fogColor.x,fogColor.y,fogColor.z);

   GLint fogNearLoc = glGetUniformLocation(shader.Program, "fogNear");
   glUniform1f(fogNearLoc,fogNear);

   GLint fogFarLoc = glGetUniformLocation(shader.Program, "fogFar");
   glUniform1f(fogFarLoc,fogFar);

   GLint opacityLoc = glGetUniformLocation(shader.Program, "opacity");
   glUniform1f(opacityLoc,opacity);
   ////////////////////////////////////////////////////


while (!glfwWindowShouldClose(window)) {

    // Set frame time
      GLfloat currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      fogFar=fogFar+0.1f;
      glUniform1f(fogFarLoc,fogFar);

      fogColor.g=fogColor.g+0.01;

      // Clear the colorbuffer
      glClearColor(0.35f, 0.35f, 0.35f, 1.0f);
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
      projection = glm::perspective(camera.Zoom, (float)screenWidth/(float)screenHeigth, 0.4f, 50.0f);
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


      GLint resLoc = glGetUniformLocation(shader.Program, "iResolution");
      glUniform3f(resLoc,resolution.x,resolution.y,resolution.z);


      GLint timeLoc = glGetUniformLocation(shader.Program, "time");
      glUniform1f(timeLoc,currentFrame);

      if (mdl_index_count>0)
      {
          // Draw mdl :-P
          if(texture1>0) {
              glActiveTexture(GL_TEXTURE0);
              glBindTexture(GL_TEXTURE0, texture1);
              glUniform1i(glGetUniformLocation(shader.Program, "tex"), 0);
          }

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



