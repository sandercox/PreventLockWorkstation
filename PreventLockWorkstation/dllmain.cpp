// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

// LockWorkStation official type definition
typedef BOOL(WINAPI* _LockWorkStation)();

//#define LOGTRACE 1

BOOL WINAPI BlockedLockWorkStation()
{
#ifdef LOGTRACE
	std::ofstream log("d:\\work\\dll_log64.txt", std::ios::app | std::ios::out);
	log << "Blocked LockWorkStation" << std::endl;
#endif
	return TRUE;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
#ifdef LOGTRACE
	std::ofstream log("d:\\work\\dll_log64.txt", std::ios::app | std::ios::out);
	log << "DLL Main... ";

	wchar_t fname[255];
	GetModuleFileName(hModule, fname, 255);
	std::wstring sWide(fname);
	std::string sFile(sWide.begin(), sWide.end());
	log << " " << sFile << " -- ";
	

#endif
	_LockWorkStation TrueLockWorkStation = (_LockWorkStation)GetProcAddress(GetModuleHandle(L"user32"), "LockWorkStation");
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
#ifdef LOGTRACE
		log << "DLL_PROCESS_ATTACH";
#endif
		if (Mhook_SetHook((PVOID*)&TrueLockWorkStation, BlockedLockWorkStation))
		{
#ifdef LOGTRACE
			log << "  -- hook set!";
#endif
		}
	}
		break;
	case DLL_THREAD_ATTACH:
#ifdef LOGTRACE
		log << "DLL_THREAD_ATTACH";
#endif
		break;
	case DLL_THREAD_DETACH:
#ifdef LOGTRACE
		log << "DLL_THREAD_DETACH";
#endif
		break;
	case DLL_PROCESS_DETACH:
#ifdef LOGTRACE
		log << "DLL_PROCESS_DETACH";
#endif
		Mhook_Unhook((PVOID*)&TrueLockWorkStation);
		break;
	}
#ifdef LOGTRACE
	log << std::endl;
#endif
	return TRUE;
}
