INCLUDEFLAGS=-I/opt/vc/include -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/include/interface/vcos/pthreads -I./ -I./src  -I./soil -I./glm

CFLAGS+=-DSTANDALONE -DHAVEGLES -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -Wall -g -DHAVE_LIBOPENMAX=2 -DOMX -DOMX_SKIP64BIT -ftree-vectorize -pipe -DUSE_EXTERNAL_OMX -DHAVE_LIBBCM_HOST -DUSE_EXTERNAL_LIBBCM_HOST -DUSE_VCHIQ_ARM -Wno-psabi 

CXXFLAGS+=-DSTANDALONE -DHAVEGLES -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -Wall -g -DHAVE_LIBOPENMAX=2 -DOMX -DOMX_SKIP64BIT -ftree-vectorize -pipe -DUSE_EXTERNAL_OMX -DHAVE_LIBBCM_HOST -DUSE_EXTERNAL_LIBBCM_HOST -DUSE_VCHIQ_ARM -Wno-psabi -std=c++11 -pthread


LIBFLAGS+=-L/opt/vc/lib/ -lGLESv2 -lEGL -lopenmaxil -lbcm_host -lvcos -lvchiq_arm -lpthread -lrt -lm 


INCLUDES+=-I/opt/vc/include/ -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux -I./ -I./src  -I./soil


CGOFILES = openvg.go

all:	pi3d triangle triangle2

pi3d: pi3d.o bcminit.o pipkg.o shader.o libsoil.a
	g++  -g  $(LIBFLAGS) -lbcm_host -lvchiq_arm -ljpeg -lpthread -lrt -lm  bcminit.o pi3d.o pipkg.o shader.o libsoil.a  -o pi3d

triangle: triangle.o shader.o
	g++  -g  $(LIBFLAGS) -lbcm_host -lvchiq_arm -ljpeg -lpthread -lrt -lm  triangle.o shader.o  -o triangle

triangle2: triangle2.o shader.o
	g++  -g  $(LIBFLAGS) -lbcm_host -lvchiq_arm -ljpeg -lpthread -lrt -lm  triangle2.o shader.o  -o triangle2


triangle.o:	src/triangle.cpp
	g++ $(CXXFLAGS) -O2 -fPIC -Wall $(INCLUDEFLAGS) -D BCMHOST -c src/triangle.cpp

triangle2.o:	src/triangle2.cpp
	g++ $(CXXFLAGS) -O2 -fPIC -Wall $(INCLUDEFLAGS) -D BCMHOST -c src/triangle2.cpp

pi3d.o:	src/pi3d.cpp
	g++ $(CXXFLAGS) -O2 -fPIC -Wall $(INCLUDEFLAGS) -D BCMHOST -c src/pi3d.cpp

shader.o:	src/shader.cpp
	g++ $(CXXFLAGS) -O2 -fPIC -Wall $(INCLUDEFLAGS) -D BCMHOST -c src/shader.cpp

pipkg.o:	src/pipkg.cpp
	g++ $(CXXFLAGS) -O2 -fPIC -Wall $(INCLUDEFLAGS) -D BCMHOST -c src/pipkg.cpp


libsoil.a:
	gcc $(CFLAGS) -O2 -fPIC -Wall $(INCLUDEFLAGS) -D BCMHOST -c soil/SOIL.c
	gcc $(CFLAGS) -O2 -fPIC -Wall $(INCLUDEFLAGS) -D BCMHOST -c soil/image_DXT.c
	gcc $(CFLAGS) -O2 -fPIC -Wall $(INCLUDEFLAGS) -D BCMHOST -c soil/image_helper.c
	gcc $(CFLAGS) -O2 -fPIC -Wall $(INCLUDEFLAGS) -D BCMHOST -c soil/stb_image_aug.c
	ar rcs libsoil.a SOIL.o  image_DXT.o image_helper.o stb_image_aug.o



bcminit.o:	src/bcminit.c
	gcc -O2 -fPIC -Wall $(INCLUDEFLAGS) -D BCMHOST -c src/bcminit.c


clean:
	rm -f *.o *.inc *.so font2openvg *.c~ *.h~ video hello
	#indent -linux -c 60 -brf -l 132  libshapes.c bcminit.c shapes.h fontinfo.h

library: oglinit.o libshapes.o video.o
	gcc $(LIBFLAGS) -fPIC -shared -o libshapes.so oglinit.o libshapes.o video.o



