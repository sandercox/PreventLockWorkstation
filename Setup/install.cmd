@echo off
echo "helo"

if exist "..\x64\Release\PreventLockWorkstation_x64.dll" (
if exist "..\Release\PreventLockWorkstation_x86.dll" (
mkdir c:\preventlockworkstation
copy "..\x64\Release\PreventLockWorkstation_x64.dll" c:\preventlockworkstation
copy "..\Release\PreventLockWorkstation_x86.dll" c:\preventlockworkstation

reg import registry.reg

))

if exist "PreventLockWorkstation_x64.dll" (
if exist "PreventLockWorkstation_x86.dll" (
mkdir c:\preventlockworkstation
copy "PreventLockWorkstation_x64.dll" c:\preventlockworkstation
copy "PreventLockWorkstation_x86.dll" c:\preventlockworkstation

reg import registry.reg

))
