//
// Created by 赵尚明 on 2018/5/25.
//
#include "TANE.h"
#include <fstream>
#include <math.h>
#include <iostream>


TANE::TANE () {
    T =  std::vector<int>(ROW_NUM, -1);
    partitions = std::vector<std::vector<std::vector<int>>>(32768);
    RHSCs = std::vector<uint32_t>(32768, (uint32_t)0);
}

void TANE::readData(std::string fname) {
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

void TANE::tane() {
    int i;
    std::set<uint32_t> L;
    RHSCs[(uint32_t)0] = (uint32_t)32767;
    for (i = 0; i < COL_NUM; i++) {
        uint32_t temp = (uint32_t)1 << (COL_NUM - i - 1);
        L.insert(temp);
        partitions[temp] = std::vector<std::vector<int>>(0);
        computeSingleAttributePartition(i, partitions[temp]);
    }
    while (L.size() != 0) {
        computeDependencies(L);
        prune(L);
        std::set<uint32_t> Lnext;
        generateNextLevel(L, Lnext);
        L = Lnext;
    }
}

void TANE::computeDependencies(std::set<uint32_t> &L) {
    std::set<uint32_t>::iterator Lit, Litend = L.end();
    for (Lit = L.begin(); Lit != Litend; Lit++) {
        RHSCs[*Lit] = (uint32_t)32767;
        auto test = (uint32_t)1;
        while (test <= *Lit) {
            if ((*Lit & ~test) != *Lit) {
                RHSCs[*Lit] &= RHSCs[*Lit & ~test];
            }
            test = test << 1;
        }
    }
    for (Lit = L.begin(); Lit != Litend; Lit++) {
        auto intersect = *Lit & RHSCs[*Lit]; // X intersect C+(X)
        auto test = (uint32_t)1;
        while (test <= intersect) {
            if ((intersect & ~test) != intersect) {
                // an attribute in X intersect C+{X}
                if (test != *Lit) {
                    auto temp = *Lit & ~test; // X \ { A }
                    if (partitions[temp].size() == partitions[*Lit].size()) {
                        // dependency
                        deps.insert((temp << 16) | test);
                        RHSCs[*Lit] &= ~test;
                        RHSCs[*Lit] &= *Lit;
                    }
                }
            }
            test = test << 1;
        }
    }
}

void TANE::prune(std::set<uint32_t> &Ll) {
    std::set<uint32_t>::iterator Lit, Lend = Ll.end();
    for (Lit = Ll.begin(); Lit != Lend; ) {
        if (RHSCs[*Lit] == (uint32_t)0) {
            // C+(X) == empty set
            Lit = Ll.erase(Lit);
            Lend = Ll.end();
            continue;
        }
        Lit++;
    }
}

void TANE::generateNextLevel(std::set<uint32_t> &Ll, std::set<uint32_t> &Lnext) {
    Lnext.clear();
    std::set<uint32_t>::iterator Lend = Ll.end();
    std::vector<uint32_t>::iterator Lit1, Lit2;
    std::vector<std::vector<uint32_t>> prefixBlocks;
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
                // i = COL_NUM - i - 1;
                auto _x = x << (17 + COL_NUM - i - 1);
                bool flag = true;
                while (_x > (uint32_t)0) {
                    auto temp = x & ~test; // X \ { A }
                    if (temp != x) {
                        // A belongs to X
                        if (Ll.find(temp) == Lend) {
                            // A belongs to X && X \ { A } not belongs to Ll
                            flag = false;
                            break;
                        }
                    }
                    test = test >> 1;
                    _x = _x << 1;
                }
                if (flag) {
                    Lnext.insert(x);
                    computeStrippedProduct(partitions[*Lit1], partitions[*Lit2], partitions[x]);
                }
            }
        }
    }
}

void TANE::computeStrippedProduct(std::vector<std::vector<int>> &partition1, std::vector<std::vector<int>> &partition2, std::vector<std::vector<int>> &result) {
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

void TANE::computeSingleAttributePartition(int attributeIndex, std::vector<std::vector<int>> &result) {
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

void TANE::computePrefixBlocks(std::set<uint32_t> &Ll, std::vector<std::vector<uint32_t>> &result) {
    std::map<uint32_t, std::vector<uint32_t>> tempResult;
    std::vector<uint32_t> emptyVec;
    auto it = Ll.begin(), itend = Ll.end();
    for (; it != itend; it++) {
        auto i = (int)log2((double)(*it));
        auto test = (uint32_t)1 << i;
        auto _it = *it << (17 + COL_NUM - i - 1);
        while (_it > (uint32_t)0) {
            auto dif = *it & ~test;
            if (dif != *it) {
                if (tempResult.find(dif) == tempResult.end()) {
                    tempResult[dif] = emptyVec;
                }
                tempResult[dif].push_back(*it);
                break;
            }
            test = test >> 1;
            _it = _it << 1;
        }
    }
    auto rit = tempResult.begin(), rend = tempResult.end();
    for (; rit != rend; rit++) {
        result.push_back(rit->second);
    }
}
