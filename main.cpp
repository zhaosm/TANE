#include <iostream>
#include "utils.h"

int main() {
    std::string fname = "../data/data.txt";
    std::vector<std::vector<std::string>> r;
    readData(fname, r);
    std::vector<uint32_t> result;
    tane(r, result);
    auto it = result.begin(), itend = result.end();
    for (; it != itend; it++) {
        std::cout << *it << '\n';
    }
    return 0;
}