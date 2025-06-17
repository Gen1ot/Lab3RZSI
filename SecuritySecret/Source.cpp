#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <shellapi.h>
#include <string>
#include <cstdio>
#include <locale.h>
#include <cstring>
#include <heapapi.h>
#include <tlhelp32.h>
#include <iostream>

#define STOP_ARG "BOOM! Ops, awkword... You are hacked, haha)"

BOOL CreateProcessWithBlockDllPolicy(IN LPSTR lpProcessPath, OUT DWORD* dwProcessId,
    OUT HANDLE* hProcess, OUT HANDLE* hThread) {
    STARTUPINFOEXA SiEx = { 0 };
    PROCESS_INFORMATION Pi = { 0 };
    SIZE_T sAttrSize = NULL;
    if (lpProcessPath == NULL)
        return FALSE;
    RtlSecureZeroMemory(&SiEx, sizeof(STARTUPINFOEXA));
    RtlSecureZeroMemory(&Pi, sizeof(PROCESS_INFORMATION));
    SiEx.StartupInfo.cb = sizeof(STARTUPINFOEXA);
    SiEx.StartupInfo.dwFlags = EXTENDED_STARTUPINFO_PRESENT;
    InitializeProcThreadAttributeList(NULL, 1, NULL, &sAttrSize);
    LPPROC_THREAD_ATTRIBUTE_LIST pAttrBuf =
        (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sAttrSize);
    if (!InitializeProcThreadAttributeList(pAttrBuf, 1, NULL, &sAttrSize)) {
        printf("[!] InitializeProcThreadAttributeList Failed With Error : %d \n",
            GetLastError());
        return FALSE;
    }
    DWORD64 dwPolicy =
        PROCESS_CREATION_MITIGATION_POLICY_BLOCK_NON_MICROSOFT_BINARIES_ALWAYS_ON;
    if (!UpdateProcThreadAttribute(pAttrBuf, NULL,
        PROC_THREAD_ATTRIBUTE_MITIGATION_POLICY, &dwPolicy, sizeof(DWORD64), NULL, NULL)) {
        printf("[!] UpdateProcThreadAttribute Failed With Error : %d \n",
            GetLastError());
        return FALSE;
    }
    SiEx.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)pAttrBuf;
    if (!CreateProcessA(
        NULL,
        lpProcessPath,
        NULL,
        NULL,
        FALSE,
        EXTENDED_STARTUPINFO_PRESENT,
        NULL,
        NULL,
        &SiEx.StartupInfo,
        &Pi)) {
        printf("[!] CreateProcessA Failed With Error : %d \n", GetLastError());
        return FALSE;
    }
    *dwProcessId = Pi.dwProcessId;
    *hProcess = Pi.hProcess;
    *hThread = Pi.hThread;
    DeleteProcThreadAttributeList(pAttrBuf);
    HeapFree(GetProcessHeap(), 0, pAttrBuf);

    return TRUE;
}

DWORD GetProcessIdByName(const std::wstring& processName) {
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(snapshot, &processEntry)) {
            do {
                if (_wcsicmp(processEntry.szExeFile, processName.c_str()) == 0) {
                    pid = processEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(snapshot, &processEntry));
        }
        CloseHandle(snapshot);
    }
    return pid; // 0, если процесс не найден
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        CreateWindowA("BUTTON", "Click me and I tell you a secret!",
            WS_VISIBLE | WS_CHILD,
            100, 80, 250, 50,
            hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            std::wstring targetProcess = L"TargetProcess.exe";
            DWORD pid = GetProcessIdByName(targetProcess);
            char command[512];
            sprintf(command, "rundll32.exe DllInjectorAsDll.dll,HelperFunc %lu", pid);
            ShellExecuteA(NULL, "open", "cmd.exe", (std::string("/c ") + command).c_str(), NULL, SW_HIDE);
        }
        break;

    case WM_ERASEBKGND: {       //окошко
        RECT rect;
        GetClientRect(hwnd, &rect);
        HDC hdc = (HDC)wParam;
        HBRUSH hBrush = CreateSolidBrush(RGB(200, 230, 255));
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);
        return 1;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    if (strcmp(lpCmdLine, STOP_ARG) != 0) {
        char filename[MAX_PATH] = { 0 };
        if (!GetModuleFileNameA(NULL, filename, sizeof(filename))) return -1;

        size_t cmdLen = strlen(filename) + strlen(STOP_ARG) + 4;
        char* cmdLine = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cmdLen);
        if (!cmdLine) return -1;

        sprintf_s(cmdLine, cmdLen, "\"%s\" %s", filename, STOP_ARG);
        DWORD pid; HANDLE hProc, hThread;

        if (CreateProcessWithBlockDllPolicy(cmdLine, &pid, &hProc, &hThread)) {
            CloseHandle(hProc);
            CloseHandle(hThread);
        }

        HeapFree(GetProcessHeap(), 0, cmdLine);
        return 0;
    }

    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "Prize";
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0, "Prize", "!",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        100, 100, 500, 300,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}