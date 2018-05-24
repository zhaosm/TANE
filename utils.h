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

extern void readData(std::string fname, std::vector<std::vector<std::string>> &r);
extern void tane(std::vector<std::vector<std::string>> &r, std::vector<uint32_t> &result);
extern void computeDependencies(std::set<uint32_t> &L, std::map<uint32_t, uint32_t> &RHSCs, std::map<uint32_t, std::vector<std::vector<int>>> &partitions, std::vector<uint32_t> &deps);
extern void prune(std::set<uint32_t> &Ll, std::map<uint32_t, uint32_t> &RHSCs, std::map<uint32_t, std::vector<std::vector<int>>> &partitions, std::vector<uint32_t> &result);
extern void generateNextLevel(std::set<uint32_t> &Ll, std::set<uint32_t> &Lnext, std::map<uint32_t, std::vector<std::vector<int>>> &partitions);
extern void computeStrippedProduct(std::vector<std::vector<int>> &partition1, std::vector<std::vector<int>> &partition2, std::vector<std::vector<int>> &result);
extern int computeE(std::vector<std::vector<int>> &sub, std::vector<std::vector<int>> &sup);
extern void computeSingleAttributePartition(std::vector<std::vector<std::string>> &r, int attributeIndex, std::vector<std::vector<int>> &result);

#endif //TANE_UTILS_H
