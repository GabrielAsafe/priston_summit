@echo off
setlocal
cd /d "%%~dp0"
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86
set "VCTargetsPath=C:\Program Files (x86)\MSBuild\Microsoft.Cpp\v4.0\V140\"
set "PATH=C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x86;%C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\;C:\Windows\System32\OpenSSH\;C:\Program Files\Git\cmd;C:\Program Files (x86)\Windows Kits\10\Windows Performance Toolkit\;C:\Users\gaymer\AppData\Local\Microsoft\WindowsApps;%"
set "INCLUDE=%C:\Users\gaymer\Desktop\SummitTale%\directx\DirectX9\Include;%C:\Users\gaymer\Desktop\SummitTale%\Shared;%C:\Users\gaymer\Desktop\SummitTale%\modules;%C:\Users\gaymer\Desktop\SummitTale%\SrcServer\src\Skills;%%INCLUDE%%"
set "LIB=%C:\Users\gaymer\Desktop\SummitTale%\directx\DirectX9\Lib;%%LIB%%"
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" SrcGame\src\priston.vcxproj /p:Configuration=Debug /p:Platform=Win32 /p:PlatformToolset=v140 /p:WindowsTargetPlatformVersion=10.0.19041.0
endlocal
