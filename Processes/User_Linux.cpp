#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <vector>

using namespace std;

bool process_exists(int pid) {
    if (pid <= 0) return false;
    return (kill(pid, 0) == 0);
}

bool process_exists(string name) {
    string cmd = "pgrep -x " + name + " > /dev/null 2>&1";
    int res = system(cmd.c_str());
    return (WEXITSTATUS(res) == 0);
}

int start_proc(string program, string arg) {
    pid_t pid = fork();
    if (pid == -1) return 0;
    if (pid == 0) {
        execlp(program.c_str(), program.c_str(), arg.c_str(), (char*)NULL);
        exit(1); 
    } else {
        usleep(100000); 
        return pid;
    }
}

int main() {
    cout << "User Application (Linux) - Testing Killer" << endl;
    cout << "=========================================" << endl << endl;

    setenv("PROC_TO_KILL", "sleep", 1);
    cout << "Set PROC_TO_KILL = sleep" << endl << endl;

    cout << "Test 1: ENV Check" << endl;
    int pid1 = start_proc("sleep", "1000"); 
    if (!pid1) return 1;

    cout << "Before: sleep process running? " << (process_exists(pid1) ? "yes" : "no") << endl;
    system("./killer"); 
    sleep(1);
    cout << "After: sleep process running? " << (process_exists(pid1) ? "yes" : "no") << endl << endl;

    cout << "Test 2: --id parameter" << endl;
    int pid3 = start_proc("sleep", "500");
    cout << "Started sleep PID: " << pid3 << endl;
    cout << "Before: exists? " << (process_exists(pid3) ? "yes" : "no") << endl;
    system(("./killer --id " + to_string(pid3)).c_str());
    sleep(1);
    cout << "After: exists? " << (process_exists(pid3) ? "yes" : "no") << endl << endl;

    cout << "Test 3: --name parameter" << endl;
    start_proc("sleep", "600");
    cout << "Before: sleep exists? " << (process_exists("sleep") ? "yes" : "no") << endl;
    system("./killer --name sleep");
    sleep(1);
    cout << "After: sleep exists? " << (process_exists("sleep") ? "yes" : "no") << endl << endl;

    unsetenv("PROC_TO_KILL");
    cout << "Removed PROC_TO_KILL. Testing complete." << endl;

    return 0;
}
