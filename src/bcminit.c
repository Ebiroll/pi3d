//#ifdef __arm__
#include <EGL/egl.h>
#include <GLES/gl.h>
#define HAVEGLES 1
#include "eglstate.h"
#include <bcm_host.h>
#include <assert.h>

// oglinit sets the display, OpenGL|ES context and screen information
// state holds the OGLES model information
extern void oglinit(STATE_T * state) {
	int32_t success = 0;
	EGLBoolean result;
	EGLint num_config;

	static EGL_DISPMANX_WINDOW_T nativewindow;

	DISPMANX_ELEMENT_HANDLE_T dispman_element;
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_UPDATE_HANDLE_T dispman_update;
	VC_RECT_T dst_rect;
	VC_RECT_T src_rect;

	static const EGLint attribute_list[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE
	};

	static const EGLint context_attributes[] = 
        {
          EGL_CONTEXT_CLIENT_VERSION, 2,
          EGL_NONE
        };
	EGLConfig config;

	// get an EGL display connection
	state->display  = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(state->display != EGL_NO_DISPLAY);

	// initialize the EGL display connection
	result = eglInitialize(state->display, NULL, NULL);
	assert(EGL_FALSE != result);

	// bind OpenVG API
	//eglBindAPI(EGL_OPENVG_API);

	eglBindAPI(EGL_OPENGL_ES_API);
        //result = eglBindAPI(EGL_OPENGL_ES_API);
        //assert(EGL_FALSE != result);
        //check();

	

	// get an appropriate EGL frame buffer configuration
	result = eglChooseConfig(state->display, attribute_list, &config, 1, &num_config);
	assert(EGL_FALSE != result);

	
	// create an EGL rendering context
	state->context = eglCreateContext(state->display, config, EGL_NO_CONTEXT, context_attributes);
	assert(state->context != EGL_NO_CONTEXT);

	// create an EGL window surface
	success = graphics_get_display_size(0 /* LCD */ , &state->screen_width,
					    &state->screen_height);
	assert(success >= 0);

	dst_rect.x = 0;
	dst_rect.y = 0;
	dst_rect.width = state->screen_width;
	dst_rect.height = state->screen_height;

	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.width = state->screen_width << 16;
	src_rect.height = state->screen_height << 16;

	dispman_display = vc_dispmanx_display_open(0 /* LCD */ );
	dispman_update = vc_dispmanx_update_start(0);

	dispman_element = vc_dispmanx_element_add(dispman_update, dispman_display, 0 /*layer */ , &dst_rect, 0 /*src */ ,
						  &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha */ , 0 /*clamp */ ,
						  0 /*transform */ );

	nativewindow.element = dispman_element;
	nativewindow.width = state->screen_width;
	nativewindow.height = state->screen_height;
	vc_dispmanx_update_submit_sync(dispman_update);

	state->surface = eglCreateWindowSurface(state->display, config, &nativewindow, NULL);
	assert(state->surface != EGL_NO_SURFACE);

	// preserve the buffers on swap
	//result = eglSurfaceAttrib(state->display, state->surface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_PRESERVED);
	//assert(EGL_FALSE != result);

	// connect the context to the surface
	result = eglMakeCurrent(state->display, state->surface, state->surface, state->context);
	assert(EGL_FALSE != result);

	// set up screen ratio
	glViewport(0, 0, (GLsizei) state->screen_width, (GLsizei) state->screen_height);

	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
        //glMatrixMode(GL_MODELVIEW);

	glClearColor(0.15f, 0.25f, 0.35f, 1.0f);

	// Enable back face culling.
	glEnable(GL_CULL_FACE);

        const char *s;
	
        s = eglQueryString(state->display, EGL_EXTENSIONS);
        printf("EGL_EXTENSIONS = %s\n", s);

        s = eglQueryString(state->display, EGL_CLIENT_APIS);
        printf("EGL_CLIENT_APIS = %s\n", s);
	
	
	float ratio = (float)state->screen_width / (float)state->screen_height;
	glFrustumf(-ratio, ratio, -1.0f, 1.0f, 1.0f, -10.0f);


	glClearColor(0.45f, 0.45f, 0.45f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT );
	
}
//#endif
