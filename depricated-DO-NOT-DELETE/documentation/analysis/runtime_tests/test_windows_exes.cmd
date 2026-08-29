@echo off
setlocal

echo ===== TESTE CLIENTE ORIGINAL =====
cd /d C:\Users\gamer69\SummitTest\cliente

for %%F in (*.exe) do (
    echo.
    echo ================================
    echo RUN %%F
    echo ================================
    start "" /wait "%%F"
    echo ERRORLEVEL=!ERRORLEVEL!
)

echo.
echo ===== EVENTOS RECENTES =====
powershell -NoProfile -Command "Get-WinEvent -FilterHashtable @{LogName='Application'; StartTime=(Get-Date).AddMinutes(-20)} | Where-Object {$_.Message -match 'Game.exe|pstale.exe|psupdate.exe|SummitTaleClient.exe|Application Error|SideBySide'} | Select-Object TimeCreated,ProviderName,Id,Message | Format-List"

pause
