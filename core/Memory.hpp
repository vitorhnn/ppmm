#pragma once

#include <unordered_map>
#include <vector>

#include "Types.hpp"
#include "Cache.hpp"



class Memory {
    std::unordered_map<u32, u32> storage;

    Cache<4> L1;
public:
    Memory();

    void Assign(std::unordered_map<u32, u32> memory);

    std::vector<std::pair<u32, u32>> AsVector();

    u32 Get(u32 address);

    void Set(u32 address, u32 val);

    bool MaybeGet(u32 address, u32& out);
};
