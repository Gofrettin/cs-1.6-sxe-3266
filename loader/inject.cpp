/////////////////////////////////////////////////////////////////////////////////////
// File        : InjectLibrary.cpp
// Author      : liuzewei
// Date        : 2007.1.17
// Description : Kernel32 function got dynamically, because not all of these 
//               functions are available on all OS
/////////////////////////////////////////////////////////////////////////////////////

#include "inject.h"
#include <stddef.h>
#include <tlhelp32.h>

#pragma pack( 1 ) // Make injectcode are consecutive
struct InjectCode
{
    BYTE  PushOpc;
    DWORD PushAdd;
    BYTE  CallOpc;
    DWORD CallAdd;
    WORD  Jmp_$;
    char  LibraryPath[MAX_PATH];
};
#pragma pack()

HANDLE  ( WINAPI *pCreateToolhelp32Snapshot )( DWORD  dwFlags, DWORD th32ProcessID );
BOOL    ( WINAPI *pProcess32First )          ( HANDLE hSnapshot, LPPROCESSENTRY32 lppe ); 
BOOL    ( WINAPI *pProcess32Next )           ( HANDLE hSnapshot, LPPROCESSENTRY32 lppe );
BOOL    ( WINAPI *pModule32First )           ( HANDLE hSnapshot, LPMODULEENTRY32 lpme ); 
BOOL    ( WINAPI *pModule32Next )            ( HANDLE hSnapshot, LPMODULEENTRY32 lpme );
BOOL    ( WINAPI *pThread32First )           ( HANDLE hSnapshot, LPTHREADENTRY32 lpte );
BOOL    ( WINAPI *pThread32Next )            ( HANDLE hSnapshot, LPTHREADENTRY32 lpte );
LPVOID  ( WINAPI *pVirtualAllocEx )          ( HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD  flAllocationType, DWORD  flProtect );
BOOL    ( WINAPI *pVirtualFreeEx )           ( HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD  dwFreeType );
HANDLE  ( WINAPI *pOpenProcess )             ( DWORD dwDesiredAccess, BOOL  bInheritHandle, DWORD dwProcessId );
HANDLE  ( WINAPI *pOpenThread )              ( DWORD dwDesiredAccess, BOOL  bInheritHandle, DWORD dwThreadId );

// From patrick's vec proof base for cs1.6
DWORD SearchPattern( DWORD start, DWORD length, BYTE *pattern, CHAR *mask )
{
    BYTE *currentAddress;
    BYTE *currentPattern;
    CHAR *currentMask;
    for ( DWORD i=0; i<length; i++ )
    {
        currentAddress = (BYTE*)(start+i);
        currentPattern = pattern;
        currentMask    = mask;
        for ( ; *currentMask; currentAddress++,currentPattern++,currentMask++ )
        {
            if ( *currentMask=='x' && *currentAddress!=*currentPattern )
                break;
        }
        if ( *currentMask == NULL ) return ( start + i );
    }
    
    return NULL;
}

LPVOID WINAPI VirtualsAllocEx9x( HANDLE hProcess, LPVOID lpAddress, DWORD dwSize, DWORD flAllocationType, DWORD flProtect )
{
    LPVOID ( WINAPI *pVirtualAlloc )( LPVOID, SIZE_T, DWORD, DWORD );
    *(PDWORD)(&pVirtualAlloc) = (DWORD)GetProcAddress( LoadLibrary( TEXT( "kernel32.dll" ) ), "VirtualAlloc" );
    
    return pVirtualAlloc( lpAddress, dwSize, flAllocationType|0x8000000, flProtect );          
}

BOOL WINAPI VirtualsFreeEx9x( HANDLE hProcess, LPVOID lpAddress, DWORD dwSize, DWORD dwFreeType )
{
    BOOL ( WINAPI *pVirtualFree )( LPVOID, SIZE_T, DWORD );
    *(PDWORD)(&pVirtualFree) = (DWORD)GetProcAddress( LoadLibrary( TEXT( "kernel32.dll" ) ), "VirtualFree" );
    
    return pVirtualFree( lpAddress, dwSize,dwFreeType );
}

