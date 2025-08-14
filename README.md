# pi3d
3D viewer for raspberry or other GLES/OpenGL linux using the Open Asset Import Library
pi3d is an openGL/GLES viewer. The goal of this project is to make it possible to view 3d models on a raspberry.

// This connects to a ESP8266 with the sketch loaded in the sketch direcory
Now it also connects to a ESP8266 with a BNO055 sensor.
https://learn.adafruit.com/bno055-absolute-orientation-sensor-with-raspberry-pi-and-beaglebone-black/hardware?view=all

Here you can test the code in a browser. https://ebiroll.github.io/pi3d/index.html
Scroll down to Live Demo.


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

To run the nice looking test shaders (built with cmake) try,

    ./create    -s ../shader/creation  
    ./create  -t ../test/drkwood2.png  -s ../shader/tunnel 
    
These programs will need X-Windows to run     


On the raspberry, make will build pi3d ,triangle & triangle2.

These programs uses EGL2 and does not need X-Windows to run.
Try it on raspberry  with:
./pi3d -t test/a320_ein.pkg
./triangle2


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
    # You need to fix Cmakefile to build this
    ../../build/gles3d  vehicle-ifv-dmm08.obj
