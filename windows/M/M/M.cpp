#include <iostream>
#include <sstream>
#include <vector>

int main() {
    std::setlocale(LC_ALL, "Russian");

    std::string line;
    if (!std::getline(std::cin, line)) {
        return 0;
    }

    std::istringstream iss(line);
    std::vector<int> numbers;
    int x;

    while (iss >> x) {
        numbers.push_back(x);
    }

    if (!iss.eof()) {
        std::cerr << "Ошибка: некорректный ввод\n";
        return 1;
    }

    for (size_t i = 0; i < numbers.size(); ++i) {
        int y = numbers[i] * 7;
        if (i > 0) std::cout << ' ';
        std::cout << y;
    }
    std::cout << std::endl;
    return 0;
}