HANDLE WINAPI OpensThread9x( DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwThreadId )
{
    // Description : 
    // 1> what System API OpenProcess() do?
    //    1st, check target is really a process
    //    2nd, call undocument system function GetHandle()
    // 2> what our OpenThread9x do?
    //    Just get thread's TDB first, then call GetHandle()
    DWORD  processID, obsfucator, *pThreadDataBase;
    HANDLE hThread;
    HANDLE ( WINAPI *pInternalOpenProcess )( DWORD, BOOL, DWORD );
    
    processID = GetCurrentProcessId();
    __asm mov eax,fs:[0x30];
    __asm xor eax,processID;
    __asm mov obsfucator,eax;
    
    pThreadDataBase = ( DWORD* ) ( dwThreadId ^ obsfucator );
    if ( IsBadReadPtr( pThreadDataBase, sizeof(DWORD) ) || ( ( *pThreadDataBase & 0x7 ) != 0x7 ) )
        return NULL;
    
    *(PDWORD)(&pInternalOpenProcess) = SearchPattern( (DWORD)pOpenProcess, 0xFF, (BYTE*)"\xB9\x00\x00\x00\x00", "xxxxx" );
    if ( pInternalOpenProcess == NULL )
        return NULL;
    
    __asm mov   eax, pThreadDataBase;
    __asm push  dwThreadId;
    __asm push  bInheritHandle;
    __asm push  dwDesiredAccess;
    __asm call  pInternalOpenProcess;
    __asm mov   hThread, eax;
      
    return hThread;
}

BOOL GetKernel32Function( VOID )
{
    static BOOL GOT = FALSE;
    if( GOT ) return TRUE;
    
    // get OS Version
    BOOL OSWin98, OSWinMe, OSWinXP;
    OSVERSIONINFO OSVI;
    OSVI.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
    if ( !GetVersionEx( &OSVI ) ) return FALSE;
    OSWin98 = ( OSVI.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS && OSVI.dwMajorVersion==4 && OSVI.dwMinorVersion<=10 ) ? TRUE : FALSE;
    OSWinMe = ( OSVI.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS && OSVI.dwMajorVersion==4 && OSVI.dwMinorVersion>=90 ) ? TRUE : FALSE;
    OSWinXP = ( OSVI.dwPlatformId==VER_PLATFORM_WIN32_NT ) ? TRUE : FALSE;
    
    // get kernel32 function pointers dynamically
    HINSTANCE hKernel32 = LoadLibrary( TEXT( "kernel32.dll" ) );
    if ( hKernel32 == NULL ) return FALSE;
   	*(PDWORD)(&pCreateToolhelp32Snapshot) = (DWORD)GetProcAddress( hKernel32, "CreateToolhelp32Snapshot" );
    *(PDWORD)(&pProcess32First) = (DWORD)GetProcAddress( hKernel32, "Process32First" );
    *(PDWORD)(&pProcess32Next)  = (DWORD)GetProcAddress( hKernel32, "Process32Next" );
    *(PDWORD)(&pModule32First)  = (DWORD)GetProcAddress( hKernel32, "Module32First" );
    *(PDWORD)(&pModule32Next)   = (DWORD)GetProcAddress( hKernel32, "Module32Next" );
    *(PDWORD)(&pThread32First)  = (DWORD)GetProcAddress( hKernel32, "Thread32First" );
    *(PDWORD)(&pThread32Next)   = (DWORD)GetProcAddress( hKernel32, "Thread32Next" );
    *(PDWORD)(&pVirtualAllocEx) = (DWORD)GetProcAddress( hKernel32, "VirtualAllocEx" );
    *(PDWORD)(&pVirtualFreeEx)  = (DWORD)GetProcAddress( hKernel32, "VirtualFreeEx" );
    *(PDWORD)(&pOpenProcess)    = (DWORD)GetProcAddress( hKernel32, "OpenProcess" );
    *(PDWORD)(&pOpenThread)     = (DWORD)GetProcAddress( hKernel32, "OpenThread" );
    if ( OSWin98 || OSWinMe )
    {
        *(PDWORD)(&pVirtualAllocEx) = (DWORD)VirtualsAllocEx9x;
        *(PDWORD)(&pVirtualFreeEx)  = (DWORD)VirtualsFreeEx9x;
    }
    if ( OSWin98 )
        *(PDWORD)(&pOpenThread) = (DWORD)OpensThread9x;
    
    return ( GOT = ( pCreateToolhelp32Snapshot && pProcess32First && pProcess32Next && pModule32First && pModule32Next && pThread32First && pThread32Next && pVirtualAllocEx && pVirtualFreeEx && pOpenProcess&& pOpenThread ) );
}

