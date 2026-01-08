#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <tlhelp32.h>

using namespace std;

void kill_by_id(int pid) {
    HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!h) {
        cout << "Cannot find process " << pid << endl;
        return;
    }
    if (TerminateProcess(h, 0)) {
        cout << "Process " << pid << " terminated" << endl;
    }
    else {
        cout << "Error terminating process " << pid << endl;
    }
    CloseHandle(h);
}

vector<int> find_by_name(string name) {
    vector<int> result;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return result;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snap, &pe)) {
        do {
            string exe;
            char buffer[260];
            size_t converted = 0;
            wcstombs_s(&converted, buffer, sizeof(buffer), pe.szExeFile, _TRUNCATE);
            buffer[sizeof(buffer) - 1] = '\0';
            exe = buffer;

            string name_lower = name;
            string exe_lower = exe;
            for (char& c : name_lower) c = tolower(c);
            for (char& c : exe_lower) c = tolower(c);

            if (exe_lower == name_lower) {
                result.push_back(pe.th32ProcessID);
            }
        } while (Process32Next(snap, &pe));
    }

    CloseHandle(snap);
    return result;
}

void kill_by_name(string name) {
    vector<int> pids = find_by_name(name);
    if (pids.empty()) {
        cout << "No processes found with name '" << name << "'" << endl;
        return;
    }
    cout << "Found " << pids.size() << " process(es) with name '" << name << "'" << endl;
    for (int pid : pids) {
        kill_by_id(pid);
    }
}

vector<string> get_env_processes() {
    vector<string> result;
    char buf[2048];

    if (GetEnvironmentVariableA("PROC_TO_KILL", buf, sizeof(buf)) == 0) {
        return result;
    }

    string s = buf;
    string item;
    for (char c : s) {
        if (c == ',') {
            while (!item.empty() && (item[0] == ' ' || item[0] == '\t' || item[0] == '"')) {
                item.erase(0, 1);
            }
            while (!item.empty() && (item.back() == ' ' || item.back() == '\t' || item.back() == '"')) {
                item.pop_back();
            }
            if (!item.empty()) result.push_back(item);
            item.clear();
        }
        else {
            item += c;
        }
    }

    if (!item.empty()) {
        while (!item.empty() && (item[0] == ' ' || item[0] == '\t' || item[0] == '"')) {
            item.erase(0, 1);
        }
        while (!item.empty() && (item.back() == ' ' || item.back() == '\t' || item.back() == '"')) {
            item.pop_back();
        }
        if (!item.empty()) result.push_back(item);
    }

    return result;
}

void kill_from_env() {
    cout << "=== Processing PROC_TO_KILL ===" << endl;
    vector<string> procs = get_env_processes();

    if (procs.empty()) {
        cout << "No processes to terminate" << endl;
        return;
    }

    for (string name : procs) {
        kill_by_name(name);
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        kill_from_env();
        return 0;
    }

    string cmd = argv[1];

    if (cmd == "--id" && argc > 2) {
        try {
            int pid = stoi(argv[2]);
            kill_by_id(pid);
        }
        catch (...) {
            cout << "Error: invalid ID" << endl;
        }
    }
    else if (cmd == "--name" && argc > 2) {
        kill_by_name(argv[2]);
    }
    else if (cmd == "--env") {
        kill_from_env();
    }
    else if (cmd == "--help") {
        cout << "Killer - process termination" << endl;
        cout << "Commands:" << endl;
        cout << "  --id PID      - terminate by ID" << endl;
        cout << "  --name NAME   - terminate all by name" << endl;
        cout << "  --env         - terminate from PROC_TO_KILL" << endl;
        cout << "  --help        - show this help" << endl;
        cout << endl;
        cout << "If started without commands, works with PROC_TO_KILL" << endl;
    }
    else {
        cout << "Unknown command" << endl;
        cout << "Use --help for usage information" << endl;
    }

    return 0;
}