# pi3d
3D viewer for raspberry or other GLES/OpenGL linux using the Open Asset Import Library
pi3d is an openGL/GLES viewer. The goal of this project is to make it possible to view 3d models on a raspberry.

It uses assview and glm included as a submodules. Try
   git submodule init
   git submodule update 
    
Here are some links

glm,
Get glm, necessary to build the pi3d viewer. Headers only library.
https://www.opengl.org/sdk/libs/GLM/ 

glfw,
http://www.glfw.org

glew,
This does not support GLES. This dependeny will be removed.

On archlinux you install the dependecies with
   pacman -S glfw
   pacman -Ss glew

To build
   mkdir build
	cd build
	cmake ..
	#To run
	pi3d ../test/tri_cube.blend