// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

// LockWorkStation official type definition
typedef BOOL(WINAPI* _LockWorkStation)();

BOOL WINAPI BlockedLockWorkStation()
{
	std::ofstream log("d:\\work\\dll_log.txt", std::ios::app | std::ios::out);
	log << "Blocked LockWorkStation" << std::endl;

	return TRUE;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	_LockWorkStation TrueLockWorkStation = (_LockWorkStation)GetProcAddress(GetModuleHandle(L"user32"), "LockWorkStation");
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Mhook_SetHook((PVOID*)&TrueLockWorkStation, BlockedLockWorkStation);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		Mhook_Unhook((PVOID*)&TrueLockWorkStation);
		break;
	}
	return TRUE;
}
