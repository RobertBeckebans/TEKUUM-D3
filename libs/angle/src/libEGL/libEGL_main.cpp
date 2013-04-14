//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// main.cpp: DLL entry point and management of thread-local data.

#include "libEGL/main.h"

#include "common/debug.h"

#if !defined(ANGLE_STATIC)
static DWORD currentTLS = TLS_OUT_OF_INDEXES;

extern "C" BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
      case DLL_PROCESS_ATTACH:
        {
#if !defined(ANGLE_DISABLE_TRACE)
            FILE *debug = fopen(TRACE_OUTPUT_FILE, "rt");

            if (debug)
            {
                fclose(debug);
                debug = fopen(TRACE_OUTPUT_FILE, "wt");   // Erase
                
                if (debug)
                {
                    fclose(debug);
                }
            }
#endif

            currentTLS = TlsAlloc();

            if (currentTLS == TLS_OUT_OF_INDEXES)
            {
                return FALSE;
            }
        }
        // Fall throught to initialize index
      case DLL_THREAD_ATTACH:
        {
            egl::Current *current = (egl::Current*)LocalAlloc(LPTR, sizeof(egl::Current));

            if (current)
            {
                TlsSetValue(currentTLS, current);

                current->error = EGL_SUCCESS;
                current->API = EGL_OPENGL_ES_API;
                current->display = EGL_NO_DISPLAY;
                current->drawSurface = EGL_NO_SURFACE;
                current->readSurface = EGL_NO_SURFACE;
            }
        }
        break;
      case DLL_THREAD_DETACH:
        {
            void *current = TlsGetValue(currentTLS);

            if (current)
            {
                LocalFree((HLOCAL)current);
            }
        }
        break;
      case DLL_PROCESS_DETACH:
        {
            void *current = TlsGetValue(currentTLS);

            if (current)
            {
                LocalFree((HLOCAL)current);
            }

            TlsFree(currentTLS);
        }
        break;
      default:
        break;
    }

    return TRUE;
}
#endif // #if !defined(ANGLE_STATIC)

namespace egl
{

#if defined(ANGLE_STATIC)
static Current current;
#endif

void setCurrentError(EGLint error)
{
#if defined(ANGLE_STATIC)
	current.error = error;
#else
    Current *current = (Current*)TlsGetValue(currentTLS);

    current->error = error;
#endif
}

EGLint getCurrentError()
{
#if defined(ANGLE_STATIC)
	return current.error;
#else
    Current *current = (Current*)TlsGetValue(currentTLS);

    return current->error;
#endif
}

void setCurrentAPI(EGLenum API)
{
#if defined(ANGLE_STATIC)
	current.API = API;
#else
    Current *current = (Current*)TlsGetValue(currentTLS);

    current->API = API;
#endif
}

EGLenum getCurrentAPI()
{
#if defined(ANGLE_STATIC)
	return current.API;
#else
    Current *current = (Current*)TlsGetValue(currentTLS);

    return current->API;
#endif
}

void setCurrentDisplay(EGLDisplay dpy)
{
#if defined(ANGLE_STATIC)
	current.display = dpy;
#else
    Current *current = (Current*)TlsGetValue(currentTLS);

    current->display = dpy;
#endif
}

EGLDisplay getCurrentDisplay()
{
#if defined(ANGLE_STATIC)
	return current.display;
#else
    Current *current = (Current*)TlsGetValue(currentTLS);

    return current->display;
#endif
}

void setCurrentDrawSurface(EGLSurface surface)
{
#if defined(ANGLE_STATIC)
	current.drawSurface = surface;
#else
    Current *current = (Current*)TlsGetValue(currentTLS);

    current->drawSurface = surface;
#endif
}

EGLSurface getCurrentDrawSurface()
{
#if defined(ANGLE_STATIC)
	return current.drawSurface;
#else
    Current *current = (Current*)TlsGetValue(currentTLS);

    return current->drawSurface;
#endif
}

void setCurrentReadSurface(EGLSurface surface)
{
#if defined(ANGLE_STATIC)
	current.readSurface = surface;
#else
    Current *current = (Current*)TlsGetValue(currentTLS);

    current->readSurface = surface;
#endif
}

EGLSurface getCurrentReadSurface()
{
#if defined(ANGLE_STATIC)
	return current.readSurface;
#else
    Current *current = (Current*)TlsGetValue(currentTLS);

    return current->readSurface;
#endif
}
}

void error(EGLint errorCode)
{
    egl::setCurrentError(errorCode);
}
