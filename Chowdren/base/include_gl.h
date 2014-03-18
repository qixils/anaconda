#ifndef INCLUDE_GL_H
#define INCLUDE_GL_H

#ifdef _WIN32
#include "windows.h"
#endif

#ifdef CHOWDREN_IS_DESKTOP
#include <GL/glew.h>
#include <SDL_opengl.h>
#elif CHOWDREN_IS_WIIU
#include "wiiu_gl.h"
#endif

#undef max
#undef min

#endif // INCLUDE_GL_H
