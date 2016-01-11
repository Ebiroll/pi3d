#ifndef EGLSTATE_H
#define EGLSTATE_H 1
#include <inttypes.h>

typedef struct {
	uint32_t screen_width;
	uint32_t screen_height;
	// OpenGL|ES objects
#ifdef HAVEGLES
    //GLuint buf;
    //GLuint uvbuffer;

    GLuint attr_position;
    GLuint attr_vtex;

	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
#endif
} STATE_T;

#ifdef HAVEGLES
extern void oglinit(STATE_T *);
#endif

#endif
