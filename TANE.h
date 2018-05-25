//
// Created by 赵尚明 on 2018/5/25.
//

#ifndef TANE_TANE_H
#define TANE_TANE_H

#define ROW_NUM 99918
#define COL_NUM 15

#include <vector>
#include <string>
#include <map>
#include <set>

class TANE {
public:
    std::vector<int> T;
    std::vector<std::vector<int>> S;
    std::vector<std::vector<int>> combinations;
    std::vector<std::vector<std::vector<int>>> partitions;
    std::vector<uint32_t> RHSCs;
    std::vector<std::vector<std::string>> r;
    std::set<uint32_t> deps;
    std::set<uint32_t> L;

public:
    TANE ();

public:
    void readData(std::string fname);
    void tane();
    void computeDependencies(std::set<uint32_t> &L);
    void prune(std::set<uint32_t> &Ll);
    void generateNextLevel(std::set<uint32_t> &Ll, std::set<uint32_t> &Lnext);
    void computeStrippedProduct(std::vector<std::vector<int>> &partition1, std::vector<std::vector<int>> &partition2, std::vector<std::vector<int>> &result);
    void computeSingleAttributePartition(int attributeIndex, std::vector<std::vector<int>> &result);
    void computePrefixBlocks(std::set<uint32_t> &Ll, std::set<std::set<uint32_t>> &result);
};

#endif //TANE_TANE_H
