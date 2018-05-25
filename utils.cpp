//
// Created by 赵尚明 on 2018/5/24.
//
#include "utils.h"
#include <fstream>
#include <math.h>
#include <iostream>

std::vector<int> T(ROW_NUM, -1);
std::vector<std::vector<int>> S(0);
std::vector<std::vector<int>> combinations(32768);
std::vector<std::vector<std::vector<int>>> partitions(32768);
std::vector<uint32_t> RHSCs(32768, (uint32_t)0);

void readData(std::string fname, std::vector<std::vector<std::string>> &r) {
    std::ifstream fin(fname);
    std::string s;
    int i = 0, j, st, len;
    while (getline(fin, s)) {
        len = (int)s.length();
        st = 0;
        std::vector<std::string> temp;
        for (j = 0; j < len; j++) {
            if (s[j] == ',') {
                temp.push_back(s.substr(st, j - st));
                st = j + 1;
            }
        }
        temp.push_back(s.substr(st, j - st));
        r.push_back(temp);
        i++;
    }
}

void tane(std::vector<std::vector<std::string>> &r, std::set<uint32_t> &deps) {
    int i;
    std::set<uint32_t> L;
    RHSCs[(uint32_t)0] = (uint32_t)32767;
    for (i = 0; i < COL_NUM; i++) {
        uint32_t temp = (uint32_t)1 << (COL_NUM - i - 1);
        L.insert(temp);
        std::vector<int> attributes;
        attributes.push_back(i);
        combinations[temp] = attributes;
        partitions[temp] = std::vector<std::vector<int>>(0);
        computeSingleAttributePartition(r, i, partitions[temp]);
    }
    while (L.size() != 0) {
        // debug
        // int s = L.size();
        computeDependencies(L, deps);
        prune(L, deps);
        std::set<uint32_t> Lnext;
        generateNextLevel(L, Lnext);
        L = Lnext;
    }
}

void computeDependencies(std::set<uint32_t> &L, std::set<uint32_t> &deps) {
    std::set<uint32_t>::iterator Lit, Litend = L.end();
    for (Lit = L.begin(); Lit != Litend; Lit++) {
        RHSCs[*Lit] = (uint32_t)32767;
        auto ait = combinations[*Lit].begin(), aend = combinations[*Lit].end();
        for (; ait != aend; ait++) {
            RHSCs[*Lit] &= RHSCs[*Lit & ~((uint32_t)1 << (COL_NUM - *ait - 1))];
        }
    }
    for (Lit = L.begin(); Lit != Litend; Lit++) {
        auto intersect = *Lit & RHSCs[*Lit]; // X intersect C+(X)
        auto ait = combinations[*Lit].begin(), aend = combinations[*Lit].end();
        for (; ait != aend; ait++) {
            // an attribute in X
            auto test = (uint32_t)1 << (COL_NUM - *ait - 1);
            if (test == *Lit) continue;
            auto temp = (intersect & ~test) & intersect;
            if (intersect != temp) { // A belongs to X intersect C+(X)
                temp = *Lit & ~test; // X \ { A }
                if (partitions[temp].size() == partitions[*Lit].size()) {
                    // dependency
                    deps.insert((temp << 16) | test);
                    RHSCs[*Lit] &= ~test;
                    RHSCs[*Lit] &= *Lit;
                }
            }
        }
    }
}

void prune(std::set<uint32_t> &Ll, std::set<uint32_t> &deps) {
    std::set<uint32_t>::iterator Lit, Lend = Ll.end();
    for (Lit = Ll.begin(); Lit != Lend; ) {
        if (RHSCs[*Lit] == (uint32_t)0) {
            // C+(X) == empty set
            Lit = Ll.erase(Lit);
            Lend = Ll.end();
            continue;
        }
        if (partitions[*Lit].size() == 0) {
            // X is a superkey
            auto dif = RHSCs[*Lit] & ~*Lit; // C+(X) \ X
            auto i = (int)log2((double)dif);
            auto test = (uint32_t)1 << i;
            auto _dif = dif << (17 + COL_NUM - i - 1);
            while (_dif > (uint32_t)0) {
                if (test != *Lit) {
                    if ((dif & ~test) != dif) {
                        // A belongs to dif
                        auto intersect = (uint32_t)32767;
                        auto ait = combinations[*Lit].begin(), aend = combinations[*Lit].end();
                        for (; ait != aend; ait++) {
                            // pick an attribute from X
                            auto test1 = (uint32_t)1 << (COL_NUM - *ait - 1);
                            intersect &= RHSCs[(*Lit | test) & ~test1];
                        }
                        if ((intersect & ~test) != intersect) {
                            deps.insert((*Lit << 16) | test);
                        }
                    }
                }
                test = test >> 1;
                _dif = _dif << 1;
            }
        }
        Lit++;
    }
}

