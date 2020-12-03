#include "client.h"

deque<player_sound_no_index_t> Sound_No_Index;
deque<player_sound_index_t> Sound_Index;

svc_entry_s* p_SvcDispatch;
sound_t p_Sound;

void S_StartDynamicSound( int entid, DWORD entchannel, char* szSoundFile, float* fOrigin, float volume, float attenuation, int timeofs , int pitch )
{
	if (szSoundFile && fOrigin)
	{
		if (!strstr(szSoundFile, "pl_shell") && !strstr(szSoundFile, "ric") && !strstr(szSoundFile, "die") && !strstr(szSoundFile, "glass") && !strstr(szSoundFile, "debris") && !strstr(szSoundFile, "death") && strstr(szSoundFile, "player"))
		{
			if (entid > 0 && entid < 33)
			{
				if (strstr(szSoundFile, "bhit_helmet"))
					g_Player[entid].iHealth -= 80;
				else if (strstr(szSoundFile, "bhit_kevlar"))
					g_Player[entid].iHealth -= 20;
				else if (strstr(szSoundFile, "bhit_flesh"))
					g_Player[entid].iHealth -= 30;
				else if (strstr(szSoundFile, "headshot"))
					g_Player[entid].iHealth -= 80;
				else if (strstr(szSoundFile, "die") || strstr(szSoundFile, "death"))
					g_Player[entid].iHealth = 100;
				if (cvar.visual_sound_index)
				{
					player_sound_index_t sound_index;
					sound_index.index = entid;
					sound_index.origin = fOrigin;
					sound_index.timestamp = GetTickCount();
					Sound_Index.push_back(sound_index);
				}
			}
			else if (cvar.visual_sound_no_index)
			{
				player_sound_no_index_t sound_no_index;
				sound_no_index.origin = fOrigin;
				sound_no_index.timestamp = GetTickCount();
				Sound_No_Index.push_back(sound_no_index);
			}
		}
	}
	(*p_Sound)(entid,entchannel, szSoundFile, fOrigin,volume,attenuation,timeofs,pitch);
}

DWORD dwHook_SvcEntry(DWORD dwBegin, DWORD dwEnd, DWORD dwCall, DWORD dwHandler, int iPos)
{
	DWORD dwMy_Handler = pMemoryTools->dwCopyRange(dwBegin, dwEnd);
	p_SvcDispatch[iPos].pfnHandler = (DWORD)dwMy_Handler;
	return pMemoryTools->dwRedirectCall((dwMy_Handler + dwCall), dwHandler);
}

void InitSound()
{
	static DWORD g_dwCL_ParseSound_Begin = NULL;
	static DWORD g_dwCL_ParseSound_Call = NULL;
	static DWORD g_dwCL_ParseSound_End = NULL;
	static DWORD g_dwSvc_Table = NULL;

	g_dwCL_ParseSound_Begin = pMemoryTools->dwFindPattern(0x01D0D000, 0x000645a4,(BYTE*)"\x83\xEC\x68\x53\x55\x56\x57\x68\x00\x00\x00\x00\xE8","xxxxxxxx????x");
	g_dwCL_ParseSound_Call  = pMemoryTools->dwFindPattern(0x01D0D000, 0x000645a4,(BYTE*)"\x50\x57\xE8\x00\x00\x00\x00\x83\xC4\x20\x5F\x5E\x5D\x5B\x83\xC4\x68\xC3","xxx????xxxxxxxxxxx");
	g_dwCL_ParseSound_End	= pMemoryTools->dwFindPattern(0x01D0D000, 0x000645a4,(BYTE*)"\x50\x57\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xC4\x0C\x5F\x5E\x5D\x5B\x83\xC4\x68\xC3","xxx????x????xxxxxxxxxxx");
	g_dwSvc_Table			= pMemoryTools->dwFindPattern(0x01D0D000, 0x000645a4,(BYTE*)"\x51\x53\x56\x57\x68\xFF\xFF\xFF\xFF\x33\xDB\xE8\xFF\xFF\xFF\xFF\x83\xC4\x04\x33\xF6\xBF","xxxxx????xxx????xxxxxx");
	g_dwSvc_Table			 += 0x15;
	g_dwCL_ParseSound_End	 += 0x16;
	g_dwCL_ParseSound_Call	 += 2;
	g_dwCL_ParseSound_Call	 -= g_dwCL_ParseSound_Begin;
	p_SvcDispatch			  = (svc_entry_s*)(*(DWORD*)(g_dwSvc_Table + 1 ) - 4 );

	p_Sound = (sound_t)dwHook_SvcEntry(g_dwCL_ParseSound_Begin , g_dwCL_ParseSound_End , g_dwCL_ParseSound_Call , (DWORD)S_StartDynamicSound, 6);
}
