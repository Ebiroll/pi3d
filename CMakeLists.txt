cmake_minimum_required(VERSION 2.4)
PROJECT(pi3d)
#SET (CMAKE_BUILD_TYPE Release)
#SET (CMAKE_BUILD_TYPE RelWithDebInfo)
SET (CMAKE_BUILD_TYPE Debug)

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
set(CMAKE_CXX_FLAGS " -pthread")
#LINK_FLAGS
#  -pthread -std=c++11
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined -pthread")
ENDIF(WIN32)




INCLUDE_DIRECTORIES(
  glew/include
)



INCLUDE_DIRECTORIES(
src
)

ADD_LIBRARY(SOIL
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
src/Mesh.cpp  
src/main.cpp
src/Mesh.h  
src/shader.frag
src/shader.vert
src/normal.frag
src/normal.vert
src/normal.geom
src/shader.h
)


TARGET_LINK_LIBRARIES(pi3d  GLEW  glfw3 X11 Xext) 


