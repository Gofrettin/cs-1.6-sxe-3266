/////////////////////////////////////////////////////////////////////////////////////
// File        : InjectLibrary.h
// Author      : liuzewei
// Date        : 2007.1.17
// Description : Inject your library to a target process
//               Support Win98, WinMe, Win2000, WinXP, Win2003 or later
// Credits     : If you use these in your project, don't forget credits
//               Miguel Feijao, yoda, patrick, Azorbix and me ^_^
//               Special credits given to Jeffrey Richter
/////////////////////////////////////////////////////////////////////////////////////

#ifndef INJECT_LIBRARY_H
#define INJECT_LIBRARY_H

#pragma warning(disable:4996)

#include <windows.h>

// Parameter
// targetName  : target process's exe name, if this name are indeterminate, 
//               we can use '/'(means or) to space each of possible names
//               for example, "nameA.exe/nameB.exe/nameC.exe"
// libraryPath : our library's full path
//
// Return Value
// 0  : inject unsuccessfully
// 1  : target process was already injected with our library before, so 
//      just returned but did nothing
// >1 : inject successfully, this return value is the base address of 
//      our library in target process
DWORD InjectLibrary( CONST CHAR *targetName, CONST CHAR *libraryPath );

#endif