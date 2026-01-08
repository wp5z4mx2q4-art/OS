#include <iostream>
#include <windows.h>
#include <string>
#include <vector>

int RunProgram(const char* program, HANDLE in, HANDLE out) {
    STARTUPINFOA startup = { 0 };
    PROCESS_INFORMATION process = { 0 };

    startup.cb = sizeof(STARTUPINFO);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = in;
    startup.hStdOutput = out;
    startup.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    char command[256];
    strcpy_s(command, program);

    BOOL created = CreateProcessA(
        nullptr,
        command,
        nullptr,
        nullptr,
        TRUE,
        0,
        nullptr,
        nullptr,
        &startup,
        &process
    );

    if (!created) {
        return 0;
    }

    CloseHandle(process.hThread);
    return (int)process.hProcess;
}

int main() {
    std::cout << "Enter 4 numbers: ";
    std::string data;
    std::getline(std::cin, data);

    if (data.empty()) {
        return 0;
    }

    data += "\n";
    std::cout << "Output: ";
    std::cout.flush();

    SECURITY_ATTRIBUTES security;
    security.nLength = sizeof(SECURITY_ATTRIBUTES);
    security.bInheritHandle = TRUE;
    security.lpSecurityDescriptor = nullptr;

    HANDLE mainM_read, mainM_write;
    HANDLE mA_read, mA_write;
    HANDLE aP_read, aP_write;
    HANDLE pS_read, pS_write;

    CreatePipe(&mainM_read, &mainM_write, &security, 0);
    CreatePipe(&mA_read, &mA_write, &security, 0);
    CreatePipe(&aP_read, &aP_write, &security, 0);
    CreatePipe(&pS_read, &pS_write, &security, 0);

    DWORD writtenBytes;
    WriteFile(mainM_write, data.c_str(), (DWORD)data.length(), &writtenBytes, nullptr);
    CloseHandle(mainM_write);

    std::vector<int> handles;

    int mProcess = RunProgram("M.exe", mainM_read, mA_write);
    CloseHandle(mainM_read);
    CloseHandle(mA_write);
    WaitForSingleObject((HANDLE)mProcess, INFINITE);
    handles.push_back(mProcess);

    int aProcess = RunProgram("A.exe", mA_read, aP_write);
    CloseHandle(mA_read);
    CloseHandle(aP_write);
    WaitForSingleObject((HANDLE)aProcess, INFINITE);
    handles.push_back(aProcess);

    int pProcess = RunProgram("P.exe", aP_read, pS_write);
    CloseHandle(aP_read);
    CloseHandle(pS_write);
    WaitForSingleObject((HANDLE)pProcess, INFINITE);
    handles.push_back(pProcess);

    int sProcess = RunProgram("S.exe", pS_read, GetStdHandle(STD_OUTPUT_HANDLE));
    CloseHandle(pS_read);
    WaitForSingleObject((HANDLE)sProcess, INFINITE);
    handles.push_back(sProcess);

    for (int h : handles) {
        if (h) {
            CloseHandle((HANDLE)h);
        }
    }

    std::cout << std::endl;
    system("pause");
    return 0;
}