BOOL CheckPattern( CONST CHAR *targetString, CONST CHAR *sourceString )
{
    CONST CHAR *index, *i, *j;
    
    for( index=i=targetString; *index; index++ )
        if ( *index == '\\' )
            i = index+1;
        
    for( index=j=sourceString; *index; index++ )
        if ( *index == '\\' )
            j = index+1;
            
    for ( ; *i && *j ; i++,j++ )
    {
        if ( tolower( *i ) != tolower( *j ) )
            return FALSE;
    }
            
    return ( *j == 0 );
}

DWORD GetProcessInfo( CONST CHAR *targetName, CONST CHAR *libraryPath, PROCESS_INFORMATION *processInfo )
{ 
    CONST DWORD GET_PROCESS_INFO_SUCCESS = 2;
    
    // interpret parameter targetName
    if( targetName==NULL || targetName[0]==0 )
        return 0;
    CHAR target[MAX_PATH][MAX_PATH];
    DWORD countTarget = 0;
    DWORD i = 0, j = 0;
    while ( targetName[i] && countTarget<MAX_PATH )
    {
        if ( targetName[i] == '/' )
        {
            target[countTarget++][j] = 0;
            j = -1;
        }
        else
        {
            target[countTarget][j] = targetName[i];
        }
        i++,j++;
    }
    target[countTarget++][j] = 0;
    
    // search target's processID
    HANDLE hSnapshotProcess = pCreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    if ( hSnapshotProcess == INVALID_HANDLE_VALUE )
        return 0;
    PROCESSENTRY32 PE32;
    PE32.dwSize = sizeof( PROCESSENTRY32 );
    BOOL gotPE32 = FALSE;
    if ( pProcess32First( hSnapshotProcess, &PE32 ) )
    {
        do
        {
            for ( DWORD k=0; k<countTarget; k++)
            {
                // do not just use stricmp(), because .szExeFile
                // means target's fullpath when on Win9x
                if ( CheckPattern( PE32.szExeFile, target[k] ) )
                {
                    CloseHandle( hSnapshotProcess );
                    gotPE32 = TRUE;
                    break;
                }
            }
        } while ( (!gotPE32) && pProcess32Next( hSnapshotProcess, &PE32 ) );
    }
    if ( !gotPE32 )
    {
        CloseHandle( hSnapshotProcess );
        return 0;
    }
    
    // check target's loaded module
    HANDLE hSnapshotModule = pCreateToolhelp32Snapshot( TH32CS_SNAPMODULE, PE32.th32ProcessID );
    if ( hSnapshotModule == INVALID_HANDLE_VALUE )
        return 0;
    MODULEENTRY32  ME32;
    ME32.dwSize = sizeof( MODULEENTRY32 );
    if ( pModule32First( hSnapshotModule, &ME32 ) )
    {
        do
        {
            if ( CheckPattern( ME32.szExePath, libraryPath ) )
            {	
                // our library was already injected before
                CloseHandle( hSnapshotModule );
                return 1;
            }
        } while ( pModule32Next( hSnapshotModule, &ME32 ) );
    }
    
    // find out one of target's threads
    HANDLE hSnapshotThread = pCreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 );
    if ( hSnapshotThread == INVALID_HANDLE_VALUE )
        return 0;
    THREADENTRY32 TE32;
    TE32.dwSize = sizeof( THREADENTRY32 );
    BOOL gotTE32 = FALSE;
    if ( pThread32First( hSnapshotThread, &TE32 ) )
    {
        do
        {
            if ( TE32.th32OwnerProcessID == PE32.th32ProcessID )
            {
                CloseHandle( hSnapshotThread );
                gotTE32 = TRUE;
            }
        } while ( (!gotTE32) && pThread32Next( hSnapshotThread, &TE32 ) );
    }
    if ( !gotTE32 )
    {
        CloseHandle( hSnapshotThread );
        return 0;
    }
    
    // get target's hProcess and hThread
    processInfo->dwProcessId = PE32.th32ProcessID;
    processInfo->hProcess = pOpenProcess( PROCESS_ALL_ACCESS, FALSE, PE32.th32ProcessID );
    processInfo->dwThreadId = TE32.th32ThreadID;
    processInfo->hThread = pOpenThread( THREAD_ALL_ACCESS, FALSE, TE32.th32ThreadID );
    if ( processInfo->hProcess==NULL || processInfo->hThread==NULL )
        return 0;
    
    return GET_PROCESS_INFO_SUCCESS;
}

