#include "client.h"

cl_clientfunc_t* g_pClient;
cl_enginefuncs_s* g_pEngine;
engine_studio_api_s* g_pStudio;
StudioModelRenderer_t* g_pStudioModelRenderer;
screenfade_t* pScreenFade;

cl_clientfunc_t g_Client;
cl_enginefuncs_s g_Engine;
engine_studio_api_t g_Studio;
StudioModelRenderer_t g_StudioModelRenderer;

DWORD HudRedraw;
DWORD SpeedPtr;
DWORD dwSpeedptr = 0;
PColor24 Console_TextColor;

void HUD_Redraw(float x, int y)
{
	HudRedraw = GetTickCount();

	DrawOverviewLayer();
	KzFameCount();
	return;
}

int HUD_Key_Event(int eventcode, int keynum, const char* pszCurrentBinding)
{
	KeyEventResult = 1;
	//preset keys bind
	bool keyrush = cvar.rage_active && keynum == cvar.route_rush_key;
	bool keyquick = cvar.misc_quick_change && keynum == cvar.misc_quick_change_key;
	bool keystrafe = cvar.kz_strafe && keynum == cvar.kz_strafe_key;
	bool keyfast = cvar.kz_fast_run && keynum == cvar.kz_fastrun_key;
	bool keygstrafe = cvar.kz_ground_strafe && keynum == cvar.kz_ground_strafe_key;
	bool keybhop = cvar.kz_bhop && keynum == cvar.kz_bhop_key;
	bool keyjump = cvar.kz_jump_bug && keynum == cvar.kz_jumpbug_key;
	bool keyrage = cvar.rage_active && !cvar.rage_auto_fire && !cvar.rage_always_fire && keynum == cvar.rage_auto_fire_key;
	bool keylegit = !cvar.rage_active && cvar.legit[g_Local.weapon.m_iWeaponID].active && IsCurWeaponGun() && keynum == cvar.legit_key;
	bool keylegittrigger = !cvar.rage_active && cvar.legit[g_Local.weapon.m_iWeaponID].trigger_active && !cvar.legit[g_Local.weapon.m_iWeaponID].active && IsCurWeaponGun() && keynum == cvar.legit_trigger_key;
	bool keychat = cvar.gui_chat && keynum == cvar.gui_chat_key;
	bool keychatteam = cvar.gui_chat && keynum == cvar.gui_chat_key_team;
	bool keymenu = keynum == cvar.gui_key;

	//send keys to menu
	keysmenu[keynum] = eventcode;

	//send chat message
	if (bInputActive && keynum == K_ENTER && eventcode)
	{
		char SayText[255];
		if (iInputMode == 1)sprintf(SayText, "say \"%s\"", InputBuf);
		if (iInputMode == 2)sprintf(SayText, "say_team \"%s\"", InputBuf);
		g_Engine.pfnClientCmd(SayText);
		bInputActive = false;
		for (unsigned int i = 0; i < 256; i++) ImGui::GetIO().KeysDown[i] = false;
		KeyEventResult = 0;
		return 0;
	}
	//return game bind if chat active
	if (bInputActive && GetTickCount() - HudRedraw <= 100)
	{
		KeyEventResult = 0;
		return 0;
	}

	//return game bind for chat bind
	if (keychat && eventcode)
	{
		bInputActive = true, iInputMode = 1;
		KeyEventResult = 0;
		return 0;
	}
	if (keychatteam && eventcode)
	{
		bInputActive = true, iInputMode = 2;
		KeyEventResult = 0;
		return 0;
	}

	//return game bind for menu bind
	if (keymenu && eventcode)
	{
		bShowMenu = !bShowMenu;
		if (!bShowMenu)SaveCvar();
		KeyEventResult = 0;
		return 0;
	}

	//return game bind if menu active
	if (bShowMenu && GetTickCount() - HudRedraw <= 100)
	{
		KeyEventResult = 0;
		return 0;
	}

	//check if alive
	if (bAliveLocal())
	{
		//do function bind
		if (keyrush)
		{
			if (eventcode)
			{
				if (cvar.route_activate) cvar.route_auto = 1;
				cvar.misc_wav_speed = 64;
			}
			else
			{
				if (cvar.route_activate) cvar.route_auto = 0;
				cvar.misc_wav_speed = 1;
			}
		}
		if (keyquick && eventcode)
		{
			cvar.rage_active = !cvar.rage_active, ModeChangeDelay = GetTickCount(), SaveCvar();
			if (!cvar.rage_active)cvar.route_auto = 0, cvar.misc_wav_speed = 1, RageKeyStatus = false;
		}
		if (keystrafe)
			Strafe = eventcode;
		if (keyfast)
			Fastrun = eventcode;
		if (keygstrafe)
			Gstrafe = eventcode;
		if (keybhop)
			Bhop = eventcode;
		if (keyjump)
			Jumpbug = eventcode;
		if (keyrage)
			RageKeyStatus = eventcode;
		if (keylegit)
			LegitKeyStatus = eventcode;
		if (keylegittrigger && eventcode)
			TriggerKeyStatus = !TriggerKeyStatus;

		//return game bind for function bind
		if ((keystrafe || keyfast || keygstrafe || keybhop || keyjump || keyrage || keylegittrigger || keylegit || keyquick || keyrush) && eventcode)
		{
			KeyEventResult = 0;
			return 0;
		}
	}
	return KeyEventResult;
}

