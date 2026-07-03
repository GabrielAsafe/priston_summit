@echo off
setlocal
set 
"
ROOT=
C:\Users\gaymer\Desktop\SummitTale\SourceCode Summit Tale
"
cd /d 
"
%ROOT%
"
call 
"
C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat
"
 x86
set 
"
VCTargetsPath=C:\Program Files (x86)\MSBuild\Microsoft.Cpp\v4.0\V140\
"
set 
"
INCLUDE=
C:\Users\gaymer\Desktop\SummitTale_old_broken\Dependency\DirectX9\Include
;%ROOT%\Shared;%ROOT%\modules;%ROOT%\SrcServer\src\Skills;%INCLUDE%
"
set 
"
LIB=
C:\Users\gaymer\Desktop\SummitTale_old_broken\Dependency\DirectX9\Lib
;%ROOT%\SrcGame\src\Library;%LIB%
"
"
C:\Program Files (x86)\MSBuild\14.0\Bin\MSBuild.exe
"
 
"
%ROOT%\SrcGame\src\priston.vcxproj
"
 /p:Configuration=Debug /p:Platform=Win32 /p:PlatformToolset=v140 /m /fl /flp:logfile=
"
%ROOT%\build_client.log
"
;verbosity=normal
endlocal
