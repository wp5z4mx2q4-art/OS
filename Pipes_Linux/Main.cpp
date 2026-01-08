#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

pid_t RunProgram(const char* program, int in_fd, int out_fd) {
    pid_t pid = fork();

    if (pid == 0) {
        if (in_fd != STDIN_FILENO) {
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }
        if (out_fd != STDOUT_FILENO) {
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }

        std::string path = "./" + std::string(program);
        execl(path.c_str(), program, nullptr);
        exit(1);
    } else if (pid < 0) {
        return 0;
    }

    return pid;
}

int main() {
    std::cout << "Enter 4 numbers: ";
    std::string data;
    if (!std::getline(std::cin, data)) return 0;

    data += "\n";
    std::cout << "Output: " << std::flush;

    int pipe_mainM[2], pipe_mA[2], pipe_aP[2], pipe_pS[2];

    if (pipe(pipe_mainM) || pipe(pipe_mA) || pipe(pipe_aP) || pipe(pipe_pS)) {
        return 1;
    }

    write(pipe_mainM[1], data.c_str(), data.length());
    close(pipe_mainM[1]);

    pid_t mPid = RunProgram("M", pipe_mainM[0], pipe_mA[1]);
    close(pipe_mainM[0]); close(pipe_mA[1]);
    waitpid(mPid, nullptr, 0);

    pid_t aPid = RunProgram("A", pipe_mA[0], pipe_aP[1]);
    close(pipe_mA[0]); close(pipe_aP[1]);
    waitpid(aPid, nullptr, 0);

    pid_t pPid = RunProgram("P", pipe_aP[0], pipe_pS[1]);
    close(pipe_aP[0]); close(pipe_pS[1]);
    waitpid(pPid, nullptr, 0);

    pid_t sPid = RunProgram("S", pipe_pS[0], STDOUT_FILENO);
    close(pipe_pS[0]);
    waitpid(sPid, nullptr, 0);

    std::cout << std::endl;
    std::cout << "Press Enter to exit...";
    std::cin.get(); 

    return 0;
}
