#pragma once

#include <unordered_map>
#include <vector>

#include "Types.hpp"
#include "Cache.hpp"



class Memory {
    std::unordered_map<u32, u32> storage;

    Cache<8> l1;

    Cache<16> l2;
public:
    unsigned misses;

    unsigned hits;

    Memory();

    void Assign(std::unordered_map<u32, u32>&& memory);

    std::vector<std::pair<u32, u32>> AsVector();

    u32 Get(u32 address);

    void Set(u32 address, u32 val);

    bool FetchInstruction(u32 address, u32& out);
};
