#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <shellapi.h>
#include <string>
#include <cstdio>
#include <locale.h>
#include <tlhelp32.h>
#include <iostream>

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