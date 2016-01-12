// InjectDLL.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#pragma comment(lib, "advapi32.lib")

#ifdef _DEBUG
#define LOG_DEBUG
#endif

BOOL SetPrivilege(
	HANDLE hToken,          // access token handle
	LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
	BOOL bEnablePrivilege   // to enable or disable privilege
	)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!LookupPrivilegeValue(
		NULL,            // lookup privilege on local system
		lpszPrivilege,   // privilege to lookup 
		&luid))        // receives LUID of privilege
	{
		printf("LookupPrivilegeValue error: %u\n", GetLastError());
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	// Enable the privilege or disable all privileges.

	if (!AdjustTokenPrivileges(
		hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL,
		(PDWORD)NULL))
	{
		printf("AdjustTokenPrivileges error: %u\n", GetLastError());
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

	{
		printf("The token does not have the specified privilege. \n");
		return FALSE;
	}

	return TRUE;
}


void LogLastError()
{
	DWORD   dwLastError = ::GetLastError();
	char   lpBuffer[512] = "?";
	if (dwLastError != 0)    // Don't want to see a "operation done successfully" error ;-)
	::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,                 // It´s a system error
					NULL,                                      // No string to be formatted needed
					dwLastError,                               // Hey Windows: Please explain this error!
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // Do it in the standard language
					lpBuffer,              // Put the message here
					(sizeof(lpBuffer) / sizeof(char)) - 1,                     // Number of bytes to store the message
					NULL);
	std::cout << "Last error: " << lpBuffer << std::endl;
}

bool InjectDllIntoProcess(DWORD dwPid, const std::string sInjectDll)
{
	bool bOk(true);
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
	if (process == NULL)
	{
		bOk = false;
		printf("Error: the specified process couldn't be found.\n");
		LogLastError();
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

struct MatchPathSeparator
{
	bool operator()(char ch) const
	{
		return ch == '\\' || ch == '/';
	}
};

std::string basename(const std::string& pathname)
{
	return std::string(
		std::find_if(pathname.rbegin(), pathname.rend(),
		MatchPathSeparator()).base(),
		pathname.end());
}

std::string GetProcessName(DWORD pid)
{
	std::string sProcessName;
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	if (NULL != hProcess)
	{
		char buffer[MAX_PATH] = "<unknown>";
		DWORD dwSize = sizeof(buffer) / sizeof(char);
		if (!QueryFullProcessImageNameA(hProcess, 0, buffer, &dwSize))
		{
			LogLastError();
			std::wcerr << "Failed to get processname of pid: " << pid << std::endl;
			sProcessName = "<unknown>";
		}
		else
		{
			sProcessName = basename(std::string(buffer));
		}
	}
	else
	{
		std::cout << "Unable to open process..." << std::endl;
		sProcessName = "<unknown>";
	}
	CloseHandle(hProcess);

	return sProcessName;
}

void PrintProcessInfo(DWORD pid)
{
#ifdef LOG_DEBUG
	printf("%s  (PID: %u)\n", GetProcessName(pid).c_str(), pid);
#endif
}


int _tmain(int argc, char* argv[])
{
#ifdef LOG_DEBUG
	std::ofstream log("d:\\work\\injectdll.txt", std::ios::app | std::ios::out);
	log << "Inject dll... " << std::endl;
#endif
	HANDLE hProc = GetCurrentProcess();

	HANDLE hToken;
	if (OpenProcessToken(hProc, TOKEN_ALL_ACCESS, &hToken))
	{
#ifdef LOG_DEBUG
		std::cout << "OPen process token ok" << std::endl;
#endif
		if (SetPrivilege(hToken, SE_DEBUG_NAME, TRUE))
		{
#ifdef LOG_DEBUG
			std::cout << "Privilige adjusted to debug" << std::endl;
#endif
		}
		else
		{
			std::cerr << "Failed to obtaing process debugging rights, injecting in system dlls could fail" << std::endl;
		}
	}
	else
	{
		std::cerr << "Open process token F A I L E D" << std::endl;
	}


	char path[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, path);

	std::string sInjectDll(argv[1]);
#ifdef LOG_DEBUG
	log << "Inject dll: " << sInjectDll << std::endl;
#endif
	std::string sProcess(argv[2]);

	int retryCount = 1;

	if (argc >= 4)
	{
		retryCount = atol(argv[3]);
	}

	for (int retryCounter = 0; retryCounter < retryCount; retryCounter++)
	{
		if (retryCounter != 0)
			Sleep(1000);

		std::transform(sProcess.begin(), sProcess.end(), sProcess.begin(), ::tolower);
#ifdef LOG_DEBUG
		log << "Process name: " << sProcess << std::endl;
#endif

#ifdef LOG_DEBUG
		std::cout << "Looking for process PID of process: " << sProcess << " to hook the dll: " << sInjectDll << " into..." << std::endl;
#endif

		DWORD dwPids[1024];
		DWORD cbNeeded;

		if (!EnumProcesses(dwPids, sizeof(dwPids), &cbNeeded))
		{
			std::cerr << "Could not enumerate processes on system!" << std::endl;
			return 1;
		}

		DWORD dwCount = cbNeeded / sizeof(DWORD);

		bool bInjected(false);
		for (DWORD i = 0; i < dwCount; ++i)
		{
			if (dwPids[i] != 0 && dwPids[i] > 10)
			{
				PrintProcessInfo(dwPids[i]);
				std::string sProcessName(GetProcessName(dwPids[i]));
				std::transform(sProcessName.begin(), sProcessName.end(), sProcessName.begin(), ::tolower);
				if (sProcessName == sProcess)
				{
#ifdef LOG_DEBUG
					log << "Process found in pid: " << dwPids[i] << std::endl;
#endif
					InjectDllIntoProcess(dwPids[i], sInjectDll);
					bInjected = true;
				}

			}
		}
		if (bInjected)
			break;
	}

	return 0;
}

