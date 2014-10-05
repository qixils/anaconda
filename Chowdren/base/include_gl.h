#ifndef INCLUDE_GL_H
#define INCLUDE_GL_H

#ifdef CHOWDREN_IS_DESKTOP
#ifdef CHOWDREN_USE_GL
#include <SDL_opengl.h>

extern PFNGLBLENDEQUATIONSEPARATEEXTPROC __glBlendEquationSeparateEXT;
extern PFNGLBLENDEQUATIONEXTPROC __glBlendEquationEXT;
extern PFNGLBLENDFUNCSEPARATEEXTPROC __glBlendFuncSeparateEXT;
extern PFNGLACTIVETEXTUREARBPROC __glActiveTextureARB;
extern PFNGLMULTITEXCOORD2FARBPROC __glMultiTexCoord2fARB;
extern PFNGLGENFRAMEBUFFERSEXTPROC __glGenFramebuffersEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC __glFramebufferTexture2DEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC __glBindFramebufferEXT;
extern PFNGLUSEPROGRAMPROC __glUseProgram;
extern PFNGLDETACHSHADERPROC __glDetachShader;
extern PFNGLGETPROGRAMINFOLOGPROC __glGetProgramInfoLog;
extern PFNGLGETPROGRAMIVPROC __glGetProgramiv;
extern PFNGLLINKPROGRAMPROC __glLinkProgram;
extern PFNGLCREATEPROGRAMPROC __glCreateProgram;
extern PFNGLATTACHSHADERPROC __glAttachShader;
extern PFNGLGETSHADERINFOLOGPROC __glGetShaderInfoLog;
extern PFNGLGETSHADERIVPROC __glGetShaderiv;
extern PFNGLCOMPILESHADERPROC __glCompileShader;
extern PFNGLSHADERSOURCEPROC __glShaderSource;
extern PFNGLCREATESHADERPROC __glCreateShader;
extern PFNGLUNIFORM1IPROC __glUniform1i;
extern PFNGLUNIFORM2FPROC __glUniform2f;
extern PFNGLUNIFORM1FPROC __glUniform1f;
extern PFNGLUNIFORM4FPROC __glUniform4f;
extern PFNGLGETUNIFORMLOCATIONPROC __glGetUniformLocation;

#elif CHOWDREN_USE_GLES1
#include <SDL_opengles.h>

#elif CHOWDREN_USE_GLES2
#include <SDL_opengles2.h>

#else // CHOWDREN_USE_GL
#define USE_GENERIC_GLC
#include "generic_glc.h"
#endif // CHOWDREN_USE_GL

#else // CHOWDREN_IS_DESKTOP

// include platform-specific glc
#include "glc.h"

#endif // CHOWDREN_IS_DESKTOP

void glc_enable(GLenum cap);
void glc_disable(GLenum cap);
void glc_begin(GLenum mode);
void glc_ortho(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top,
               GLfloat near, GLfloat far);
void glc_load_identity();
void glc_color_4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
void glc_color_4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void glc_matrix_mode(GLenum mode);
void glc_translate_3f(GLfloat x, GLfloat y, GLfloat z);
void glc_scale_f(GLfloat x, GLfloat y, GLfloat z);
void glc_push_matrix();
void glc_pop_matrix();
void glc_vertex_3f(GLfloat x, GLfloat y, GLfloat z);
void glc_end();
void glc_multi_texcoord_2f(GLenum target, GLfloat s, GLfloat t);
void glc_texcoord_2f(GLfloat s, GLfloat t);
void glc_rotate_f(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);

#ifdef CHOWDREN_USE_GLES2
#define glPushMatrix glc_push_matrix
#define glMatrixMode glc_matrix_mode
#define glPopMatrix glc_pop_matrix
#define glLoadIdentity glc_load_identity
#define glColor4ub glc_color_4ub
#define glColor4f glc_color_4f
#define glOrtho glc_ortho
#define glTranslatef glc_translate_3f
#define glTranslated glc_translate_3f
#define glScalef glc_scale_f
#define glScaled glc_scale_f
#define glRotated glc_rotate_f
#endif

#if defined(CHOWDREN_USE_GLES1) || defined(CHOWDREN_USE_GLES2)
#define glBegin glc_begin
#define glEnd glc_end
#define glVertex2f(x, y) glc_vertex_3f(x, y, 0.0f)
#define glVertex3f glc_vertex_3f
#define glVertex2i(x, y) glc_vertex_3f(x, y, 0.0f)
#define glVertex2d(x, y) glc_vertex_3f(x, y, 0.0f)
#define glMultiTexCoord2f glc_multi_texcoord_2f
#define glTexCoord2f glc_texcoord_2f
#define GL_CLAMP GL_CLAMP_TO_EDGE

#endif

#ifndef CHOWDREN_BUILD_GLC

#if defined(CHOWDREN_USE_GL)
#define glBlendEquation __glBlendEquationEXT
#define glBlendEquationSeparate __glBlendEquationSeparateEXT
#define glBlendFuncSeparate __glBlendFuncSeparateEXT
#define glActiveTexture __glActiveTextureARB
#define glMultiTexCoord2f __glMultiTexCoord2fARB
#define glGenFramebuffers __glGenFramebuffersEXT
#define glBindFramebuffer __glBindFramebufferEXT
#define glFramebufferTexture2D __glFramebufferTexture2DEXT
#define glUseProgram __glUseProgram
#define glDetachShader __glDetachShader
#define glGetProgramInfoLog __glGetProgramInfoLog
#define glGetProgramiv __glGetProgramiv
#define glLinkProgram __glLinkProgram
#define glCreateProgram __glCreateProgram
#define glAttachShader __glAttachShader
#define glGetShaderInfoLog __glGetShaderInfoLog
#define glGetShaderiv __glGetShaderiv
#define glCompileShader __glCompileShader
#define glShaderSource __glShaderSource
#define glCreateShader __glCreateShader
#define glUniform1i __glUniform1i
#define glUniform2f __glUniform2f
#define glUniform1f __glUniform1f
#define glUniform4f __glUniform4f
#define glGetUniformLocation __glGetUniformLocation

#endif

#ifdef CHOWDREN_USE_GLES2
#define glDisable glc_disable
#define glEnable glc_enable
#endif

#define glPopAttrib glPopAttribUndef
#define glPushAttrib glPushAttribUndef

#endif // CHOWDREN_BUILD_GLC

#ifdef CHOWDREN_USE_GLES2
#define GL_MODELVIEW 0x1700
#define GL_PROJECTION 0x1701
#define GL_QUADS 0x0007
#endif

#endif // INCLUDE_GL_H
