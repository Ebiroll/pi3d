/*
 * Copyright (c) 2011-2013 Luc Verhaegen <libv@skynet.be>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * Opens a EGL context and tries to do, eglBindAPI(EGL_OPENVG_API);
 * This has never been tested but migth work as a replacement for oglinit.c 
 * for non Broadcom boards with OpenVG support.
 * An option for those with GLES but no OpenVG could be to port ShivaVG to GLES
 * http://pandorawiki.org/Porting_to_GLES_from_GL
 */

#define HAVEGLES 1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
//#include <GLES/gl.h>
//#include "eglstate.h"
#include <assert.h>
//#include <openvg.h>

#include "eglstate.h"

#define WIDTH 800
#define HEIGHT 800

#ifdef _X11_XLIB_H_
Display *XDisplay;
Window XWindow;
#else
struct mali_native_window native_window = {
	.width = WIDTH,
	.height = HEIGHT,
};
#endif
/*
          from oglinit.c

		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE
*/

static EGLint const config_attribute_list[] = {
	EGL_RED_SIZE, 8,
	EGL_GREEN_SIZE, 8,
	EGL_BLUE_SIZE, 8,
	EGL_ALPHA_SIZE, 8,
    //EGL_BUFFER_SIZE, 32,
    //EGL_STENCIL_SIZE, 0,
    EGL_DEPTH_SIZE, 16,

    //  EGL_SAMPLES, 4,
    EGL_RENDERABLE_TYPE,
    EGL_OPENGL_ES2_BIT,
    EGL_SURFACE_TYPE,
    EGL_WINDOW_BIT | EGL_PIXMAP_BIT,

	EGL_NONE
};

static EGLint window_attribute_list[] = {
	EGL_NONE
};

static const EGLint context_attribute_list[] = {
	EGL_CONTEXT_CLIENT_VERSION, 2,
	EGL_NONE
};


