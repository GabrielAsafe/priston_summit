#include <windows.h>
#include <imagehlp.h>
#include "WinDump.h"
#include <string>

using namespace std;

#pragma comment(lib,"Imagehlp.lib")

typedef	BOOL(WINAPI * MINIDUMP_WRITE_DUMP)(
	IN HANDLE			hProcess,
	IN DWORD			ProcessId,
	IN HANDLE			hFile,
	IN MINIDUMP_TYPE	DumpType,
	IN CONST PMINIDUMP_EXCEPTION_INFORMATION	ExceptionParam, OPTIONAL
	IN PMINIDUMP_USER_STREAM_INFORMATION		UserStreamParam, OPTIONAL
	IN PMINIDUMP_CALLBACK_INFORMATION			CallbackParam OPTIONAL
	);

void create_minidump(struct _EXCEPTION_POINTERS* apExceptionInfo)
{
	HMODULE	hDbgHelp;
	MINIDUMP_WRITE_DUMP	MiniDumpWriteDump_;

	hDbgHelp = LoadLibraryA("DBGHELP.DLL");

	if(hDbgHelp)
	{
		MiniDumpWriteDump_ = (MINIDUMP_WRITE_DUMP)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");

		if(MiniDumpWriteDump_)
		{
			MINIDUMP_EXCEPTION_INFORMATION  M;
			HANDLE  hDump_File;

			M.ThreadId = GetCurrentThreadId();
			M.ExceptionPointers = apExceptionInfo;  //got by GetExceptionInformation()
			M.ClientPointers = 0;		

			hDump_File = CreateFileA("Configs//Crash.dmp", GENERIC_WRITE, 0,
									 NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			const DWORD Flags = MiniDumpWithFullMemory |
				MiniDumpWithFullMemoryInfo |
				MiniDumpWithHandleData |
				MiniDumpWithUnloadedModules |
				MiniDumpWithThreadInfo;

			MiniDumpWriteDump_(GetCurrentProcess(), GetCurrentProcessId(),
							   hDump_File, (MINIDUMP_TYPE)Flags,
							   (apExceptionInfo) ? &M : NULL, NULL, NULL);

			CloseHandle(hDump_File);
		}
	}
}

LONG WINAPI unhandled_handler(struct _EXCEPTION_POINTERS* apExceptionInfo)
{

	/* MINGW_SERVER_CRASH_STACK_V2 */
	if(apExceptionInfo &&
	   apExceptionInfo->ExceptionRecord &&
	   apExceptionInfo->ContextRecord)
	{
		CONTEXT *ctx = apExceptionInfo->ContextRecord;

		DWORD eip = 0;
		DWORD esp = 0;
		DWORD ebp = 0;
		DWORD eax = 0;
		DWORD ebx = 0;
		DWORD ecx = 0;
		DWORD edx = 0;
		DWORD esi = 0;
		DWORD edi = 0;

#if defined(__i386__) || defined(_M_IX86)
		eip = ctx->Eip;
		esp = ctx->Esp;
		ebp = ctx->Ebp;
		eax = ctx->Eax;
		ebx = ctx->Ebx;
		ecx = ctx->Ecx;
		edx = ctx->Edx;
		esi = ctx->Esi;
		edi = ctx->Edi;
#endif

		DWORD exeBase =
			(DWORD)(ULONG_PTR)GetModuleHandleA(NULL);

		DWORD exeSize = 0;

		if(exeBase)
		{
			IMAGE_DOS_HEADER *dos =
				(IMAGE_DOS_HEADER *)(ULONG_PTR)exeBase;

			if(dos->e_magic == IMAGE_DOS_SIGNATURE)
			{
				IMAGE_NT_HEADERS *nt =
					(IMAGE_NT_HEADERS *)
					((BYTE *)dos + dos->e_lfanew);

				if(nt->Signature == IMAGE_NT_SIGNATURE)
					exeSize = nt->OptionalHeader.SizeOfImage;
			}
		}

		HANDLE h =
			CreateFileA(
				"LOGS\\mingw_server_crash_stack.txt",
				GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL
			);

		if(h != INVALID_HANDLE_VALUE)
		{
			char line[1024];
			DWORD written = 0;
			int len;

#define MINGW_CRASH_WRITE(...) \
			do { \
				len = wsprintfA(line, __VA_ARGS__); \
				if(len > 0) \
					WriteFile(h, line, (DWORD)len, &written, NULL); \
			} while(0)

			MINGW_CRASH_WRITE(
				"MINGW_SERVER_CRASH_STACK_V2\r\n"
			);

			MINGW_CRASH_WRITE(
				"CODE=0x%08lX EXCEPTION_ADDR=0x%08lX\r\n",
				(unsigned long)
					apExceptionInfo->ExceptionRecord->ExceptionCode,
				(unsigned long)(DWORD)(ULONG_PTR)
					apExceptionInfo->ExceptionRecord->ExceptionAddress
			);

			MINGW_CRASH_WRITE(
				"EXE_BASE=0x%08lX EXE_SIZE=0x%08lX "
				"EXE_END=0x%08lX\r\n",
				(unsigned long)exeBase,
				(unsigned long)exeSize,
				(unsigned long)(exeBase + exeSize)
			);

			MINGW_CRASH_WRITE(
				"EIP=0x%08lX ESP=0x%08lX EBP=0x%08lX\r\n",
				(unsigned long)eip,
				(unsigned long)esp,
				(unsigned long)ebp
			);

			MINGW_CRASH_WRITE(
				"EAX=0x%08lX EBX=0x%08lX "
				"ECX=0x%08lX EDX=0x%08lX "
				"ESI=0x%08lX EDI=0x%08lX\r\n",
				(unsigned long)eax,
				(unsigned long)ebx,
				(unsigned long)ecx,
				(unsigned long)edx,
				(unsigned long)esi,
				(unsigned long)edi
			);

			MEMORY_BASIC_INFORMATION mbi;

			ZeroMemory(&mbi, sizeof(mbi));

			if(VirtualQuery(
				(LPCVOID)(ULONG_PTR)eip,
				&mbi,
				sizeof(mbi)))
			{
				MINGW_CRASH_WRITE(
					"EIP_MEM base=0x%08lX alloc=0x%08lX "
					"size=0x%08lX state=0x%08lX "
					"protect=0x%08lX type=0x%08lX\r\n",
					(unsigned long)(DWORD)(ULONG_PTR)mbi.BaseAddress,
					(unsigned long)(DWORD)(ULONG_PTR)mbi.AllocationBase,
					(unsigned long)(DWORD)mbi.RegionSize,
					(unsigned long)mbi.State,
					(unsigned long)mbi.Protect,
					(unsigned long)mbi.Type
				);
			}

			ZeroMemory(&mbi, sizeof(mbi));

			if(VirtualQuery(
				(LPCVOID)(ULONG_PTR)esp,
				&mbi,
				sizeof(mbi)))
			{
				MINGW_CRASH_WRITE(
					"ESP_MEM base=0x%08lX alloc=0x%08lX "
					"size=0x%08lX state=0x%08lX "
					"protect=0x%08lX type=0x%08lX\r\n",
					(unsigned long)(DWORD)(ULONG_PTR)mbi.BaseAddress,
					(unsigned long)(DWORD)(ULONG_PTR)mbi.AllocationBase,
					(unsigned long)(DWORD)mbi.RegionSize,
					(unsigned long)mbi.State,
					(unsigned long)mbi.Protect,
					(unsigned long)mbi.Type
				);
			}

			MINGW_CRASH_WRITE(
				"---- STACK CANDIDATES ----\r\n"
			);

			for(int i = 0; i < 128; i++)
			{
				DWORD slotAddr = esp + (DWORD)(i * sizeof(DWORD));

				MEMORY_BASIC_INFORMATION slotMbi;
				ZeroMemory(&slotMbi, sizeof(slotMbi));

				if(!VirtualQuery(
					(LPCVOID)(ULONG_PTR)slotAddr,
					&slotMbi,
					sizeof(slotMbi)))
					break;

				if(slotMbi.State != MEM_COMMIT)
					break;

				if(slotMbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))
					break;

				DWORD value =
					*(DWORD *)(ULONG_PTR)slotAddr;

				MEMORY_BASIC_INFORMATION valueMbi;
				ZeroMemory(&valueMbi, sizeof(valueMbi));

				if(VirtualQuery(
					(LPCVOID)(ULONG_PTR)value,
					&valueMbi,
					sizeof(valueMbi)) &&
				   valueMbi.State == MEM_COMMIT &&
				   valueMbi.Type == MEM_IMAGE)
				{
					char module[MAX_PATH];
					ZeroMemory(module, sizeof(module));

					HMODULE mod =
						(HMODULE)valueMbi.AllocationBase;

					GetModuleFileNameA(
						mod,
						module,
						MAX_PATH - 1
					);

					DWORD modBase =
						(DWORD)(ULONG_PTR)
						valueMbi.AllocationBase;

					MINGW_CRASH_WRITE(
						"S+%03d addr=0x%08lX "
						"val=0x%08lX "
						"moduleBase=0x%08lX "
						"rva=0x%08lX "
						"module='%s'\r\n",
						i * 4,
						(unsigned long)slotAddr,
						(unsigned long)value,
						(unsigned long)modBase,
						(unsigned long)(value - modBase),
						module
					);
				}
				else if(i < 32)
				{
					MINGW_CRASH_WRITE(
						"S+%03d addr=0x%08lX "
						"val=0x%08lX\r\n",
						i * 4,
						(unsigned long)slotAddr,
						(unsigned long)value
					);
				}
			}

			MINGW_CRASH_WRITE(
				"---- END ----\r\n"
			);

#undef MINGW_CRASH_WRITE

			FlushFileBuffers(h);
			CloseHandle(h);
		}
	}


	/* MINGW_SERVER_CRASH_INFO_V1 */
	if(apExceptionInfo &&
	   apExceptionInfo->ExceptionRecord &&
	   apExceptionInfo->ContextRecord)
	{
		DWORD crashCode =
			(DWORD)apExceptionInfo->ExceptionRecord->ExceptionCode;

		DWORD crashAddr =
			(DWORD)(ULONG_PTR)
			apExceptionInfo->ExceptionRecord->ExceptionAddress;

		DWORD moduleBase =
			(DWORD)(ULONG_PTR)GetModuleHandleA(NULL);

		DWORD crashRva =
			(crashAddr >= moduleBase)
				? (crashAddr - moduleBase)
				: 0;

		DWORD eip = 0;
		DWORD esp = 0;
		DWORD ebp = 0;

#if defined(__i386__) || defined(_M_IX86)
		eip = apExceptionInfo->ContextRecord->Eip;
		esp = apExceptionInfo->ContextRecord->Esp;
		ebp = apExceptionInfo->ContextRecord->Ebp;
#endif

		DWORD info0 = 0;
		DWORD info1 = 0;

		if(apExceptionInfo->ExceptionRecord->NumberParameters > 0)
			info0 = (DWORD)
				apExceptionInfo->ExceptionRecord->ExceptionInformation[0];

		if(apExceptionInfo->ExceptionRecord->NumberParameters > 1)
			info1 = (DWORD)
				apExceptionInfo->ExceptionRecord->ExceptionInformation[1];

		HANDLE hCrashLog =
			CreateFileA(
				"LOGS\\mingw_server_crash.txt",
				FILE_APPEND_DATA,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL
			);

		if(hCrashLog != INVALID_HANDLE_VALUE)
		{
			char line[768];

			int len = wsprintfA(
				line,
				"MINGW_SERVER_CRASH_INFO_V1 "
				"code=0x%08lX "
				"addr=0x%08lX "
				"base=0x%08lX "
				"rva=0x%08lX "
				"eip=0x%08lX "
				"esp=0x%08lX "
				"ebp=0x%08lX "
				"info0=0x%08lX "
				"info1=0x%08lX "
				"thread=%lu\r\n",
				(unsigned long)crashCode,
				(unsigned long)crashAddr,
				(unsigned long)moduleBase,
				(unsigned long)crashRva,
				(unsigned long)eip,
				(unsigned long)esp,
				(unsigned long)ebp,
				(unsigned long)info0,
				(unsigned long)info1,
				(unsigned long)GetCurrentThreadId()
			);

			DWORD written = 0;

			if(len > 0)
			{
				WriteFile(
					hCrashLog,
					line,
					(DWORD)len,
					&written,
					NULL
				);

				FlushFileBuffers(hCrashLog);
			}

			CloseHandle(hCrashLog);
		}
	}

	create_minidump(apExceptionInfo);
	ExitProcess(0);
	return EXCEPTION_CONTINUE_SEARCH;
}



