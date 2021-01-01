#ifndef _OPENGL_
#define _OPENGL_

void APIENTRY hooked_glBegin(GLenum mode);
void APIENTRY hooked_glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
BOOL APIENTRY hooked_wglSwapBuffers(HDC hdc);
void APIENTRY hooked_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
void APIENTRY hooked_glClear(GLbitfield mask);
void APIENTRY hooked_glPopMatrix(void);
void APIENTRY hooked_glEnable(GLenum mode);
void APIENTRY hooked_glDisable(GLenum mode);
void APIENTRY hooked_glEnd(void);
void APIENTRY hooked_glVertex3f(GLfloat x, GLfloat y, GLfloat z);
void APIENTRY hooked_glVertex2f(GLfloat x, GLfloat y);
void APIENTRY hooked_glVertex3fv(const GLfloat* v);
void APIENTRY hooked_glPushMatrix(void);
void APIENTRY hooked_glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void APIENTRY hooked_glBlendFunc(GLenum sfactor, GLenum dfactor);
void APIENTRY hooked_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);

void InitOpenGL();

#endif