#include "shader.h"
#include <string.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include <GL/glew.h>

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
                vec4 color = texture2D(tex, texcoord);
                gl_FragColor = color;
                //gl_FragColor = vec4(0.0,1.0,0.0,1.0);
                //gl_FragColor = vec4(texcoord.x,texcoord.y,0.0,1.0);
            });



Shader::Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath)
{
    geometry=-1;
    // 1. Retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    try
    {
        // Open files
        std::ifstream vShaderFile(vertexPath);
        std::ifstream fShaderFile(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;

        if (vShaderFile.is_open())
        {

            // Read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // Convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
            // If geometry shader path is present, also load a geometry shader
            if(geometryPath != nullptr)
            {
                std::ifstream gShaderFile(geometryPath);
                if (gShaderFile.is_open())
                {
                    std::stringstream gShaderStream;
                    gShaderStream << gShaderFile.rdbuf();
                    gShaderFile.close();
                    geometryCode = gShaderStream.str();
                }
                else
                {
                    geometryPath=NULL;
                }
            }
        }
        else
        {
            std::cout << "No shader file found, using default"  << vertexPath << std::endl;
            vertexCode=std::string(vs,strlen(vs));
            fragmentCode=std::string(fs,strlen(fs));
        }

    }
    catch (std::exception e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const GLchar* vShaderCode = vertexCode.c_str();
    const GLchar * fShaderCode = fragmentCode.c_str();
    // 2. Compile shaders
    GLint success;
    GLchar infoLog[512];
    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // If geometry shader is given, compile geometry shader
    if(geometryPath != nullptr)
    {
        const GLchar * gShaderCode = geometryCode.c_str();
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, NULL);
        glCompileShader(geometry);
        checkCompileErrors(geometry, "GEOMETRY");
    }
    this->Program = glCreateProgram();
    // Shader Program
    glAttachShader(this->Program, vertex);
    glAttachShader(this->Program, fragment);
    if(geometry > -1)
        glAttachShader(this->Program, geometry);

}


void Shader::linkProg()
{
    glLinkProgram(this->Program);
    checkCompileErrors(this->Program, "PROGRAM");
    // Delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if(geometry > -1)
        glDeleteShader(geometry);

}


