#include "main.h"

bool hovered;

#define INJECTOR			    1
#define FACEBOOK			    2
#define CHECKMARK			    3
#define WRONG			        4
#define MENUBACK			    5
#define MENUBACK0			    5
#define MENUBACK1			    6
#define MENUBACK2			    7
#define MENUBACK3			    8
#define MENUBACK4			    9
#define MENUBACK5			    10
#define MENUBACK6			    11
#define MENUBACK7			    12
#define MENUBACK8			    13
#define MENUBACK9			    14
#define MENUBACK10			    15
#define MENUBACK11			    16
#define MENUBACK12			    17
#define MENUBACK13			    18
#define MENUBACK14			    19

GLuint texture_id[1024];
GLFWimage icons;

void MenuImage()
{
	static DWORD Tickcount;
	static int texture = MENUBACK0;
	if (GetTickCount() - Tickcount >= 0.03 * 1000)
	{
		if (texture < MENUBACK14)
			texture++;
		else
			texture = MENUBACK0;
		Tickcount = GetTickCount();
	}
	ImGui::GetWindowDrawList()->AddImage((GLuint*)texture_id[texture], ImVec2(ImGui::GetCurrentWindow()->Pos.x + 6, ImGui::GetCurrentWindow()->Pos.y + 27), ImVec2(ImGui::GetCurrentWindow()->Pos.x + ImGui::GetCurrentWindow()->Size.x - 6, ImGui::GetCurrentWindow()->Pos.y + ImGui::GetCurrentWindow()->Size.y - 8));
}

void LoadTextureMemory(const unsigned char* image, int index, int size)
{
	int width, height;

	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGenTextures(1, &texture_id[index]);
	glBindTexture(GL_TEXTURE_2D, texture_id[index]);
	unsigned char* soilimage = SOIL_load_image_from_memory(image, size, &width, &height, 0, SOIL_LOAD_RGBA);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, soilimage);
	SOIL_free_image_data(soilimage);
	glBindTexture(GL_TEXTURE_2D, last_texture);
}

WNDPROC hWndProc;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HOOK_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;
	return CallWindowProc(hWndProc, hWnd, uMsg, wParam, lParam);
}

