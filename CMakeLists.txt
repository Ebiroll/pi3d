cmake_minimum_required(VERSION 2.4)
PROJECT(pi3d)
#SET (CMAKE_BUILD_TYPE Release)
#SET (CMAKE_BUILD_TYPE RelWithDebInfo)
SET (CMAKE_BUILD_TYPE Debug)

# Test if we are running on raspberry-pi -D BCMHOST
include(CheckIncludeFile)
CHECK_INCLUDE_FILE(/opt/vc/include/bcm_host.h BCMHOST)


# Cmake 3.0 and above
#find_package(GLEW REQUIRED)
#if (GLEW_FOUND)
#    include_directories(${GLEW_INCLUDE_DIRS})
#    link_libraries(${GLEW_LIBRARIES})
#endif()

#find_path(GLEW_INCLUDE_DIR GL/glew.h)
#find_library(GLEW_LIBRARY NAMES GLEW glew32 glew glew32s PATH_SUFFIXES lib64)

#SET (LIBRARY_TYPE STATIC)

IF(WIN32)
ELSE(WIN32)
set(CMAKE_CXX_FLAGS "-std=c++0x -pthread")
#LINK_FLAGS
#  -pthread 
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined -pthread -lm")
ENDIF(WIN32)


INCLUDE_DIRECTORIES(
  glew/include
  glfw/include
  assimp/include
  glm
  soil
  /home/olas/work/si_vis/glfw/include
  /home/olas/work/si_vis/glew/include
)


INCLUDE_DIRECTORIES(
src
)

SET(SOIL_SRC
soil/image_DXT.c
soil/SOIL.h
soil/image_DXT.h
soil/stbi_DDS_aug_c.h
soil/image_helper.c
soil/stbi_DDS_aug.h
soil/image_helper.h
soil/stb_image_aug.c
soil/SOIL.c
soil/stb_image_aug.h
)

ADD_EXECUTABLE(pi3d
${SOIL_SRC}
src/hash.cpp
src/Mesh.cpp
src/main.cpp
src/pkg.cpp
src/Mesh.h
src/shader.frag
src/shader.vert
src/normal.frag
src/normal.vert
src/normal.geom
src/shader.h
src/shader.cpp
)


# Test GLES
#add_definitions(-DHAVEGLES -DHAVEGLES)


#ADD_EXECUTABLE(gles3d
#${SOIL_SRC}
#src/hash.cpp
#src/pi3d.cpp
#src/pipkg.cpp
#src/eglstate.h
#src/camera.h
#src/eglinit.c
#src/shader.frag
#src/shader.vert
#src/normal.frag
#src/normal.vert
#src/normal.geom
#src/shader.h
#src/shader.cpp
#)


ADD_EXECUTABLE(create
${SOIL_SRC}
src/hash.cpp
src/Mesh.cpp
src/create.cpp
src/pkg.cpp
src/Mesh.h
src/shader.frag
src/shader.vert
src/normal.frag
src/normal.vert
src/normal.geom
src/shader.h
src/shader.cpp
)

ADD_EXECUTABLE(ass2pkg
src/hash.cpp
src/ass2pkg.cpp
)


add_subdirectory(assimp)
#add_subdirectory(glfw)
#add_subdirectory(glew)



IF(WIN32)
TARGET_LINK_LIBRARIES(pi3d opengl32 glfw X11 Xext) 
ELSE(WIN32)
TARGET_LINK_LIBRARIES(pi3d assimp m GL glfw GLEW X11 Xext)
#TARGET_LINK_LIBRARIES(pi3d assimp m GL glfw /home/olof/work/pi3d/glew/lib/libGLEW.a  X11 Xext) 
#TARGET_LINK_LIBRARIES(create assimp m GL glfw /home/olof/work/pi3d/glew/lib/libGLEW.a  X11 Xext)
TARGET_LINK_LIBRARIES(create assimp m GL glfw GLEW  X11 Xext)
#TARGET_LINK_LIBRARIES(pi3d assimp m GL /home/olas/work/si_vis/glfw/build/src/libglfw3.a /home/olas/work/si_vis/glew/lib/libGLEW.a  X11 Xext Xinerama Xi Xcursor dl Xrandr GL) 
#TARGET_LINK_LIBRARIES(create assimp m GL /home/olas/work/si_vis/glfw/build/src/libglfw3.a /home/olas/work/si_vis/glew/lib/libGLEW.a  X11 Xext Xinerama Xi Xcursor dl Xrandr GL)
TARGET_LINK_LIBRARIES(ass2pkg assimp)

#TARGET_LINK_LIBRARIES(gles3d  m GL GLESv2 EGL X11)


ENDIF(WIN32)






