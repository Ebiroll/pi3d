# pi3d
3D viewer for raspberry or other GLES/OpenGL linux using the Open Asset Import Library
pi3d is an openGL/GLES viewer. The goal of this project is to make it possible to view 3d models on a raspberry.

## Build

It uses assview and glm included as a submodules. Try


    git submodule init
    git submodule update 
    
    
Here are some other dependencies

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

On raspbian

    sudo apt-get install cmake
    sudo apt-get install libglew-dev
    sudo apt-get install  libglfw3-dev

To build


    mkdir build
    cd build
    cmake ..
    #To run
    pi3d ../test/tri_cube.blend

To run the nice looking test shaders try,

    ./create    -s ../shader/creation  
    ./create  -t ../test/drkwood2.png  -s ../shader/tunnel 
    
These programs will need X-Windows to run     


On the raspberry, make will build pi3d ,triangle & triangle2.

These programs uses EGL2 and does not need X-Windows to run.
Try it with:
./pi3d -t test/push_back_car.dds  test/Towcar.mdl

## PKG and MDL file format

The program ass2pkg will use assimp to read a 3d model. Each mesh will generate a meshname.mdl file and they will be packaged into a filename.pkg file containig all the meshes and images used. Only one image per mesh and the filename is used to generate a hash that is stored in the mdl file.

The Cmake version of pi3d reads all file formats supported by assimp.
The Makefile version of pi3d only reads mdl and pkg files.

To illustrate with an example:

    # Go to directory where model and images are stored.
    cd model/tank
    # To generate pkg file
    ../../build/ass2pkg tank.blend
    # To draw pkg file with accelerated opengl
    ../../pi3d tank.pkg
    # To draw using software-mesa version with assimp loading
    ../../build/pi3d  vehicle-ifv-dmm08.obj
