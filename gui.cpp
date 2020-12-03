#include "client.h"

HWND hGameWnd;
WNDPROC hGameWndProc;
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK HOOK_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ((bInputActive || bShowMenu))
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	return CallWindowProc(hGameWndProc, hWnd, uMsg, wParam, lParam);
}

bool bOldOpenGL = true; 
GLint iMajor, iMinor;
bool bInitializeImGui = false;
void InistalizeImgui(HDC hdc)
{
	if (!bInitializeImGui)
	{
		hGameWnd = WindowFromDC(hdc);
		hGameWndProc = (WNDPROC)SetWindowLong(hGameWnd, GWL_WNDPROC, (LONG)HOOK_WndProc);
		glGetIntegerv(GL_MAJOR_VERSION, &iMajor);
		glGetIntegerv(GL_MINOR_VERSION, &iMinor);
		if ((iMajor * 10 + iMinor) >= 32)
			bOldOpenGL = false;
		ImGui::CreateContext();
		ImGui_ImplWin32_Init(hGameWnd);
		if (!bOldOpenGL)
		{
			ImplementGl3();
			ImGui_ImplOpenGL3_Init();
		}
		else
			ImGui_ImplOpenGL2_Init();
		ImGui::StyleColorsDark();
		ImGui::GetStyle().AntiAliasedFill = !bOldOpenGL ? true : false;
		ImGui::GetStyle().AntiAliasedLines = !bOldOpenGL ? true : false;
		ImGui::GetStyle().FrameRounding = 0.0f;
		ImGui::GetStyle().WindowRounding = 0.0f;
		ImGui::GetStyle().ChildRounding = 0.0f;
		ImGui::GetStyle().ScrollbarRounding = 0.0f;
		ImGui::GetStyle().GrabRounding = 0.0f;
		ImGui::GetStyle().FramePadding = ImVec2(2, 2);
		ImGui::GetStyle().WindowPadding = ImVec2(3, 3);
		ImGui::GetStyle().WindowTitleAlign = ImVec2(0.5f, 0.5f);
		ImGui::GetStyle().DisplaySafeAreaPadding = ImVec2(0.0f, 0.0f);
		ImGui::GetStyle().WindowBorderSize = 0.0f;
		ImGui::GetStyle().FrameBorderSize = 0.0f;
		ImGui::GetStyle().PopupBorderSize = 0.0f;
		ImGui::GetIO().IniFilename = NULL;
		ImGui::GetIO().LogFilename = NULL;

		char fontname[256];
		sprintf(fontname, "%s%s", hackdir, "font/Arialuni.ttf");

		ImFontConfig config;
		config.MergeMode = true;
		ImGui::GetIO().Fonts->AddFontDefault();
		ImGui::GetIO().Fonts->AddFontFromFileTTF(fontname, 13.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesDefault());
		ImGui::GetIO().Fonts->AddFontFromFileTTF(fontname, 13.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());
		ImGui::GetIO().Fonts->AddFontFromFileTTF(fontname, 13.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());
		ImGui::GetIO().Fonts->AddFontFromFileTTF(fontname, 13.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
		ImGui::GetIO().Fonts->Build();
		if (cvar.gui_key < 0 || cvar.gui_key > 254)
			cvar.gui_key = K_INS;

		bInitializeImGui = true;
	}
}

void MenuHandle()
{
	static bool checkmenu = bShowMenu;
	static bool checkchat = bInputActive;
	if (checkmenu != bShowMenu || checkchat != bInputActive)
	{
		if (bShowMenu || bInputActive)
		{
			ImGui::GetIO().MouseDrawCursor = true;
			g_pClient->IN_DeactivateMouse();
		}
		else
		{
			SetCursorPos(g_Engine.GetWindowCenterX(), g_Engine.GetWindowCenterY());
			ImGui::GetIO().MouseDrawCursor = false;
			g_pClient->IN_ActivateMouse();
		}
		checkmenu = bShowMenu;
		checkchat = bInputActive;
	}

	POINT Point;
	if ((bShowMenu || bInputActive) && ::GetCursorPos(&Point) && ::GetActiveWindow() == hGameWnd)
	{
		if (CheckDraw() && Point.x == g_Engine.GetWindowCenterX() && Point.y == g_Engine.GetWindowCenterY())
			g_pClient->IN_DeactivateMouse();
	}
}

void ClearHudKeys()
{
	for (unsigned int i = 0; i < 256; i++)
	{
		if (keysmenu[i] == true)
			keysmenu[i] = false;
	}
}

void ClearSound()
{
	if (Sound_No_Index.size() && GetTickCount() - Sound_No_Index.front().timestamp > 900)
		Sound_No_Index.pop_front();
	if (Sound_Index.size() && GetTickCount() - Sound_Index.front().timestamp > 900)
		Sound_Index.pop_front();
}

void ClearDeque()
{
	PlayerBone.deque::clear();
	PlayerHitbox.deque::clear();
	PlayerEsp.deque::clear();
	WorldBone.deque::clear();
	WorldHitbox.deque::clear();
	Grenadeline.deque::clear();
	Routeline.deque::clear();
	StrafeDraw.deque::clear();
	FOVDraw.deque::clear();
	CrosshairDraw.deque::clear();
	PlayerHitboxNum.deque::clear();
}

void HookImGui(HDC hdc)
{
	//Credit to my kind friend BloodSharp for helping a noob <3
	ColorChange();
	ClearSound();
	if (CheckDraw())
	{
		InistalizeImgui(hdc);
		if (!bOldOpenGL)
			ImGui_ImplOpenGL3_NewFrame();
		else
			ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		DrawFullScreenWindow(); 
		DrawOverview();
		DrawKzWindows();
		DrawMenuWindow();

		ImGui::Render();
		if (!bOldOpenGL)
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		else
			ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
	}
	MenuHandle();
	ClearHudKeys();
	ClearDeque();
}

bool CheckDrawEngine()
{
	if (DrawVisuals && GetTickCount() - HudRedraw <= 100)
		return true;
	return false;
}

bool CheckDraw()
{
	if (GetTickCount() - HudRedraw <= 100)
		return true;
	return false;
}