# pi3d
3D viewer for raspberry or other GLES/OpenGL linux using the Open Asset Import Library
pi3d is an openGL/GLES viewer. The goal of this project is to make it possible to view 3d models on a raspberry.

It uses

assview is included as a submodule. Try
    git submodule init
    git submodule update 
    

glm,
Get glm, necessary to build the pi3d viewer. Headers only library.
https://www.opengl.org/sdk/libs/GLM/ Dowmload and put in glm sub-directory

glfw,
http://www.glfw.org

glew,
This does not support GLES. This dependeny will be removed.

