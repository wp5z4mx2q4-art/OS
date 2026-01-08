#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <dirent.h>  
#include <csignal>   
#include <unistd.h>  
#include <sys/types.h>
#include <cstdlib>  

using namespace std;

void kill_by_id(int pid) {
    if (kill(pid, SIGKILL) == 0) {
        cout << "Process " << pid << " terminated" << endl;
    } else {
        perror(("Error terminating process " + to_string(pid)).c_str());
    }
}

vector<int> find_by_name(string name) {
    vector<int> result;
    DIR* dir;
    struct dirent* ent;

    string name_lower = name;
    for (char& c : name_lower) c = tolower(c);

    if ((dir = opendir("/proc")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            string pid_str = ent->d_name;
            if (!all_of(pid_str.begin(), pid_str.end(), ::isdigit)) {
                continue;
            }

            string comm_path = "/proc/" + pid_str + "/comm";
            ifstream comm_file(comm_path);
            if (comm_file.is_open()) {
                string exe_name;
                getline(comm_file, exe_name);
                
                if (!exe_name.empty() && exe_name.back() == '\n') {
                    exe_name.pop_back();
                }

                string exe_lower = exe_name;
                for (char& c : exe_lower) c = tolower(c);

                if (exe_lower == name_lower) {
                    result.push_back(stoi(pid_str));
                }
                comm_file.close();
            }
        }
        closedir(dir);
    }
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
    const char* env_val = getenv("PROC_TO_KILL");
    if (env_val == NULL) return result;

    string s = env_val;
    string item;
    for (char c : s) {
        if (c == ',') {
            while (!item.empty() && (item[0] == ' ' || item[0] == '\t' || item[0] == '"')) item.erase(0, 1);
            while (!item.empty() && (item.back() == ' ' || item.back() == '\t' || item.back() == '"')) item.pop_back();
            if (!item.empty()) result.push_back(item);
            item.clear();
        } else {
            item += c;
        }
    }
    if (!item.empty()) {
        while (!item.empty() && (item[0] == ' ' || item[0] == '\t' || item[0] == '"')) item.erase(0, 1);
        while (!item.empty() && (item.back() == ' ' || item.back() == '\t' || item.back() == '"')) item.pop_back();
        if (!item.empty()) result.push_back(item);
    }
    return result;
}

void kill_from_env() {
    cout << "=== Processing PROC_TO_KILL ===" << endl;
    vector<string> procs = get_env_processes();
    if (procs.empty()) {
        cout << "No processes to terminate from ENV" << endl;
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
            kill_by_id(stoi(argv[2]));
        } catch (...) {
            cout << "Error: invalid ID" << endl;
        }
    }
    else if (cmd == "--name" && argc > 2) {
        kill_by_name(argv[2]);
    }
    else if (cmd == "--env") {
        kill_from_env();
    }
    else {
        cout << "Unknown command. Use --id, --name or --env" << endl;
    }
    return 0;
}
