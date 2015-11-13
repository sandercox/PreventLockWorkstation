// TestApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main()
{
	HANDLE lib = LoadLibrary(L"PreventLockWorkstation_x86.dll");

	LockWorkStation();


    return 0;
}

