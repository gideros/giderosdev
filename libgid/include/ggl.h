#ifndef GGL_H
#define GGL_H

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#define OPENGL_ES
#elif __ANDROID__
#include <GLES/gl.h>
#include <GLES/glext.h>
#define OPENGL_ES
#else
#include <GL/glew.h>
#define OPENGL_DESKTOP
#endif

#include <gglobal.h>

G_API void gglBindTexture(GLenum target, GLuint texture);
G_API void gglBlendFunc(GLenum sfactor, GLenum dfactor);
G_API void gglClear(GLbitfield mask);
G_API void gglClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
G_API void gglColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
G_API void gglDeleteTextures(GLsizei n, const GLuint *textures);
G_API void gglEnable(GLenum cap);
G_API void gglDisable(GLenum cap);
G_API void gglEnableClientState(GLenum array);
G_API void gglDisableClientState(GLenum array);
G_API void gglDrawArrays(GLenum mode, GLint first, GLsizei count);
G_API void gglDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
G_API void gglGenTextures(GLsizei n, GLuint *textures);
G_API void gglLoadMatrixf(const GLfloat *m);
G_API void gglMatrixMode(GLenum mode);
G_API void gglScissor(GLint x, GLint y, GLsizei width, GLsizei height);
G_API void gglTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
G_API void gglTexImage2D(GLenum target,  GLint level,  GLint internalformat,  GLsizei width,  GLsizei height,  GLint border,  GLenum format,  GLenum type,  const GLvoid * pixels);
G_API void gglVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
G_API void gglViewport(GLint x, GLint y, GLsizei width, GLsizei height);

#define PREMULTIPLIED_ALPHA 1

#endif