void oglinit(STATE_T * state) 
{
	EGLint egl_major, egl_minor;
	EGLConfig config;
	EGLint num_config;
	//EGLContext context;
	EGLBoolean result;

#ifdef _X11_XLIB_H_
	XDisplay = XOpenDisplay(NULL);
	if (!XDisplay) {
		fprintf(stderr, "Error: failed to open X display.\n");
        return;
	}

	Window XRoot = DefaultRootWindow(XDisplay);

	XSetWindowAttributes XWinAttr;
	XWinAttr.event_mask  =  ExposureMask | PointerMotionMask;

	XWindow = XCreateWindow(XDisplay, XRoot, 0, 0, WIDTH, HEIGHT, 0,
				CopyFromParent, InputOutput,
				CopyFromParent, CWEventMask, &XWinAttr);

    state->screen_width=WIDTH;
    state->screen_height=HEIGHT;


	Atom XWMDeleteMessage =
		XInternAtom(XDisplay, "WM_DELETE_WINDOW", False);

	XMapWindow(XDisplay, XWindow);
    XStoreName(XDisplay, XWindow, "GLES test");
	XSetWMProtocols(XDisplay, XWindow, &XWMDeleteMessage, 1);

    state->display = eglGetDisplay((EGLNativeDisplayType) XDisplay);
#else
	state->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
#endif /* _X11_XLIB_H_ */
    if (state->display == EGL_NO_DISPLAY) {
		fprintf(stderr, "Error: No display found!\n");
		return ;
	}

	if (!eglInitialize(state->display, &egl_major, &egl_minor)) {
		fprintf(stderr, "Error: eglInitialise failed!\n");
		return;
	}

	
	printf("EGL Version: \"%s\"\n",
	       eglQueryString(state->display, EGL_VERSION));
	printf("EGL Vendor: \"%s\"\n",
	       eglQueryString(state->display, EGL_VENDOR));
	printf("EGL Extensions: \"%s\"\n",
	       eglQueryString(state->display, EGL_EXTENSIONS));


    result=eglBindAPI(EGL_OPENGL_ES_API);

	if (result==EGL_FALSE) {
		fprintf(stderr, "Error: eglBindAPI failed: 0x%08X\n",
			eglGetError());
	}

	
    eglChooseConfig(state->display, config_attribute_list, &config, 1,
        &num_config);

    if (num_config==0)
    {
        fprintf(stderr, "Error: eglChooseConfig found no usable config: 0x%08X\n",
            eglGetError());
    }


       //EGLConfig config;



    state->context = eglCreateContext(state->display, config, EGL_NO_CONTEXT, context_attribute_list);
	assert(state->context != EGL_NO_CONTEXT);

	
	//context = eglCreateContext(egl_display, config, EGL_NO_CONTEXT,
	//			   context_attribute_list);
	//if (context == EGL_NO_CONTEXT) {
	//	fprintf(stderr, "Error: eglCreateContext failed: 0x%08X\n",
	//		eglGetError());
	//	return -1;
	//}


	
	
#ifdef _X11_XLIB_H_


    state->surface = eglCreateWindowSurface(state->display, config, (EGLNativeWindowType)XWindow,
                         NULL);
    if ( state->surface == EGL_NO_SURFACE )
    {
       fprintf(stderr,"eglCreateWindowSurface failed %d\n", eglGetError());
    }
    assert(state->surface != EGL_NO_SURFACE);
#else
	//egl_surface = eglCreateWindowSurface(egl_display, config,
	//				     &native_window,
	//				     window_attribute_list);

	state->surface = eglCreateWindowSurface(state->display, config, &native_window, window_attribute_list);
	assert(state->surface != EGL_NO_SURFACE);

	
#endif
	//if (egl_surface == EGL_NO_SURFACE) {
	//	fprintf(stderr, "Error: eglCreateWindowSurface failed: "
	//		"0x%08X\n", eglGetError());
	//	return -1;
	//}

	//if (!eglQuerySurface(egl_display, egl_surface, EGL_WIDTH, &width) ||
	//    !eglQuerySurface(egl_display, egl_surface, EGL_HEIGHT, &height)) {
	//	fprintf(stderr, "Error: eglQuerySurface failed: 0x%08X\n",
	//		eglGetError());
	//	return -1;
	//}
	//printf("Surface size: %dx%d\n", width, height);

	//if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, context)) {
	//	fprintf(stderr, "Error: eglMakeCurrent() failed: 0x%08X\n",
	//		eglGetError());
	//	return -1;
	//}


    result = eglSurfaceAttrib(state->display, state->surface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_PRESERVED);
    assert(EGL_FALSE != result);


	// connect the context to the surface
	result = eglMakeCurrent(state->display, state->surface, state->surface, state->context);



    if ( result == EGL_FALSE )
    {
       fprintf(stderr,"eglMakeCurrent failed 0x%08X\n", eglGetError());
    }

	assert(EGL_FALSE != result);

	
	printf("GL Vendor: \"%s\"\n", glGetString(GL_VENDOR));
	printf("GL Renderer: \"%s\"\n", glGetString(GL_RENDERER));
	printf("GL Version: \"%s\"\n", glGetString(GL_VERSION));
	printf("GL Extensions: \"%s\"\n", glGetString(GL_EXTENSIONS));


	glClearColor(0.2, 0.2, 0.2, 1.0);

	
	// set up screen ratio
	glViewport(0, 0, (GLsizei) state->screen_width, (GLsizei) state->screen_height);

	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();

	//float ratio = (float)state->screen_width / (float)state->screen_height;
	//glFrustumf(-ratio, ratio, -1.0f, 1.0f, 1.0f, 10.0f);

	
#ifdef _X11_XLIB_H_
#if 0
	while (1) {
		XEvent event;

		XNextEvent(XDisplay, &event);

		if ((event.type == MotionNotify) ||
		    (event.type == Expose))
        {
            printf(".\n");
            //Redraw(state->screen_width, state->screen_height);
        }
		else if (event.type == ClientMessage) {
			if (event.xclient.data.l[0] == XWMDeleteMessage)
				break;
		}
	}
	XSetWMProtocols(XDisplay, XWindow, &XWMDeleteMessage, 0);
#endif
#endif
	return;
}

 