void generateNextLevel(std::set<uint32_t> &Ll, std::set<uint32_t> &Lnext) {
    Lnext.clear();
    std::set<uint32_t>::iterator Lit1, Lit2, Lend = Ll.end();
//    for (Lit1 = Ll.begin(); Lit1 != Lend; Lit1++) {
//        Lit2 = next(Lit1);
//        for (; Lit2 != Lend; Lit2++) {
//            if ((*Lit1 & (*Lit1 - 1)) == (*Lit2 & (*Lit2 - 1))) {
//                // Y, Z belong to K, K belongs to PREFIX_BLOCKS(Ll)
//                // debug
//                auto test = (uint32_t)1;
//                auto x = *Lit1 | *Lit2;
//                bool flag = true;
//                std::set<int> attributes;
//                while (test <= x) {
//                    auto temp = x & ~test; // X \ { A }
//                    if (temp != x) {
//                        // A belongs to X
//                        attributes.insert((int)log2((double)test));
//                        if (Ll.find(temp) == Lend) {
//                            // A belongs to X && X \ { A } not belongs to Ll
//                            flag = false;
//                            break;
//                        }
//                    }
//                    test = test << 1;
//                }
//                if (flag) {
//                    Lnext.insert(x);
//                    partitions[x] = std::vector<std::vector<int>>(0);
//                    computeStrippedProduct(partitions[*Lit1], partitions[*Lit2], partitions[x]);
//                    combinations[x] = attributes;
//                }
//            }
//        }
//    }
    std::set<std::set<uint32_t>> prefixBlocks;
    computePrefixBlocks(Ll, prefixBlocks);
    auto bit = prefixBlocks.begin(), bend = prefixBlocks.end();
    for (; bit != bend; bit++) {
        if ((*bit).size() < 2) continue;
        auto Lend1 = (*bit).end();
        for (Lit1 = (*bit).begin(); Lit1 != Lend1; Lit1++) {
            for (Lit2 = next(Lit1); Lit2 != Lend1; Lit2++) {
                // Y, Z belong to K, K belongs to PREFIX_BLOCKS(Ll)
                // debug
                auto x = *Lit1 | *Lit2;
                auto i = (int)log2((double)x);
                auto test = (uint32_t)1 << i;
                i = COL_NUM - i - 1;
                auto _x = x << (17 + i);
                bool flag = true;
                std::vector<int> attributes;
                while (_x > (uint32_t)0) {
                    auto temp = x & ~test; // X \ { A }
                    if (temp != x) {
                        // A belongs to X
                        attributes.push_back(i);
                        if (Ll.find(temp) == Lend) {
                            // A belongs to X && X \ { A } not belongs to Ll
                            flag = false;
                            break;
                        }
                    }
                    test = test >> 1;
                    i++;
                    _x = _x << 1;
                }
                if (flag) {
                    Lnext.insert(x);
                    computeStrippedProduct(partitions[*Lit1], partitions[*Lit2], partitions[x]);
                    combinations[x] = attributes;
                }
            }
        }
    }
}

void computeStrippedProduct(std::vector<std::vector<int>> &partition1, std::vector<std::vector<int>> &partition2, std::vector<std::vector<int>> &result) {
    result.clear();
    std::vector<int> emptyVec;
    int i, size1 = partition1.size(), size2 = partition2.size();
    for (i = 0;i < size1; i++) {
        auto vit = partition1[i].begin();
        auto vend = partition1[i].end();
        for (; vit != vend; vit++) {
            T[*vit] = i;
        }
        S.push_back(emptyVec);
    }
    for (i = 0; i < size2; i++) {
        auto vit = partition2[i].begin();
        auto vend = partition2[i].end();
        for (; vit != vend; vit++) {
            if (T[*vit] != -1) {
                S[T[*vit]].push_back(*vit);
            }
        }
        for (vit = partition2[i].begin(); vit != vend; vit++) {
            if (T[*vit] != -1) {
                auto temp = S[T[*vit]];
                if (temp.size() >= 2) {
                    result.push_back(temp);
                }
                S[T[*vit]].clear();
            }
        }
    }
    for (i = 0; i < size1; i++) {
        auto vit = partition1[i].begin();
        auto vend = partition1[i].end();
        for (; vit != vend; vit++) {
            T[*vit] = -1;
        }
    }
    S.clear();
}

void computeSingleAttributePartition(std::vector<std::vector<std::string>> &r, int attributeIndex, std::vector<std::vector<int>> &result) {
    result.clear();
    std::map<std::string, int> values;
    std::vector<std::set<int>> tempResult;
    std::vector<int> sizes;
    int i, k = 0;
    for (i = 0; i < ROW_NUM; i++) {
        auto it = values.find(r[i][attributeIndex]);
        if (it == values.end()) {
            std::set<int> temp;
            temp.insert(i);
            tempResult.push_back(temp);
            sizes.push_back(1);
            values[r[i][attributeIndex]] = k;
            k++;
        } else {
            tempResult[it->second].insert(i);
            sizes[it->second]++;
        }
    }
    auto it = tempResult.begin(), itend = tempResult.end();
    k = (int)sizes.size();
    std::vector<int> emptyVec;
    for (i = 0; i < k; i++) {
        if (sizes[i] > 1) {
            auto tempVec = emptyVec;
            auto rit = tempResult[i].begin(), rend = tempResult[i].end();
            for (; rit != rend; rit++) {
                tempVec.push_back(*rit);
            }
            result.push_back(tempVec);
        }
    }
}

void computePrefixBlocks(std::set<uint32_t> &Ll, std::set<std::set<uint32_t>> &result) {
    std::map<uint32_t, std::set<uint32_t>> tempResult;
    std::set<uint32_t> emptySet;
    auto it = Ll.begin(), itend = Ll.end();
    for (; it != itend; it++) {
        auto i = (int)log2((double)(*it));
        auto test = (uint32_t)1 << i;
        auto _it = *it << (17 + COL_NUM - i - 1);
        while (_it > (uint32_t)0) {
            auto dif = *it & ~test;
            if (dif != *it) {
                if (tempResult.find(dif) == tempResult.end()) {
                    tempResult[dif] = emptySet;
                }
                tempResult[dif].insert(*it);
                break;
            }
            test = test >> 1;
            _it = _it << 1;
        }
    }
    auto rit = tempResult.begin(), rend = tempResult.end();
    for (; rit != rend; rit++) {
        result.insert(rit->second);
    }
}
