#include "client.h"

glBegin_t			pglBegin;
glPopMatrix_t	 	pglPopMatrix;
glPushMatrix_t	 	pglPushMatrix;
glClear_t			pglClear;
glEnable_t			pglEnable;
glDisable_t			pglDisable;
glEnd_t				pglEnd;
glPolygonOffset_t	pglPolygonOffset;
glTranslatef_t		pglTranslatef;
glVertex2f_t		pglVertex2f;
glVertex3f_t	 	pglVertex3f;
glVertex3fv_t	 	pglVertex3fv;
glViewport_t	 	pglViewport;
wglSwapBuffers_t 	pwglSwapBuffers;
glShadeModel_t	 	pglShadeModel;
glFrustum_t         pglFrustum;
glBlendFunc_t       pglBlendFunc;
glColor4f_t         pglColor4f;
glReadPixels_t      pglReadPixels;

void APIENTRY hooked_glBegin(GLenum mode)
{
	cl_entity_s* ent = g_Studio.GetCurrentEntity();
	bool Player = ent && ent->player;
	bool Weapon = ent && ent->model && (strstr(ent->model->name, "p_") || strstr(ent->model->name, "w_"));
	bool View_Model = ent && ent->model && strstr(ent->model->name, "v_");

	if (cvar.visual_wall && CheckDrawEngine())
	{
		if (Player || Weapon)
			glDepthRange(0, 0.5);
		else
			glDepthRange(0.5, 1);
	}

	if (cvar.visual_lambert && (Player && !cvar.chams_player_glow || Weapon && !cvar.chams_world_glow || View_Model && !cvar.chams_view_model_glow) && CheckDrawEngine())
	{
		if (mode == GL_TRIANGLE_STRIP)
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	}
	pglBegin(mode);
}

void APIENTRY hooked_glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	if (chams_world)
	{
		if (cvar.chams_world == 1 || cvar.chams_world == 3)
			red = chams_world_r, green = chams_world_g, blue = chams_world_b;
		if (cvar.chams_world == 2)
			red = chams_world_r * red, green = chams_world_g * green, blue = chams_world_b * blue;
	}
	if (chams_viewmodel)
	{
		if (cvar.chams_view_model == 1 || cvar.chams_view_model == 3)
			red = chams_viewmodel_r, green = chams_viewmodel_g, blue = chams_viewmodel_b;
		if (cvar.chams_view_model == 2)
			red = chams_viewmodel_r * red, green = chams_viewmodel_g * green, blue = chams_viewmodel_b * blue;
	}
	if (chams_player)
	{
		if (cvar.chams_player == 1 || cvar.chams_player == 3)
			red = chams_player_r, green = chams_player_g, blue = chams_player_b;
		if (cvar.chams_player == 2)
			red = chams_player_r * red, green = chams_player_g * green, blue = chams_player_b * blue;
	}
	pglColor4f(red, green, blue, alpha);
}

BOOL APIENTRY hooked_wglSwapBuffers(HDC hdc)
{
	if (hdc)HookImGui(hdc);
	return(*pwglSwapBuffers)(hdc);
}

void APIENTRY hooked_glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	static bool FirstFrame = true;
	if (FirstFrame)
	{
		g_Engine.pfnClientCmd("toggleconsole");

		ConsolePrintColor(255, 255, 255, "\n\n\t\t\t\tHello, %s ;)\n", g_Engine.pfnGetCvarString("name"));
		ConsolePrintColor(255, 255, 255, "\t\t\t\tYou are injected!\n\n");

		ConsolePrintColor(255, 0, 255, "\t\t\t\tMultihack by:\n");
		ConsolePrintColor(255, 0, 255, "\t\t\t\t            [..         [..    [....     [..      \n");
		ConsolePrintColor(255, 0, 255, "\t\t\t\t             [..       [..   [..    [..  [..      \n");
		ConsolePrintColor(255, 0, 255, "\t\t\t\t     [..      [..     [..  [..        [..[..      \n");
		ConsolePrintColor(255, 0, 255, "\t\t\t\t   [.   [..    [..   [..   [..        [..[..      \n");
		ConsolePrintColor(255, 0, 255, "\t\t\t\t  [..... [..    [.. [..    [..        [..[..      \n");
		ConsolePrintColor(255, 0, 255, "\t\t\t\t  [.             [....       [..     [.. [..      \n");
		ConsolePrintColor(255, 0, 255, "\t\t\t\t    [....         [..          [....     [........\n");
		ConsolePrintColor(255, 0, 255, "\t\t\t\tand team!\n\n");

		ConsolePrintColor(255, 255, 255, "\t\t\t\tSpecial thanks to my friend BloodSharp and oxiKKK <3\n\n");
		ConsolePrintColor(255, 255, 255, "\t\t\t\tATTENTION! Menu only active in game!\n");
		if (cvar.gui_key != -1)
			ConsolePrintColor(0, 255, 0, "\t\t\t\tMenu key is [%s]!\n", KeyEventChar((int)cvar.gui_key));

		FirstFrame = false;
	}
	static float ChangeKey = cvar.gui_key;
	if (ChangeKey != cvar.gui_key)
	{
		if (cvar.gui_key == -1)
			ConsolePrintColor(255, 255, 0, "\t\t\t\tMenu key is [Press key]!\n");
		else
			ConsolePrintColor(0, 255, 0, "\t\t\t\tMenu key is [%s]!\n", KeyEventChar((int)cvar.gui_key));

		ChangeKey = cvar.gui_key;
	}
	pglViewport(x, y, width, height);
}

