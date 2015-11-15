// InjectDLL.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

bool InjectDllIntoProcess(DWORD dwPid, const std::string sInjectDll)
{
	bool bOk(true);
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
	if (process == NULL)
	{
		bOk = false;
		printf("Error: the specified process couldn't be found.\n");
	}

	if (bOk)
	{
		// Get address of the LoadLibrary function.
		LPVOID addr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
		LPVOID arg(0);
		if (addr == NULL) 
		{
			bOk = false;
			printf("Error: the LoadLibraryA function was not found inside kernel32.dll library.\n");
		}

		if (bOk)
		{
			// Allocate new memory region inside the process's address space.
			arg = (LPVOID)VirtualAllocEx(process, NULL, sInjectDll.length(), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (arg == NULL)
			{
				bOk = false;
				printf("Error: the memory could not be allocated inside the chosen process.\n");
			}
		}

		if (bOk)
		{
			// Write the argument to LoadLibraryA to the process's newly allocated memory region.
			int n = WriteProcessMemory(process, arg, sInjectDll.c_str(), sInjectDll.length(), NULL);
			if (n == 0) 
			{
				bOk = false;
				printf("Error: there was no bytes written to the process's address space.\n");
			}
		}

		if (bOk)
		{
			// Create the remote thread into the process
			HANDLE threadID = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)addr, arg, NULL, NULL);
			if (threadID == NULL) 
			{
				bOk = false;
				printf("Error: the remote thread could not be created.\n");
			}
			else 
			{
				printf("Success: the remote thread was successfully created.\n");
			}
		}

		// Close the handle to the process, becuase we've already injected the DLL.
		CloseHandle(process);
	}
	return bOk;
}

std::string GetProcessName(DWORD pid)
{
	char buffer[MAX_PATH] = "<unknown>";
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (NULL != hProcess)
	{
		HMODULE hMod[512];
		DWORD cbNeeded;

		if (EnumProcessModules(hProcess, hMod, sizeof(hMod), &cbNeeded))
		{
			GetModuleBaseNameA(hProcess, hMod[0], buffer, sizeof(buffer) / sizeof(char));
		}
	}
	CloseHandle(hProcess);

	return std::string(buffer);
}

void PrintProcessInfo(DWORD pid)
{
	printf("%s  (PID: %u)\n", GetProcessName(pid).c_str(), pid);
}


int _tmain(int argc, char* argv[])
{
	char path[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, path);

	std::string sInjectDll(path);
	sInjectDll += "\\PreventLockWorkStation_x86.dll";

	std::string sProcess(argv[1]);
	std::transform(sProcess.begin(), sProcess.end(), sProcess.begin(), ::tolower);

	std::cout << "Looking for process PID of process: " << sProcess << " to hook the dll: " << sInjectDll << " into..." << std::endl;

	DWORD dwPids[1024];
	DWORD cbNeeded;

	if (!EnumProcesses(dwPids, sizeof(dwPids), &cbNeeded))
	{
		std::cerr << "Could not enumerate processes on system!" << std::endl;
		return 1;
	}

	DWORD dwCount = cbNeeded / sizeof(DWORD);

	for (DWORD i = 0; i < dwCount; ++i)
	{
		if (dwPids[i] != 0)
		{
			PrintProcessInfo(dwPids[i]);
			std::string sProcessName(GetProcessName(dwPids[i]));
			std::transform(sProcessName.begin(), sProcessName.end(), sProcessName.begin(), ::tolower);
			if (sProcessName == sProcess)
			{
				InjectDllIntoProcess(dwPids[i], sInjectDll);
			}

		}
	}

	return 0;
}

