@echo off

If Exist C:\Backup GoTo inicia
Echo Diretorio na existe, criando ele...
MkDir C:\Backup
GoTo inicia


:Final
cls
Echo.
Echo FINALIZADO...  TECLE ALGO PARA SAIR...
Echo Arquivos estao na pasta C:\Backup
Echo.
pause
exit

:INICIA
set path=%path%;%ProgramFiles%\winrar

net stop mssql$sqlexpress

winrar a -u "c:\Backup\SQLBASE"  "%programFiles%\Microsoft SQL Server\MSSQL.1\MSSQL\Data\Acc*.*" 
winrar a -u "c:\Backup\SQLBASE"  "%programFiles%\Microsoft SQL Server\MSSQL.1\MSSQL\Data\Clan*.*"
winrar a -u "c:\Backup\SQLBASE"  "%programFiles%\Microsoft SQL Server\MSSQL.1\MSSQL\Data\Sod2d*.*"

net start mssql$sqlexpress

winrar a -r -u c:\Backup\DataServer C:\Server\DataServer\*.*
