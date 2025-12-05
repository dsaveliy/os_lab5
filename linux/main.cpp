#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <string>
#include <cstring>

pid_t startProcess(const char* path, int stdinFd, int stdoutFd) {
    pid_t pid = fork();
    
    if (pid < 0) {
        std::cerr << "Ошибка fork для " << path << "\n";
        return -1;
    }
    
    // дочерний процесс
    if (pid == 0) {
        // перенаправляем stdin на stdinFd
        if (dup2(stdinFd, STDIN_FILENO) == -1) {
            std::cerr << "Ошибка dup2 для stdin в " << path << "\n";
            _exit(1);
        }
        
        // перенаправляем stdout на stdoutFd
        if (dup2(stdoutFd, STDOUT_FILENO) == -1) {
            std::cerr << "Ошибка dup2 для stdout в " << path << "\n";
            _exit(1);
        }
        
        // закрываем оригинальные дескрипторы (они уже продублированы)
        close(stdinFd);
        close(stdoutFd);
        
        // запускаем программу
        execl(path, path, nullptr);
        
        // если execl вернулся, значит ошибка
        std::cerr << "Ошибка execl для " << path << ": " << strerror(errno) << "\n";
        _exit(1);
    }
    
    // родительский процесс возвращает PID дочернего
    return pid;
}

int main() {
    // логика аналогична
    
    int pipe0[2]; // Main -> M
    if (pipe(pipe0) == -1) return 1;
    
    int pipe1[2]; // M -> A 
    if (pipe(pipe1) == -1) return 1;
    pid_t pidM = startProcess("./M", pipe0[0], pipe1[1]);
    if (pidM == -1) return 1;
    close(pipe0[0]);
    close(pipe1[1]);
    
    int pipe2[2]; // A -> P
    if (pipe(pipe2) == -1) return 1;
    pid_t pidA = startProcess("./A", pipe1[0], pipe2[1]);
    if (pidA == -1) return 1;
    close(pipe1[0]);
    close(pipe2[1]);
    
    int pipe3[2]; // P -> S
    if (pipe(pipe3) == -1) return 1;
    pid_t pidP = startProcess("./P", pipe2[0], pipe3[1]);
    if (pidP == -1) return 1;
    close(pipe2[0]);
    close(pipe3[1]);

    int pipe4[2]; // S -> Main
    if (pipe(pipe4) == -1) return 1;
    pid_t pidS = startProcess("./S", pipe3[0], pipe4[1]);
    if (pidS == -1) return 1;
    close(pipe3[0]);
    close(pipe4[1]);
    
    // теперь у Main остались только:
    // pipe0[1] для записи в stdin M
    // pipe4[0] для чтения из stdout S
    
    // читаем пользовательский ввод
    std::string data;
    std::cout << "Введите числа через пробел: ";
    std::getline(std::cin, data);
    data.push_back('\n');
    
    // пишем в stdin первого процесса
    ssize_t written = write(pipe0[1], data.c_str(), data.size());
    if (written == -1) {
        std::cerr << "Ошибка записи в pipe0\n";
    }
    close(pipe0[1]);
    
    // читаем результат из stdout последнего процесса
    std::string result;
    char buffer[256];
    ssize_t n;
    while ((n = read(pipe4[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        result += buffer;
    }
    if (n == -1) {
        std::cerr << "Ошибка чтения из pipe4\n";
    }
    close(pipe4[0]);
    
    int status;
    waitpid(pidM, &status, 0);
    waitpid(pidA, &status, 0);
    waitpid(pidP, &status, 0);
    waitpid(pidS, &status, 0);

    if (result.empty()) {
        std::cerr << "Цепочка обработки завершилась с ошибкой (нет результата)\n";
        return 1;
    } else {
        std::cout << "Результат: " << result;
    }
    
    return 0;
}