void APIENTRY hooked_glClear(GLbitfield mask)
{
	if (mask == GL_DEPTH_BUFFER_BIT)
		(*pglClear)(GL_COLOR_BUFFER_BIT), glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	pglClear(mask);
}

void APIENTRY hooked_glPopMatrix(void)
{
	pglPopMatrix();
}

void APIENTRY hooked_glEnable(GLenum mode)
{
	pglEnable(mode);
}

void APIENTRY hooked_glDisable(GLenum mode)
{
	pglDisable(mode);
}

void APIENTRY hooked_glEnd(void)
{
	pglEnd();
}

void APIENTRY hooked_glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
	pglVertex3f(x, y, z);
}

void APIENTRY hooked_glVertex2f(GLfloat x, GLfloat y)
{
	pglVertex2f(x, y);
}

void APIENTRY hooked_glVertex3fv(const GLfloat* v)
{
	pglVertex3fv(v);
}

void APIENTRY hooked_glPushMatrix(void)
{
	pglPushMatrix();
}

void APIENTRY hooked_glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	pglFrustum(left, right, bottom, top, zNear, zFar);
}

void APIENTRY hooked_glBlendFunc(GLenum sfactor, GLenum dfactor)
{
	pglBlendFunc(sfactor, dfactor);
}

void APIENTRY hooked_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels)
{
	pglReadPixels(x, y, width, height, format, type, pixels);
}

