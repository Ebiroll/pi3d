#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include "SOIL.h"
#include "pipkg.h"
#include <string>
#include <vector>
#include "SOIL.h"

// GL includes
#include "shader.h"
//#include "camera.h"
//#include "model.h"

using std::vector;
using std::string;


// GL includes
#include "shader.h"
#include "camera.h"
#include "EGL/egl.h"
#include "GLES/glplatform.h"
#include "GLES/gl.h"
#include "GLES2/gl2.h"

// Should be defined somewhere
#ifndef GLAPIENTRY
#define GLAPIENTRY __stdcall
#endif

#include "GLES2/gl2ext.h"
#ifdef BCMHOST
#include "bcm_host.h"
#endif
extern "C" {
#include "eglstate.h"	
}
#ifdef _X11_XLIB_H_

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#endif


static STATE_T _state, *state = &_state;	// global graphics state

// Nice tutorials
// http://duriansoftware.com/joe/An-intro-to-modern-OpenGL.-Chapter-2.2:-Shaders.html
//
// http://blog.db-in.com/all-about-opengl-es-2-x-part-1/
//
// Builtin GL ES functions
// http://www.shaderific.com/glsl-functions/
//

#ifndef M_PI
   #define M_PI 3.141592654
#endif

/***********************************************************
 * Name: reset_model
 *
 * Arguments:
 *       CUBE_STATE_T *state - holds OGLES model info
 *
 * Description: Resets the Model projection and rotation direction
 *
 * Returns: void
 *
 ***********************************************************/
static void reset_model(STATE_T *state)
{
   // reset model position
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0.f, 0.f, -50.f);

   // reset model rotation
   //state->rot_angle_x = 45.f; state->rot_angle_y = 30.f; state->rot_angle_z = 0.f;
   //state->rot_angle_x_inc = 0.5f; state->rot_angle_y_inc = 0.5f; state->rot_angle_z_inc = 0.f;
   //state->distance = 40.f;
}



/***********************************************************
 * Name: init_model_proj
 *
 * Arguments:
 *       CUBE_STATE_T *state - holds OGLES model info
 *
 * Description: Sets the OpenGL|ES model to default values
 *
 * Returns: void
 *
 ***********************************************************/
static void init_model_proj(STATE_T *state)
{
   float nearp = 1.0f;
   float farp = 500.0f;
   float hht;
   float hwd;

   glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

   glViewport(0, 0, (GLsizei)state->screen_width, (GLsizei)state->screen_height);
      
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   hht = nearp * (float)tan(45.0 / 2.0 / 180.0 * M_PI);
   hwd = hht * (float)state->screen_width / (float)state->screen_height;

   //glFrustumf(-hwd, hwd, -hht, hht, nearp, farp);
   
   glEnableClientState( GL_VERTEX_ARRAY );
   //glVertexPointer( 3, GL_BYTE, 0, quadx );

   //reset_model(state);
}



float vertices[] = {
     -10.0f,  -10.0f, 0.0f , // Vertex 0 (X, Y, Z)
     10.0f, -10.0f, 0.0f , // Vertex 1 (X, Y, Z)
     10.0f, 10.0f, 0.0f,   // Vertex 2 (X, Y ,Z)
     -10.0f,  10.0f, 0.0f , // Vertex 3 (X, Y, Z)
     -10.0f, -10.0f, 0.0f   // Vertex 4 (X, Y ,Z)

};

GLushort  indexes[] = {0,1,2,2,3,0};

#define check() assert(glGetError() == 0)

////////////////// Default shaders ///////////

#define GLSL(src) "#version 100\n" #src
//#define GLSL(src) #src


const char* vs = GLSL(
        attribute highp vec3 position;
        attribute vec3 normal;
        attribute vec2 vtex;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        varying vec2 texcoord;

        void main()
        {
	     //gl_Position = projection * view * model * vec4(position, 1.0);
	    gl_Position = vec4(position, 1.0);
            //texcoord = vec2(vtex.x , vtex.y)  + vec2(0.5);
            //texcoord  =  vtex;
            texcoord = vec2(vtex.x, 1.0 - vtex.y);
        });

const char* fs = GLSL(
//            precision highp vec2;
//            precision mediump float;
//            precision mediump sampler2D;

//           varying highp vec2 texcoord;
//           uniform sampler2D tex;

            void main()
            {
	        //gl_FragColor = texture2D(tex, texcoord);
                gl_FragColor = vec4(0.0,1.0,0.0,1.0);
                //gl_FragColor = vec4(texcoord.x,texcoord.y,0.0,1.0);
            });



glm::vec3 center(0.0f, 0.0f, 0.0f);


