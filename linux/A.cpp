#include <iostream>
#include <sstream>
#include <vector>

int main() {
    const int N = 9;

    std::string line;
    if (!std::getline(std::cin, line)) {
        return 0;
    }

    std::istringstream iss(line);
    int x;
    std::vector<int> numbers;

    while (iss >> x) {
        numbers.push_back(x);
    }

    if (iss.fail() && !iss.eof()) {
        std::cerr << "Ошибка: некорректный ввод\n";
        return 1;
    }

    for (size_t i = 0; i < numbers.size(); ++i) {
        int y = numbers[i] + N;
        if (i > 0) std::cout << ' ';
        std::cout << y;
    }
    std::cout << std::endl;
    return 0;
}
