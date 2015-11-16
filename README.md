# PreventLockWorkStation

DLL that can be hooked into any process to prevent the LockWorkStation() call to user32.dll

This program was designed to circumvent the automatic locking of a workstation by the HP Remote Graphics Server
when a client disconnects.

## Binaries

http://scox.nl/PreventLockWorkStation.zip

Extract contents of archive and run the install script (install_hprgs.cmd) do this as administrator!

Turn off UAC on the computer to start have the inject process work on logon.

All programms will be installed into c:\PreventLockWorkStation

Optionally you can use install_appinit.cmd to install the program as AppInit_DLLs making the dll load into all programs
started by Windows. However it seems that sub processes do not call this and therefore HP rgserver.exe 'forgot' to
load this hack dll. Which lead to the creation of InjectDll