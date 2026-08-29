#include <windows.h>

extern int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);

extern "C" int WINAPI DebugWinMainCRTStartup()
{
    HANDLE f = CreateFileA("mingw_entry_reached.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f != INVALID_HANDLE_VALUE) {
        const char *msg = "ENTROU NO DEBUG ENTRY\n";
        DWORD written = 0;
        WriteFile(f, msg, lstrlenA(msg), &written, NULL);
        CloseHandle(f);
    }

    MessageBoxA(NULL, "ENTROU NO ENTRYPOINT DEBUG", "DEBUG MINGW", MB_OK);

    HINSTANCE hInst = GetModuleHandleA(NULL);
    LPSTR cmd = GetCommandLineA();

    return WinMain(hInst, NULL, cmd, SW_SHOWDEFAULT);
}
