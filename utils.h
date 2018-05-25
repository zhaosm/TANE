//
// Created by 赵尚明 on 2018/5/24.
//

#ifndef TANE_UTILS_H
#define TANE_UTILS_H

#define ROW_NUM 99918
#define COL_NUM 15

#include <vector>
#include <string>
#include <map>
#include <set>

extern std::vector<int> T;
extern std::vector<std::vector<int>> S;
extern std::vector<std::vector<int>> combinations;
extern std::vector<std::vector<std::vector<int>>> partitions;
extern std::vector<uint32_t> RHSCs;

extern void readData(std::string fname, std::vector<std::vector<std::string>> &r);
extern void tane(std::vector<std::vector<std::string>> &r, std::set<uint32_t> &deps);
extern void computeDependencies(std::set<uint32_t> &L, std::set<uint32_t> &deps);
extern void prune(std::set<uint32_t> &Ll, std::set<uint32_t> &deps);
extern void generateNextLevel(std::set<uint32_t> &Ll, std::set<uint32_t> &Lnext);
extern void computeStrippedProduct(std::vector<std::vector<int>> &partition1, std::vector<std::vector<int>> &partition2, std::vector<std::vector<int>> &result);
extern void computeSingleAttributePartition(std::vector<std::vector<std::string>> &r, int attributeIndex, std::vector<std::vector<int>> &result);
extern void computePrefixBlocks(std::set<uint32_t> &Ll, std::set<std::set<uint32_t>> &result);

#endif //TANE_UTILS_H
