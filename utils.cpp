//
// Created by 赵尚明 on 2018/5/24.
//
#include "utils.h"
#include <fstream>
#include <math.h>
#include <iostream>


std::vector<int> T(ROW_NUM, -1);
std::vector<std::set<int>> S(ROW_NUM);
std::map<uint32_t, std::set<int>> combinations;

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
    std::map<uint32_t, uint32_t> RHSCs;
    RHSCs[(uint32_t)0] = (uint32_t)32767;
    std::map<uint32_t, std::vector<std::set<int>>> partitions;
    for (i = 0; i < COL_NUM; i++) {
        uint32_t temp = (uint32_t)1 << i;
        L.insert(temp);
        std::set<int> attributes;
        attributes.insert(i);
        combinations[temp] = attributes;
        partitions[temp] = std::vector<std::set<int>>(0);
        computeSingleAttributePartition(r, i, partitions[temp]);
    }
    while (L.size() != 0) {
        // debug
        int s = L.size();
        computeDependencies(L, RHSCs, partitions, deps);
        prune(L, RHSCs, partitions, deps);
        std::set<uint32_t> Lnext;
        generateNextLevel(L, Lnext, partitions);
        L = Lnext;
    }
}

void computeDependencies(std::set<uint32_t> &L, std::map<uint32_t, uint32_t> &RHSCs, std::map<uint32_t, std::vector<std::set<int>>> &partitions, std::set<uint32_t> &deps) {
    std::set<uint32_t>::iterator Lit, Litend = L.end();
    for (Lit = L.begin(); Lit != Litend; Lit++) {
        RHSCs[*Lit] = (uint32_t)32767;
        auto attributes = combinations[*Lit];
        auto ait = attributes.begin(), aend = attributes.end();
        for (; ait != aend; ait++) {
            RHSCs[*Lit] &= RHSCs[*Lit & ~((uint32_t)1 << (*ait))];
        }
    }
    for (Lit = L.begin(); Lit != Litend; Lit++) {
        auto RHSCit = RHSCs.find(*Lit);
        auto intersect = *Lit & RHSCit->second; // X intersect C+(X)
        auto attributes = combinations[*Lit];
        auto ait = attributes.begin(), aend = attributes.end();
        for (; ait != aend; ait++) {
            // an attribute in X
            auto test = (uint32_t)1 << (*ait);
            if (test == *Lit) continue;
            auto temp = (intersect & ~test) & intersect;
            if (intersect != temp) { // A belongs to X intersect C+(X)
                temp = *Lit & ~test; // X \ { A }
                if (computeE(partitions[temp], partitions[*Lit]) == 0) {
                    // dependency
                    deps.insert((temp << 16) | test);
                    RHSCit->second = RHSCit->second & ~test;
                    RHSCit->second &= *Lit;
                }
            }
        }
    }
}

void prune(std::set<uint32_t> &Ll, std::map<uint32_t, uint32_t> &RHSCs, std::map<uint32_t, std::vector<std::set<int>>> &partitions, std::set<uint32_t> &deps) {
    std::set<uint32_t>::iterator Lit, Lend = Ll.end();
    for (Lit = Ll.begin(); Lit != Lend; ) {
        auto RHSCit = RHSCs.find(*Lit);
        if (RHSCit->second == (uint32_t)0) {
            // C+(X) == empty set
            Lit = Ll.erase(Lit);
            Lend = Ll.end();
            continue;
        }
        auto attributes = combinations[*Lit];
        if (partitions[*Lit].size() == 0) {
            // X is a superkey
            auto test = (uint32_t)1;
            auto dif = RHSCs[*Lit] & ~*Lit; // C+(X) \ X
            while (test <= dif) {
                if (test != *Lit) {
                    if ((dif & ~test) != dif) {
                        // A belongs to dif
                        auto intersect = (uint32_t)32767;
                        auto ait = attributes.begin(), aend = attributes.end();
                        for (; ait != aend; ait++) {
                            // pick an attribute from X
                            auto test1 = (uint32_t)1 << (*ait);
                            intersect &= RHSCs[(*Lit | test) & ~test1];
                        }
                        if ((intersect & ~test) != intersect) {
                            deps.insert((*Lit << 16) | test);
                        }
                    }
                }
                test = test << 1;
            }
        }
        Lit++;
    }
}

void generateNextLevel(std::set<uint32_t> &Ll, std::set<uint32_t> &Lnext, std::map<uint32_t, std::vector<std::set<int>>> &partitions) {
    Lnext.clear();
    std::set<uint32_t>::iterator Lit1, Lit2, Lend = Ll.end();
    for (Lit1 = Ll.begin(); Lit1 != Lend; Lit1++) {
        Lit2 = ++Lit1;
        Lit1--;
        for (; Lit2 != Lend; Lit2++) {
            if ((*Lit1 & (*Lit1 - 1)) == (*Lit2 & (*Lit2 - 1))) {
                // Y, Z belong to K, K belongs to PREFIX_BLOCKS(Ll)
                // debug
                auto test = (uint32_t)1;
                auto x = *Lit1 | *Lit2;
                bool flag = true;
                std::set<int> attributes;
                while (test <= x) {
                    auto temp = x & ~test; // X \ { A }
                    if (temp != x) {
                        // A belongs to X
                        attributes.insert((int)log2((double)test));
                        if (Ll.find(temp) == Lend) {
                            // A belongs to X && X \ { A } not belongs to Ll
                            flag = false;
                            break;
                        }
                    }
                    test = test << 1;
                }
                if (flag) {
                    Lnext.insert(x);
                    partitions[x] = std::vector<std::set<int>>(0);
                    computeStrippedProduct(partitions[*Lit1], partitions[*Lit2], partitions[x]);
                    combinations[x] = attributes;
                }
            }
        }
    }
}

void computeStrippedProduct(std::vector<std::set<int>> &partition1, std::vector<std::set<int>> &partition2, std::vector<std::set<int>> &result) {
    result.clear();
    int i, size1 = partition1.size(), size2 = partition2.size();
    for (i = 0;i < size1; i++) {
        auto vit = partition1[i].begin();
        auto vend = partition1[i].end();
        for (; vit != vend; vit++) {
            T[*vit] = i;
            S[i].clear();
        }
    }
    for (i = 0; i < size2; i++) {
        auto vit = partition2[i].begin();
        auto vend = partition2[i].end();
        for (; vit != vend; vit++) {
            if (T[*vit] != -1) {
                S[T[*vit]].insert(*vit);
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
}

int computeE(std::vector<std::set<int>> &sub, std::vector<std::set<int>> &sup) {
    auto it = sup.begin(), itend = sup.end();
    int e = 0;
    for (; it != itend; it++) {
        auto tempit = (*it).begin();
        T[*tempit] = (int)(*it).size();
    }
    itend = sub.end();
    for (it = sub.begin(); it != itend; it++) {
        int m = 1;
        auto it1 = (*it).begin(), itend1 = (*it).end();
        for (; it1 != itend1; it1++) {
            m = m > T[*it1] ? m : T[*it1];
        }
        e = e + (int)(*it).size() - m;
    }
    itend = sup.end();
    for (it = sup.begin(); it != itend; it++) {
        auto tempit = (*it).begin();
        T[*tempit] = -1;
    }
    return e;
}

void computeSingleAttributePartition(std::vector<std::vector<std::string>> &r, int attributeIndex, std::vector<std::set<int>> &result) {
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
    for (i = 0; i < k; i++) {
        if (sizes[i] > 1) {
            result.push_back(tempResult[i]);
        }
    }
}
