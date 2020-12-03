#ifndef _SOUNDHOOK_H_
#define _SOUNDHOOK_H_

typedef struct
{
	int index;
	Vector origin;
	DWORD timestamp;
} player_sound_index_t;
extern deque<player_sound_index_t> Sound_Index;

typedef struct
{
	Vector origin;
	DWORD timestamp;
} player_sound_no_index_t;
extern deque<player_sound_no_index_t> Sound_No_Index;

struct svc_entry_s
{
	DWORD dwId;
	char  *szName;
	DWORD pfnHandler;
};

typedef void (*sound_t)(int, int, char*, float*, float, float, int, int);
void InitSound();

#endif