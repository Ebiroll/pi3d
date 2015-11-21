#include <inttypes.h>

typedef struct {
	uint32_t screen_width;
	uint32_t screen_height;
	// OpenGL|ES objects
#ifdef __arm__
        GLuint buf;
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
#endif
} STATE_T;

#ifdef __arm__
extern void oglinit(STATE_T *);
#endif
