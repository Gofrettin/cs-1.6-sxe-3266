#include "client.h"

pfnUserMsgHook TeamInfoOrg = NULL;
pfnUserMsgHook ScoreAttribOrg = NULL;
pfnUserMsgHook SetFOVOrg = NULL;
pfnUserMsgHook ResetHUDOrg = NULL;
pfnUserMsgHook DeathMsgOrg = NULL;
pfnUserMsgHook HealthOrg = NULL;
pfnUserMsgHook ServerNameOrg = NULL;
DWORD msgOrg = 0;

int ServerName(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	char* m_szServerName = READ_STRING();
	sprintf(sServerName, "%s", m_szServerName);
	return (*ServerNameOrg)(pszName, iSize, pbuf);
}

int ScoreAttrib(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int id = READ_BYTE();
	int info = READ_BYTE();
	g_Player[id].bVip = (info & (1 << 2));
	g_Player[id].bAliveInScoreTab = !(info & (1 << 0));
	return (*ScoreAttribOrg)(pszName, iSize, pbuf);
}

int SetFOV(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int iFOV = READ_BYTE();
	g_Local.iFOV = iFOV;
	return (*SetFOVOrg)(pszName, iSize, pbuf);
}

int ResetHUD(const char* pszName, int iSize, void* pbuf)
{
	static char currentMap[100];
	if (strcmp(currentMap, g_Engine.pfnGetLevelName()))
	{
		strcpy(currentMap, g_Engine.pfnGetLevelName());
		LoadOverview((char*)getfilename(g_Engine.pfnGetLevelName()).c_str());
	}
	RunHLCommands();
	ContinueRoute();
	ResetSpawn();
	Sound_No_Index.deque::clear();
	Sound_Index.deque::clear();
	for (unsigned int i = 0; i < 33; i++)
		g_Player[i].iHealth = 100;
	return (*ResetHUDOrg)(pszName, iSize, pbuf);
}

int DeathMsg(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int killer = READ_BYTE();
	int victim = READ_BYTE();
	int headshot = READ_BYTE();
	if (killer != victim && killer == g_Local.iPlayer_Index + 1 && victim > 0 && victim <= g_Engine.GetMaxClients())
		dwReactionTime = GetTickCount();

	if (victim != g_Local.iPlayer_Index + 1 && victim > 0 && victim <= g_Engine.GetMaxClients())
		g_Player[victim].iHealth = 100;

	cl_entity_s* ent = GetEntityByIndex(victim);

	hud_player_info_t entinfo;
	pfnGetPlayerInfo(victim, &entinfo);
	if (entinfo.name && ent && victim > 0 && victim <= g_Engine.GetMaxClients() && cvar.visual_spawn_scan)
	{
		spawndeath_t Spawns;
		Spawns.index = ent->index;
		Spawns.Origin = ent->origin;
		Spawns.Tickcount = GetTickCount();
		strcpy(Spawns.name, entinfo.name);
		SpawnDeath.push_back(Spawns);
	}
	KillSound(victim, killer, headshot);
	return (*DeathMsgOrg)(pszName, iSize, pbuf);
}

int TeamInfo(const char* pszName, int iSize, void* pbuf) 
{
	BEGIN_READ(pbuf, iSize);
	int id = READ_BYTE();
	char* szTeam = READ_STRING();
	if (id > 0 && id <= g_Engine.GetMaxClients())
	{
		if (!lstrcmpA(szTeam, "TERRORIST"))
		{
			g_Player[id].iTeam = 1;
			if (id == g_Local.iPlayer_Index + 1) { g_Local.iTeam = 1; }
		}
		else if (!lstrcmpA(szTeam, "CT"))
		{
			g_Player[id].iTeam = 2;
			if (id == g_Local.iPlayer_Index + 1) { g_Local.iTeam = 2; }
		}
		else
		{
			g_Player[id].iTeam = 0;
			if (id == g_Local.iPlayer_Index + 1) { g_Local.iTeam = 0; }
		}
	}
	return (*TeamInfoOrg)(pszName, iSize, pbuf);
}

int Health(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	return (*HealthOrg)(pszName, iSize, pbuf);
}

DWORD GetUserMsgHook(PCHAR name) 
{
	PBYTE address = (PBYTE)msgOrg;
	PUserMgsList uml = *(PUserMgsList*)(*(PDWORD)(((address += 0x1A) + *(PDWORD)(address + 1) + 5) + 0x0D));

	int i = 0;
	while (uml) 
	{
		if (i == 200) 
			break;
		if (!strncmp(uml->msgname, name, 16))
			return uml->address;
		uml = uml->next;
		i++;
	}
	return 0;
}

void InitMsg()
{
	msgOrg = (DWORD)g_Engine.pfnHookUserMsg;

	static bool bApplied = false;
	if (bApplied == false)
	{
		ServerNameOrg = (pfnUserMsgHook)GetUserMsgHook("ServerName\0");
		if (ServerNameOrg)g_Engine.pfnHookUserMsg("ServerName\0", ServerName);
		TeamInfoOrg = (pfnUserMsgHook)GetUserMsgHook("TeamInfo\0");
		if (TeamInfoOrg)g_Engine.pfnHookUserMsg("TeamInfo\0", TeamInfo);
		ScoreAttribOrg = (pfnUserMsgHook)GetUserMsgHook("ScoreAttrib\0");
		if (ScoreAttribOrg)g_Engine.pfnHookUserMsg("ScoreAttrib\0", ScoreAttrib);
		SetFOVOrg = (pfnUserMsgHook)GetUserMsgHook("SetFOV\0");
		if (SetFOVOrg)g_Engine.pfnHookUserMsg("SetFOV\0", SetFOV);
		ResetHUDOrg = (pfnUserMsgHook)GetUserMsgHook("ResetHUD\0");
		if (ResetHUDOrg)g_Engine.pfnHookUserMsg("ResetHUD\0", ResetHUD);
		DeathMsgOrg = (pfnUserMsgHook)GetUserMsgHook("DeathMsg\0");
		if (DeathMsgOrg)g_Engine.pfnHookUserMsg("DeathMsg\0", DeathMsg);
		HealthOrg = (pfnUserMsgHook)GetUserMsgHook("Health\0");
		if (HealthOrg)g_Engine.pfnHookUserMsg("Health\0", Health);

		bApplied = true;
	}
}