void CodeWalk(DWORD dwStartAddress, DWORD dwEndAddress)
{
	for (DWORD dwCurrentAddress = dwStartAddress; dwCurrentAddress <= dwEndAddress - 0x6; dwCurrentAddress += 0x6)
	{
		PDWORD pdwTempAddress = (PDWORD)dwCurrentAddress + 0x2;

		PDWORD pdwTableAddress = (PDWORD)* pdwTempAddress;

		HMODULE hmOpenGL = LoadLibrary("opengl32.dll");

		if (*pdwTableAddress == (DWORD)GetProcAddress(hmOpenGL, "glBegin"))
		{
			pglBegin = (glBegin_t)* pdwTableAddress;
			*pdwTableAddress = (DWORD)& hooked_glBegin;
		}
		else if (*pdwTableAddress == (DWORD)GetProcAddress(hmOpenGL, "glPopMatrix"))
		{
			pglPopMatrix = (glPopMatrix_t)* pdwTableAddress;
			*pdwTableAddress = (DWORD)& hooked_glPopMatrix;
		}
		else if (*pdwTableAddress == (DWORD)GetProcAddress(hmOpenGL, "glPushMatrix"))
		{
			pglPushMatrix = (glPushMatrix_t)* pdwTableAddress;
			*pdwTableAddress = (DWORD)& hooked_glPushMatrix;
		}
		else if (*pdwTableAddress == (DWORD)GetProcAddress(hmOpenGL, "glShadeModel"))
		{
			pglShadeModel = (glShadeModel_t)* pdwTableAddress;
		}
		else if (*pdwTableAddress == (DWORD)GetProcAddress(hmOpenGL, "glClear"))
		{
			pglClear = (glClear_t)* pdwTableAddress;
			*pdwTableAddress = (DWORD)& hooked_glClear;
		}
		else if (*pdwTableAddress == (DWORD)GetProcAddress(hmOpenGL, "glEnable"))
		{
			pglEnable = (glEnable_t)* pdwTableAddress;
			*pdwTableAddress = (DWORD)& hooked_glEnable;
		}
		else if (*pdwTableAddress == (DWORD)GetProcAddress(hmOpenGL, "glEnd"))
		{
			pglEnd = (glEnd_t)* pdwTableAddress;
			*pdwTableAddress = (DWORD)& hooked_glEnd;
		}
		else if (*pdwTableAddress == (DWORD)GetProcAddress(hmOpenGL, "glDisable"))
		{
			pglDisable = (glDisable_t)* pdwTableAddress;
			*pdwTableAddress = (DWORD)& hooked_glDisable;
		}
		else if (*pdwTableAddress == (DWORD)GetProcAddress(hmOpenGL, "glVertex2f"))
		{
			pglVertex2f = (glVertex2f_t)* pdwTableAddress;
			*pdwTableAddress = (DWORD)& hooked_glVertex2f;
		}
		else if (*pdwTableAddress == (DWORD)GetProcAddress(hmOpenGL, "glVertex3f"))
		{
			pglVertex3f = (glVertex3f_t)* pdwTableAddress;
			*pdwTableAddress = (DWORD)& hooked_glVertex3f;
		}
		else if (*pdwTableAddress == (DWORD)GetProcAddress(hmOpenGL, "glVertex3fv"))
		{
			pglVertex3fv = (glVertex3fv_t)* pdwTableAddress;
			*pdwTableAddress = (DWORD)& hooked_glVertex3fv;
		}
		else if (*pdwTableAddress == (DWORD)GetProcAddress(hmOpenGL, "glViewport"))
		{
			pglViewport = (glViewport_t)* pdwTableAddress;
			*pdwTableAddress = (DWORD)& hooked_glViewport;
		}
		else if (*pdwTableAddress == (DWORD)GetProcAddress(hmOpenGL, "wglSwapBuffers"))
		{
			pwglSwapBuffers = (wglSwapBuffers_t)* pdwTableAddress;
			*pdwTableAddress = (DWORD)& hooked_wglSwapBuffers;
		}
		else if (*pdwTableAddress == (DWORD)GetProcAddress(hmOpenGL, "glFrustum"))
		{
			pglFrustum = (glFrustum_t)* pdwTableAddress;
			*pdwTableAddress = (DWORD)& hooked_glFrustum;
		}
		else if (*pdwTableAddress == (DWORD)GetProcAddress(hmOpenGL, "glBlendFunc"))
		{
			pglBlendFunc = (glBlendFunc_t)* pdwTableAddress;
			*pdwTableAddress = (DWORD)& hooked_glBlendFunc;
		}
		else if (*pdwTableAddress == (DWORD)GetProcAddress(hmOpenGL, "glColor4f"))
		{
			pglColor4f = (glColor4f_t)* pdwTableAddress;
			*pdwTableAddress = (DWORD)& hooked_glColor4f;
		}
		else if (*pdwTableAddress == (DWORD)GetProcAddress(hmOpenGL, "glVertex3f"))
		{
			pglVertex3f = (glVertex3f_t)* pdwTableAddress;
			*pdwTableAddress = (DWORD)& hooked_glVertex3f;
		}
		else if (*pdwTableAddress == (DWORD)GetProcAddress(hmOpenGL, "glReadPixels"))
		{
			pglReadPixels = (glReadPixels_t)* pdwTableAddress;
			*pdwTableAddress = (DWORD)& hooked_glReadPixels;
		}
	}
}

void OpenGL(void)
{
	DWORD dwImportCode = pMemoryTools->dwFindPattern(0x01D7AC00, 0x000FF000, (BYTE*)"\xA1\xFF\xFF\xFF\xFF\x56\x33\xF6\x3B\xC6\x74\x07\x50", "x????xxxxxxxx");

	if (dwImportCode == NULL) return;
	else
	{
		dwImportCode += 0x13;
		CodeWalk(dwImportCode, dwImportCode + 0x870);
	}
}

void InitOpenGL()
{
	if (GetModuleHandle("opengl32.dll"))
		OpenGL();
}