void AntiAfk(usercmd_s* cmd)
{
	int afktime = cvar.afk_time;
	afktime -= 1;
	afktime *= 1000;
	static DWORD antiafk = GetTickCount();
	static Vector prevorigin;
	static Vector prevangles;
	if (bAliveLocal())
	{
		if (g_Local.vOrigin != prevorigin || cmd->viewangles != prevangles)
			antiafk = GetTickCount();
		prevorigin = g_Local.vOrigin;
		prevangles = cmd->viewangles;
		if (cvar.afk_anti)
		{
			if (GetTickCount() - antiafk > afktime)
			{
				cmd->buttons |= IN_JUMP;
				cmd->viewangles[1] += 5;
				g_Engine.SetViewAngles(cmd->viewangles);
			}

		}
	}
	else
		antiafk = GetTickCount();
}

void CL_CreateMove(float frametime, struct usercmd_s* cmd, int active)
{
	AdjustSpeed(cvar.misc_wav_speed);
	UpdateWeaponData();
	AimBot(cmd);
	ContinueFire(cmd);
	ItemPostFrame(cmd);
	Kz(frametime, cmd);
	NoRecoil(cmd);
	NoSpread(cmd);
	Route(cmd);
	AntiAim(cmd);
	CustomFOV();
	WallRun();
	CrossHair();
	AntiAfk(cmd);
	return;
}

void HUD_PostRunCmd(struct local_state_s* from, struct local_state_s* to, struct usercmd_s* cmd, int runfuncs, double time, unsigned int random_seed)
{
	ItemPreFrame(from, to, cmd, runfuncs, time, random_seed);
	return;
}

void PreV_CalcRefdef(struct ref_params_s* pparams)
{
	g_Local.vPunchangle = pparams->punchangle;
	g_Local.vPrevForward = pparams->forward;
	g_Local.iPrevHealth = pparams->health;
	V_CalcRefdefRecoil(pparams);
	return;
}

void PostV_CalcRefdef(struct ref_params_s* pparams)
{
	TraceGrenade(pparams);
	if (DrawVisuals && (!cvar.route_auto || cvar.route_draw_visual) && GetTickCount() - HudRedraw <= 100)
	{
		cl_entity_s* ent = g_Studio.GetCurrentEntity();
		bool ViewModel = ent && ent->model && ent->model->name && strstr(ent->model->name, "v_");
		if (ViewModel)
		{
			Vector forward = pparams->forward;
			ent->origin += forward * cvar.visual_viewmodel_fov;
		}
	}
	g_Local.vPostForward = pparams->forward;
	g_Local.iPostHealth = pparams->health;
	return;
}

void AdjustSpeed(double x)
{
	if (dwSpeedptr == 0)
	{
		dwSpeedptr = SpeedPtr;
	}
	static double LastSpeed = 1;
	if (x != LastSpeed)
	{
		*(double*)dwSpeedptr = (x * 1000);
		LastSpeed = x;
	}
}

void HUD_Frame(double time)
{
	Snapshot();
	ThirdPerson();
	FindSpawn();
	LoadTextureWall();
	Sky();
	NoFlash();
	Lightmap();
	PlayerAim.deque::clear();
	return;
}

void HUD_PlayerMove(struct playermove_s* ppmove, qboolean server)
{
	ONCE_ONLY(PM_InitTextureTypes(ppmove));
	
	g_Local.vEye = ppmove->origin + ppmove->view_ofs;
	g_Local.vVelocity = ppmove->velocity;
	g_Local.flFallVelocity = ppmove->flFallVelocity;
	g_Local.iPlayer_Index = ppmove->player_index;
	g_Local.vView_Ofs = ppmove->view_ofs;
	g_Local.iMovetype = ppmove->movetype;
	g_Local.iFlags = ppmove->flags;
	g_Local.vAngles = ppmove->angles;
	g_Local.vOrigin = ppmove->origin;
	g_Local.iUseHull = ppmove->usehull;
	g_Local.flFrametime = ppmove->frametime;
	g_Local.flMoveGravity = ppmove->movevars->gravity;

	return;
}

void HUD_AddEntity(int type, struct cl_entity_s* ent, const char* modelname)
{
	AddEntResult = 1;
	cl_entity_s* entlocal = GetLocalPlayer();
	if (ent && entlocal && entlocal->curstate.iuser1 == 4 && entlocal->curstate.iuser2 == ent->index)
		AddEntResult = 0;
	return;
}