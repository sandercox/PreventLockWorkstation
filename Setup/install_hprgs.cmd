@echo off

if exist "..\x64\Release\PreventLockWorkstation_x64.dll" (
if exist "..\Release\PreventLockWorkstation_x86.dll" (
if exist "..\Release\InjectDLL.exe" (
mkdir C:\PreventLockWorkStation
copy "..\x64\Release\PreventLockWorkstation_x64.dll" c:\preventlockworkstation
copy "..\Release\PreventLockWorkstation_x86.dll" c:\preventlockworkstation
copy "..\Release\InjectDLL.exe" c:\preventlockworkstation
reg import registry_hprgs.reg

)))

if exist "PreventLockWorkstation_x64.dll" (
if exist "PreventLockWorkstation_x86.dll" (
if exist "InjectDLL.exe" (
mkdir C:\PreventLockWorkStation
copy "PreventLockWorkstation_x64.dll" c:\preventlockworkstation
copy "PreventLockWorkstation_x86.dll" c:\preventlockworkstation
copy "InjectDLL.exe" c:\preventlockworkstation

reg import registry_hprgs.reg

)))
