//
// Created by 赵尚明 on 2018/5/24.
//
#include "utils.h"
#include <fstream>


std::vector<int> T(ROW_NUM, -1);
std::vector<std::vector<int>> S(ROW_NUM);

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

void tane(std::vector<std::vector<std::string>> &r, std::vector<uint32_t> &result) {
    int i, j;
    std::set<uint32_t> L;
    std::map<uint32_t, uint32_t> RHSCs;
    RHSCs[(uint32_t)0] = (uint32_t)4294967295;
    std::map<uint32_t, std::vector<std::vector<int>>> partitions;
    for (i = 0; i < ROW_NUM; i++) {
        uint32_t temp = (uint32_t)1 << i;
        L.insert(temp);
        partitions[temp] = std::vector<std::vector<int>>(0);
        computeSingleAttributePartition(r, i, partitions[temp]);
    }
    std::vector<uint32_t> deps;
    while (L.size() != 0) {
        computeDependencies(L, RHSCs, partitions, deps);
        prune(L, RHSCs, partitions, result);
        std::set<uint32_t> Lnext;
        generateNextLevel(L, Lnext, partitions);
        L = Lnext;
    }
}

void computeDependencies(std::set<uint32_t> &L, std::map<uint32_t, uint32_t> &RHSCs, std::map<uint32_t, std::vector<std::vector<int>>> &partitions, std::vector<uint32_t> &deps) {
    std::set<uint32_t>::iterator Lit, Litend = L.end();
    for (Lit = L.begin(); Lit != Litend; Lit++) {
        auto RHSCit = RHSCs.find(*Lit);
        RHSCit->second = (uint32_t)0;
        auto test = (uint32_t)1;
        while (test <= 32768) {
            auto temp = test ^ *Lit;
            if (temp != *Lit) {
                // found an including attribute
                RHSCit->second &=  RHSCs[temp];
            }
            test = test << 1;
        }
    }
    for (Lit = L.begin(); Lit != Litend; Lit++) {
        auto test = (uint32_t)1;
        while (test <= 32768) {
            auto RHSCit = RHSCs.find(*Lit);
            auto intersect = *Lit & RHSCit->second; // X intersect C+(X)
            auto temp = test ^ intersect;
            if (intersect != temp) { // A belongs to X intersect C+(X)
                temp = *Lit & test; // X \ { A }
                if (computeE(partitions[*Lit], partitions[temp]) == 0) {
                    // dependency
                    deps.push_back((*Lit << 16) | test);
                    RHSCit->second ^= test;
                    RHSCit->second &= *Lit;
                }
            }
            test = test << 1;
        }
    }
}

void prune(std::set<uint32_t> &Ll, std::map<uint32_t, uint32_t> &RHSCs, std::map<uint32_t, std::vector<std::vector<int>>> &partitions, std::vector<uint32_t> &result) {
    std::set<uint32_t>::iterator Lit, Lend = Ll.end();
    auto RHSCitend = RHSCs.end();
    for (Lit = Ll.begin(); Lit != Lend; Lit++) {
        auto RHSCit = RHSCs.find(*Lit);
        if (RHSCit == RHSCitend) {
            Lit = Ll.erase(Lit);
            Lend = Ll.end();
        }
        if (partitions[*Lit].size() == 0) {
            // X is a superkey
            auto test = (uint32_t)1;
            auto dif = RHSCs[*Lit] ^ *Lit; // C+(X) \ X
            while (test <= 32768) {
                if ((test ^ dif) != dif) {
                    // A belongs to dif
                    auto intersect = (uint32_t)65535;
                    auto test1 = (uint32_t)1;
                    while (test1 <= 32768) {
                        if ((test1 ^ *Lit) != *Lit) {
                            // B belongs to X
                            intersect &= RHSCs[(*Lit | test) ^ test1];
                        }
                        test1 = test1 << 1;
                    }
                    if ((test ^ intersect) != intersect) {
                        result.push_back((*Lit << 16) | test);
                    }
                }
                test = test << 1;
            }
        }
    }
}

void generateNextLevel(std::set<uint32_t> &Ll, std::set<uint32_t> &Lnext, std::map<uint32_t, std::vector<std::vector<int>>> &partitions) {
    Lnext.clear();
    std::set<uint32_t>::iterator Lit1, Lit2, Lend = Ll.end();
    for (Lit1 = Ll.begin(); Lit1 != Lend; Lit1++) {
        Lit2 = Lit1++;
        Lit1--;
        for (; Lit2 != Lend; Lit2++) {
            if (((*Lit1 & (*Lit1 - 1)) & (*Lit2 & (*Lit2 - 1))) == (*Lit1 & (*Lit1 - 1))) {
                // Y, Z belong to K, K belongs to PREFIX_BLOCKS(Ll)
                auto test = (uint32_t)1;
                auto x = *Lit1 | *Lit2;
                while (test <= 32768) {
                    auto temp = test ^ x; // X \ { A }
                    if (temp != x && Ll.find(temp) != Lend) {
                        // A belongs to X && X \ { A } belongs to Ll
                        Lnext.insert(x);
                        partitions[x] = std::vector<std::vector<int>>(0);
                        computeStrippedProduct(partitions[*Lit1], partitions[*Lit2], partitions[x]);
                    }
                    test = test << 1;
                }
            }
        }
    }
}

void computeStrippedProduct(std::vector<std::vector<int>> &partition1, std::vector<std::vector<int>> &partition2, std::vector<std::vector<int>> &result) {
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
                S[T[*vit]].push_back(*vit);
            }
        }
        for (vit = partition2[i].begin(); vit != vend; vit++) {
            auto temp = S[T[*vit]];
            if (temp.size() >= 2) {
                result.push_back(temp);
            }
            S[T[*vit]].clear();
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

int computeE(std::vector<std::vector<int>> &sub, std::vector<std::vector<int>> &sup) {
    auto it = sup.begin(), itend = sup.end();
    int e = 0;
    for (; it != itend; it++) {
        T[(*it)[0]] = (int)(*it).size();
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
        T[(*it)[0]] = -1;
    }
    return e;
}

void computeSingleAttributePartition(std::vector<std::vector<std::string>> &r, int attributeIndex, std::vector<std::vector<int>> &result) {
    std::map<std::string, int> values;
    int i;
    for (i = 0; i < ROW_NUM; i++) {
        auto it = values.find(r[i][attributeIndex]);
        if (it == values.end()) {
            std::vector<int> temp;
            temp.push_back(i);
            result.push_back(temp);
            values[r[i][attributeIndex]] = (int)result.size() - 1;
        } else {
            result[it->second].push_back(i);
        }
    }
}


