#include <iostream>
#include "utils.h"

int main() {
    std::string fname = "../data/data.txt";
    std::vector<std::vector<std::string>> r;
    readData(fname, r);
    std::set<uint32_t> deps;
    tane(r, deps);
    auto it = deps.begin(), itend = deps.end();
    for (; it != itend; it++) {
        std::cout << *it << '\n';
    }
    return 0;
}