void Menu()
{
	hovered = false; 
	static bool bShowMenu = true;
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(312, 277));
	ImGui::Begin("##menu", &bShowMenu, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
	{
		MenuImage();
		static int sucess = 0;
		static int fail = 0;
		if (sucess)sucess--;
		if (fail)fail--;
		GLuint texture = texture_id[INJECTOR];
		if (sucess) texture = texture_id[CHECKMARK];
		if (fail) texture = texture_id[WRONG];
		ImVec2 LastSapcing = ImGui::GetStyle().ItemSpacing;
		ImGui::GetStyle().ItemSpacing.x = 0;
		if (ImGui::ImageButton((GLuint*)texture, ImVec2(142, 144)))
		{
			bool foundfile = true;
			bool foundprocess = true;
			char moduleName[32] = "hack.dll";
			char modulePath[MAX_PATH];
			GetModuleFileName(NULL, modulePath, MAX_PATH);
			char* index = strrchr(modulePath, '\\');
			modulePath[index - modulePath + 1] = 0;
			strcat_s(modulePath, MAX_PATH, moduleName);
			WIN32_FIND_DATA WFD;
			if (FindFirstFile(modulePath, &WFD) == INVALID_HANDLE_VALUE)
			{
				char msg[512];
				strcpy_s(msg, 512, "File ");
				strcat_s(msg, 512, moduleName);
				strcat_s(msg, 512, " was not found!");
				MessageBox(NULL, msg, "Error", MB_OK | MB_ICONERROR);
				foundfile = false;
			}
			PROCESS_INFORMATION processInfo;
			if (Injector::GetProcessInfo("cstrike.exe", &processInfo) == false && Injector::GetProcessInfo("hl.exe", &processInfo) == false)
			{
				MessageBox(NULL, "Process hl.exe or cstrike.exe is not running!", "Error", MB_OK | MB_ICONERROR);
				foundprocess = false;
			}
			if ((InjectLibrary("cstrike.exe", modulePath) || InjectLibrary("hl.exe", modulePath)) && foundfile && foundprocess)
				sucess = 25;
			else
				fail = 5;
		}
		if (ImGui::IsItemHovered())
		{
			hovered = true;
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted("Inject DLL To Process");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		ImGui::SameLine();
		if (ImGui::ImageButton((GLuint*)texture_id[FACEBOOK], ImVec2(142, 144)))
		{
			ShellExecute(NULL, "open", "https://www.facebook.com/groups/1628540544084700/", NULL, NULL, SW_HIDE);
		}
		if (ImGui::IsItemHovered())
		{
			hovered = true;
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted("Visit Facebook Group");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		if (ImGui::ArrowButtonDouble("##left", ImGuiDir_Left, 90))
			backwards = true;
		if (ImGui::IsItemHovered())
		{
			hovered = true;
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted("Radio Channel Back");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		ImGui::SameLine();
		if (gui_radio)
		{
			if (ImGui::SquareButton("##square", 120))
				gui_radio = !gui_radio;
		}
		else
		{
			if (ImGui::ArrowButton("##right", ImGuiDir_Right, 120))
				gui_radio = !gui_radio;
		}

		if (ImGui::IsItemHovered())
		{
			hovered = true;
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted("Radio Channel Play/Pause");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		ImGui::SameLine();
		if (ImGui::ArrowButtonDouble("##right", ImGuiDir_Right, 90))
			forwards = true;
		if (ImGui::IsItemHovered())
		{
			hovered = true;
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted("Radio Channel Forward");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		ImGui::GetStyle().ItemSpacing = LastSapcing;
		ImGui::PushItemWidth(-1);
		const char* listbox_radio[] = { "PulseEDM Dance Music", "Hard Style", "Big Fm", "Big Fm Deutsch Rap", "Radio Record", "Record Dubstep", "Record Hardstyle", "Record Dancecore", "Anison FM", "Nrk MP3" };
		if (ImGui::Combo("Radio Channel", &gui_radio_channel, listbox_radio, IM_ARRAYSIZE(listbox_radio), 10))channelchange = true;
		if (ImGui::IsItemHovered())
		{
			hovered = true;
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted("Select Radio Channel");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		ImGui::SliderFloat("Radio Volume", &gui_radio_volume, 0.f, 100.f, "Radio Volume: %.f");
		if (ImGui::IsItemHovered())
		{
			hovered = true;
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted("Adjust Radio Volume");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		if (ImGui::InputText("Input Text", zstationstype, IM_ARRAYSIZE(zstationstype), ImGuiInputTextFlags_EnterReturnsTrue))channeltype = true;
		if (ImGui::IsItemHovered())
		{
			hovered = true;
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted("Input Link To Radio Stream");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		ImGui::PopItemWidth();
	}
	ImGui::End();
	if (!bShowMenu)
		exit(0);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HANDLE handle = CreateMutex(NULL, TRUE, "Dialog553453534");
	if (GetLastError() != ERROR_SUCCESS)
	{
		MessageBox(0, "Injector is already running!", "Information", MB_OK | MB_ICONINFORMATION);
		return 0;
	}
	InitRadio();
	int screenW = GetSystemMetrics(SM_CXSCREEN);
	int screenH = GetSystemMetrics(SM_CYSCREEN);
	int screensizex = 296, screensizey = 239;
	int centrx = (screenW / 2) - (screensizex / 2);
	int centry = (screenH / 2) - (screensizey / 2);
	if (!glfwInit())
		return 1;
	GLFWwindow* window = glfwCreateWindow(screensizex, screensizey, "loader", NULL, NULL);
	if (window == NULL)
		return 1; 
	icons.pixels = SOIL_load_image_from_memory(injector, 26315, &icons.width, &icons.height, 0, SOIL_LOAD_RGBA);
	glfwSetWindowIcon(window, 1, &icons);
	SOIL_free_image_data(icons.pixels);
	glfwSetWindowPos(window, centrx, centry);
	HWND hWnd = glfwGetWin32Window(window);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	LONG style = GetWindowLong(hWnd, GWL_STYLE);
	style &= ~(WS_CAPTION | WS_THICKFRAME );
	SetWindowLong(hWnd, GWL_STYLE, style);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	hWndProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (LONG)HOOK_WndProc);
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplOpenGL2_Init();
	ImGui::StyleColorsDark();
	ImGui::GetStyle().AntiAliasedFill = false;
	ImGui::GetStyle().AntiAliasedLines = false;
	ImGui::GetStyle().FrameRounding = 0.0f;
	ImGui::GetStyle().WindowRounding = 0.0f;
	ImGui::GetStyle().ChildRounding = 0.0f;
	ImGui::GetStyle().ScrollbarRounding = 0.0f;
	ImGui::GetStyle().GrabRounding = 0.0f;
	ImGui::GetStyle().FrameBorderSize = 1.0f;
	ImGui::GetStyle().WindowPadding = ImVec2(6, 8);
	ImGui::GetStyle().WindowTitleAlign = ImVec2(0.5f, 0.5f);
	ImGui::GetStyle().DisplaySafeAreaPadding = ImVec2(0.0f, 0.0f);
	ImGui::GetStyle().WindowBorderSize = 0.0f;
	ImGui::GetStyle().FrameBorderSize = 0.0f;
	ImGui::GetStyle().PopupBorderSize = 0.0f;
	ImGui::GetIO().IniFilename = NULL;
	ImGui::GetIO().LogFilename = NULL;

	while (!glfwWindowShouldClose(window))
	{
		static bool loadtexture = true;
		if (loadtexture)
		{
			LoadTextureMemory(injector, INJECTOR, ARRAYSIZE(injector));
			LoadTextureMemory(facebook, FACEBOOK, ARRAYSIZE(facebook));
			LoadTextureMemory(checkmark, CHECKMARK, ARRAYSIZE(checkmark));
			LoadTextureMemory(wrong, WRONG, ARRAYSIZE(wrong));
			LoadTextureMemory(menuback0, MENUBACK0, ARRAYSIZE(menuback0));
			LoadTextureMemory(menuback1, MENUBACK1, ARRAYSIZE(menuback1));
			LoadTextureMemory(menuback2, MENUBACK2, ARRAYSIZE(menuback2));
			LoadTextureMemory(menuback3, MENUBACK3, ARRAYSIZE(menuback3));
			LoadTextureMemory(menuback4, MENUBACK4, ARRAYSIZE(menuback4));
			LoadTextureMemory(menuback5, MENUBACK5, ARRAYSIZE(menuback5));
			LoadTextureMemory(menuback6, MENUBACK6, ARRAYSIZE(menuback6));
			LoadTextureMemory(menuback7, MENUBACK7, ARRAYSIZE(menuback7));
			LoadTextureMemory(menuback8, MENUBACK8, ARRAYSIZE(menuback8));
			LoadTextureMemory(menuback9, MENUBACK9, ARRAYSIZE(menuback9));
			LoadTextureMemory(menuback10, MENUBACK10, ARRAYSIZE(menuback10));
			LoadTextureMemory(menuback11, MENUBACK11, ARRAYSIZE(menuback11));
			LoadTextureMemory(menuback12, MENUBACK12, ARRAYSIZE(menuback12));
			LoadTextureMemory(menuback13, MENUBACK13, ARRAYSIZE(menuback13));
			LoadTextureMemory(menuback14, MENUBACK14, ARRAYSIZE(menuback14));
			loadtexture = false;
		}
		glfwPollEvents();
		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		PlayRadio();
		Menu();
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImVec4 clear_color = ImVec4(1.f, 1.f, 1.f, 1.f);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
		glfwMakeContextCurrent(window);
		glfwSwapBuffers(window);
	}
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}