// Function prototypes

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


void createSurface()
{
    //GLuint vb;
   glGenBuffers(1, &state->buf);
   glBindBuffer(GL_ARRAY_BUFFER, state->buf);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

   GLuint ib;
   glGenBuffers(1, &ib);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), &indexes[0], GL_STATIC_DRAW);
   mdl_index_count=6;


   glEnableVertexAttribArray(0);

}




void printHelp(int argc, char *argv[]) {

    printf("Usage, %s [-t texture] [-s shader_base] modelname.*\n",argv[0]);

    printf("     -t   Texture to force load when loading a mdl file \n");
    printf("     -s   shaderfile will load and compile shaderfile.vert & shaderfile.frag\n");
    printf("     -h   This helptext\n");

}


void loadSimple(char *filename,Camera &camera);


GLint TextureFromFile(const char* path, string directory, bool gamma)
{
     //Generate texture ID and load texture data
    string filename = string(path);
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


int main(int argc, char* argv[])
{
  char *shader_base="shader";
  char vertex_shader[512];
  char fragment_shader[512];
  char geometry_shader[512];
  
#ifdef BCMHOST
  bcm_host_init();
#endif
  memset(state, 0, sizeof(*state));

  oglinit(state);


  screenWidth=state->screen_width;
  screenHeigth=state->screen_height;
  

  if (argc<2)
  {
      printHelp(argc,argv);
  }

  //int ret=glfwInit();
  // if (!ret)
  // {
  //    std::cerr << "Init failed"  << ret << std::endl;
  // }


   

   // Setup some OpenGL options
   //glEnable(GL_DEPTH_TEST);

   glEnable(GL_TEXTURE_2D);


   // Setup the model world
   init_model_proj(state);


   
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



   //strcpy(out_filename,filename);

   char *pExt = strrchr(argv[argc-1], '.');


   if (pExt != NULL)
   {
      if (strcmp(pExt,".mdl")==0)
      {
          loadSimple(argv[argc-1],camera);
      } else if (strcmp(pExt,".pkg")==0) {
          loadPkg(argv[argc-1],camera);
      }
      else
      {
	// mesh = new Mesh(argv[argc-1]);
      }

   }
   else
   {
              printf("Please specify model to load \n");
   }
   createSurface();


   //setupTestData();


   glColor4f(0.8f, 0.1f, 0.1f,1.0f);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);


   shader.linkProg();
   shader.Use();

   float time=0.0f;
#ifdef _X11_XLIB_H_
   Display *XDisplay;

   XDisplay = XOpenDisplay(NULL);
   if (!XDisplay) {
       fprintf(stderr, "Error: failed to open X display.\n");
       return -1;
   }

   Atom XWMDeleteMessage =
       XInternAtom(XDisplay, "WM_DELETE_WINDOW", False);


    while (0) {

        XEvent event;

        XNextEvent(XDisplay, &event);

        if ((event.type == MotionNotify) ||
            (event.type == Expose))
        {
            printf(".\n");
            glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            //glBindBuffer(GL_ARRAY_BUFFER, state->buf);
            //glDrawArrays( GL_TRIANGLES, 0, 3);
            //eglSwapBuffers(state->display,state->surface);

            //Redraw(state->screen_width, state->screen_height);
        }
        else if (event.type == ClientMessage) {
            if (event.xclient.data.l[0] == XWMDeleteMessage)
                break;
        }
    }
    //XSetWMProtocols(XDisplay, XWindow, &XWMDeleteMessage, 0);
#endif



   while (true) {

     glBindFramebuffer(GL_FRAMEBUFFER,0);
     glLoadIdentity();
     // move camera back to see the cube
     glTranslatef(0.f, 0.f, -10.0f);

     
    // Set frame time
      GLfloat currentFrame = time;
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
      //glfwPollEvents();
      //Do_Movement();


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
          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, texture1);
          glUniform1i(glGetUniformLocation(shader.Program, "tex"), 0);  // Texture unit 0 is for base images.

          // OLAS HERE!!  glBindVertexArrayOES(VAO);
	  // glDrawElements(GL_TRIANGLES, mdl_index_count, GL_UNSIGNED_SHORT, 0);

           glBindBuffer(GL_ARRAY_BUFFER, state->buf);	  
           glDrawArrays( GL_TRIANGLES, 0, 3);
      }

      //if (mesh) mesh->render(shader.Program);
      //glfwSwapBuffers(window);
      eglSwapBuffers(state->display,state->surface);
}


return 0;
}

#pragma region "User input"
#if 0
 
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
 
#endif
 
#pragma endregion



