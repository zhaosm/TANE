//
// Created by 赵尚明 on 2018/5/24.
//
#include "utils.h"




void tane(std::string (&r)[ROW_NUM][COL_NUM], std::vector<uint32_t> &result) {
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
        auto xe = computeE(partitions[*Lit]);
        while (test <= 32768) {
            auto RHSCit = RHSCs.find(*Lit);
            auto intersect = *Lit & RHSCit->second; // X intersect C+(X)
            auto temp = test ^ intersect;
            if (intersect != temp) { // A belongs to X intersect C+(X)
                temp = *Lit & test; // X \ { A }
                if (computeE(partitions[temp]) == xe) {
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
            if ((*Lit1 & (*Lit1 - 1)) & (*Lit2 & (*Lit2 - 1)) == (*Lit1 & (*Lit1 - 1))) {
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

}

