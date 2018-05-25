#include <iostream>
#include <fstream>
#include "utils.h"
#include <time.h>

int main() {
    double start, stop;
    start = clock();
    std::string fname = "../data/data.txt";
    std::string ofname = "../data/result.txt";
    std::vector<std::vector<std::string>> r;
    std::ofstream fout(ofname);
    readData(fname, r);
    std::set<uint32_t> deps;
    tane(r, deps);
    auto it = deps.rbegin(), itend = deps.rend();
    int i = 0;
    for (; it != itend; it++) {
        i = 0;
        uint32_t temp = *it & (uint32_t)4294901760;
        while (i <= 15) {
            if (temp & (uint32_t)2147483648) {
                fout << i << " ";
            }
            i++;
            temp = temp << 1;
        }
        fout << "-> ";
        temp = *it << 16;
        i = 0;
        while (i <= 15) {
            if (temp & (uint32_t)2147483648) {
                fout << i << " ";
            }
            i++;
            temp = temp << 1;
        }
        fout << "\n";
    }
    stop = clock();
    std::cout << "Time: " << ((double)(stop - start)) / CLOCKS_PER_SEC << "s" << std::endl;
    return 0;
}