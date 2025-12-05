#include <iostream>
#include <sstream>

int main() {
    std::setlocale(LC_ALL, "Russian");

    std::string line;
    if (!std::getline(std::cin, line)) {
        return 0;
    }

    std::istringstream iss(line);
    long long x;
    long long sum = 0;
    bool hasNumbers = false;

    while (iss >> x) {
        sum += x;
        hasNumbers = true;
    }

    if (!iss.eof()) {
        std::cerr << "Ошибка: некорректный ввод\n";
        return 1;
    }

    if (!hasNumbers) {
        std::cerr << "Предупреждение: процесс S не получил ни одного числа\n";
    }
    std::cout << sum << std::endl;
    return 0;
}
