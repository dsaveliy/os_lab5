#include <windows.h>
#include <iostream>
#include <string>

// вспомогательная функция для запуска процесса с заданными stdin/stdout
bool startProcess(const std::wstring& cmd, HANDLE hStdIn,
    HANDLE hStdOut, PROCESS_INFORMATION& pi) {
    STARTUPINFOW si; // структура для параметров запуска
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES; // говорим использовать мои переданные хендлы потоков
    si.hStdInput = hStdIn;
    si.hStdOutput = hStdOut;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE); // но ошибки в общую консоль
    ZeroMemory(&pi, sizeof(pi));

    BOOL ok = CreateProcessW(
        NULL,
        const_cast<wchar_t*>(cmd.c_str()),
        NULL,
        NULL,
        TRUE, // bInheritHandles - делаем хендлы наследуемыми
        0,
        NULL,
        NULL,
        &si,
        &pi
    );

    return ok == TRUE;
}

int main() {
    setlocale(LC_ALL, "Russian");
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE; // делаем хендлы наследуемыми

    // переменные для хранения информации о процессах
    PROCESS_INFORMATION piM, piA, piP, piS;

    // будем пошагвоо создвать и использовать нужные хендлы (а как следствие и пайпы),
    // пошаговость нужна, чтоб все процессы не наследовали лишние потоки, т.к. bInheritHandle = TRUE;
    HANDLE r0, w0; // Main -> M
    if (!CreatePipe(&r0, &w0, &sa, 0)) return 1;
    SetHandleInformation(w0, HANDLE_FLAG_INHERIT, 0); // не наследуемый

    HANDLE r1, w1; // M -> A
    if (!CreatePipe(&r1, &w1, &sa, 0)) return 1;
    if (!startProcess(L"M.exe", r0, w1, piM)) return 1;
    CloseHandle(r0);
    CloseHandle(w1);

    HANDLE r2, w2; // A -> P
    if (!CreatePipe(&r2, &w2, &sa, 0)) return 1;
    if (!startProcess(L"A.exe", r1, w2, piA)) return 1;
    CloseHandle(r1);
    CloseHandle(w2);

    HANDLE r3, w3; // P -> S
    if (!CreatePipe(&r3, &w3, &sa, 0)) return 1;
    if (!startProcess(L"P.exe", r2, w3, piP)) return 1;
    CloseHandle(r2);
    CloseHandle(w3);

    HANDLE r4, w4; // S -> Main
    if (!CreatePipe(&r4, &w4, &sa, 0)) return 1;
    if (!startProcess(L"S.exe", r3, w4, piS)) return 1;
    CloseHandle(r3);
    CloseHandle(w4);
    SetHandleInformation(r4, HANDLE_FLAG_INHERIT, 0); // не наследуемый

    // читаем пользовательский ввод
    std::string data;
    std::cout << "Введите числа через пробел: ";
    std::getline(std::cin, data);
    data.push_back('\n');

    // переносим пользовательский ввод в хендл потока ввода первого процесса
    DWORD written = 0;
    if (!WriteFile(w0, data.c_str(), (DWORD)data.size(), &written, NULL)) {
        std::cerr << "Не удалось записать данные\n";
    }
    CloseHandle(w0);

    // читаем вывод последнего процесса назад в Main
    // будет просто создавать временный буфер на 255 байт+символ конца строки, но
    // если что циклом будем перезаписывать его и добавлять невместившися байты в результат
    std::string result;
    char buffer[256];
    DWORD readBytes = 0;

    while (ReadFile(r4, buffer, sizeof(buffer) - 1, &readBytes, NULL) && readBytes > 0) {
        buffer[readBytes] = '\0';
        result += buffer;
    }
    CloseHandle(r4);

    WaitForSingleObject(piM.hProcess, INFINITE);
    WaitForSingleObject(piA.hProcess, INFINITE);
    WaitForSingleObject(piP.hProcess, INFINITE);
    WaitForSingleObject(piS.hProcess, INFINITE);

    if (result.empty()) {
        std::cerr << "Цепочка обработки завершилась с ошибкой (нет результата)\n";
    }
    else {
        std::cout << "Результат: " << result;
    }

    CloseHandle(piM.hProcess);
    CloseHandle(piM.hThread);
    CloseHandle(piA.hProcess);
    CloseHandle(piA.hThread);
    CloseHandle(piP.hProcess);
    CloseHandle(piP.hThread);
    CloseHandle(piS.hProcess);
    CloseHandle(piS.hThread);

    return 0;
}