DWORD InjectLibrary( CONST CHAR *targetName, CONST CHAR *libraryPath )
{
    PROCESS_INFORMATION processInfo;
    InjectCode injectCode;
    DWORD      basePoint, loadLibraryA, endPoint;
    CONTEXT    orgContext, runContext;
    DWORD      libraryBase;
    
    // get kernel32 funciton pointers
    if ( !GetKernel32Function() )
        return 0;
    
    // get target's processInfo
    DWORD returnValue = GetProcessInfo( targetName, libraryPath, &processInfo );
    if ( returnValue==0 || returnValue==1 )
        return returnValue;
    
    // initialize injectCode
    if ( !( basePoint = (DWORD)pVirtualAllocEx( processInfo.hProcess, NULL, sizeof( InjectCode ), MEM_COMMIT, PAGE_EXECUTE_READWRITE ) ) )
        return 0;
    if ( !( loadLibraryA = (DWORD)GetProcAddress( LoadLibrary( TEXT( "kernel32.dll" ) ), "LoadLibraryA" ) ) )
        return 0;
    injectCode.PushOpc = 0x68;   // 0x68 means push
    injectCode.PushAdd = basePoint + offsetof( InjectCode, LibraryPath );
    injectCode.CallOpc = 0xE8;   // 0xE8 is a relative type call
    injectCode.CallAdd = loadLibraryA - basePoint - offsetof( InjectCode, Jmp_$ );
    injectCode.Jmp_$   = 0xFEEB; // 0xFEEB means jmp here, loop until we checked
    strcpy( injectCode.LibraryPath, libraryPath );
    endPoint = basePoint + offsetof( InjectCode, Jmp_$ );
    
    // writes injectCode to target's process
    if ( !WriteProcessMemory( processInfo.hProcess, (VOID*)basePoint, &injectCode, sizeof( InjectCode ), new DWORD ) )
    {
        pVirtualFreeEx( processInfo.hProcess, (VOID*)basePoint, sizeof( InjectCode ), MEM_DECOMMIT );
        return 0;
    }

    // let target's process excute our injectCode
    SuspendThread( processInfo.hThread );
    orgContext.ContextFlags = CONTEXT_FULL;
    if ( !GetThreadContext( processInfo.hThread, &orgContext ) )
        return 0;
   	runContext = orgContext;
    runContext.Eip = basePoint;
    if ( !SetThreadContext( processInfo.hThread, &runContext ) )
        return 0;  
    ResumeThread( processInfo.hThread );
    do
    {
        Sleep( 10 );
       	GetThreadContext( processInfo.hThread, &runContext );
    }  while ( runContext.Eip != endPoint );
    libraryBase = runContext.Eax;
    SuspendThread( processInfo.hThread );
    if ( !SetThreadContext( processInfo.hThread, &orgContext ) )
        return 0;
    ResumeThread( processInfo.hThread );
    
    // release resource
    pVirtualFreeEx( processInfo.hProcess, (VOID*)basePoint, sizeof( InjectCode ), MEM_DECOMMIT );
    CloseHandle( processInfo.hProcess );
    CloseHandle( processInfo.hThread );
    
    return libraryBase;
}