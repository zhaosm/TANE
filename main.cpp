#include <iostream>
#include <fstream>
#include <time.h>
#include "TANE.h"

int main() {
    double start, stop;
    start = clock();
    std::string fname = "../data/data.txt";
    std::string ofname = "../data/result.txt";
    std::vector<std::vector<std::string>> r;
    std::ofstream fout(ofname);
    TANE tane;
    tane.readData(fname);
    tane.tane();
    auto it = tane.deps.rbegin(), itend = tane.deps.rend();
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
                fout << i;
                break;
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