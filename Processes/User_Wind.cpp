#include <windows.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <tlhelp32.h>

using namespace std;

bool process_exists(int pid) {
    HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!h) return false;
    DWORD code;
    GetExitCodeProcess(h, &code);
    CloseHandle(h);
    return code == STILL_ACTIVE;
}

bool process_exists(string name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    bool found = false;

    if (Process32First(snap, &pe)) {
        do {
            char buf[260];
            size_t conv;
            wcstombs_s(&conv, buf, pe.szExeFile, 259);

            string exe = buf;
            for (char& c : exe) c = tolower(c);
            for (char& c : name) c = tolower(c);

            if (exe == name) {
                found = true;
                break;
            }
        } while (Process32Next(snap, &pe));
    }

    CloseHandle(snap);
    return found;
}

int start_proc(string cmd) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (CreateProcessA(NULL, (char*)cmd.c_str(), NULL, NULL, FALSE,
        CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        Sleep(1000);
        return pi.dwProcessId;
    }
    return 0;
}

int main() {
    cout << "User Application - Testing Killer" << endl;
    cout << "=================================" << endl;
    cout << endl;

    SetEnvironmentVariableA("PROC_TO_KILL", "notepad.exe,calc.exe");
    cout << "Set PROC_TO_KILL = notepad.exe,calc.exe" << endl;
    cout << endl;

    cout << "Test 1: No arguments" << endl;
    int pid1 = start_proc("notepad.exe");
    int pid2 = start_proc("calc.exe");

    if (!pid1 || !pid2) {
        cout << "Error starting processes" << endl;
        return 1;
    }

    cout << "Before: notepad.exe = " << (process_exists("notepad.exe") ? "yes" : "no") << endl;
    cout << "Before: calc.exe = " << (process_exists("calc.exe") ? "yes" : "no") << endl;

    system("killer.exe");
    Sleep(2000);

    cout << "After: notepad.exe = " << (process_exists("notepad.exe") ? "yes" : "no") << endl;
    cout << "After: calc.exe = " << (process_exists("calc.exe") ? "yes" : "no") << endl;
    cout << endl;

    cout << "Test 2: --id parameter" << endl;
    int pid3 = start_proc("notepad.exe");

    if (!pid3) {
        cout << "Error starting process" << endl;
        return 1;
    }

    cout << "Before: Process " << pid3 << " = " << (process_exists(pid3) ? "yes" : "no") << endl;

    string cmd1 = "killer.exe --id " + to_string(pid3);
    system(cmd1.c_str());
    Sleep(2000);

    cout << "After: Process " << pid3 << " = " << (process_exists(pid3) ? "yes" : "no") << endl;
    cout << endl;

    cout << "Test 3: --name parameter" << endl;
    int pid4 = start_proc("notepad.exe");
    Sleep(500);
    int pid5 = start_proc("notepad.exe");

    if (!pid4 || !pid5) {
        cout << "Error starting processes" << endl;
        return 1;
    }

    cout << "Before: notepad.exe processes = " << (process_exists("notepad.exe") ? "yes" : "no") << endl;

    system("killer.exe --name notepad.exe");
    Sleep(2000);

    cout << "After: notepad.exe processes = " << (process_exists("notepad.exe") ? "yes" : "no") << endl;
    cout << endl;

    SetEnvironmentVariableA("PROC_TO_KILL", NULL);
    cout << "Removed PROC_TO_KILL" << endl;
    cout << endl;

    cout << "Testing complete" << endl;

    return 0;
}