#pragma once

#include <cstddef>
#include <array>
#include <bitset>
#include <cmath>

#include "Types.hpp"
#include "Memory.hpp"

template <size_t tagsz>
struct Block {
    bool valid;
    std::bitset<tagsz> tag;
    u32 data;
};

constexpr bool is_powerof2(int v) {
    return v && ((v & (v - 1)) == 0);
}

constexpr int clog2(int v)
{
    int ret = 0;
    while (v >>= 1) {
        ++ret;
    }

    return ret;
}

// size gets used as 2^size
template <u32 size>
class Cache {
    static_assert(is_powerof2(size), "size should be a power of two");

    static const u32 tag_size = 32 - static_cast<u32>(clog2(size)) - 2;

    std::array<Block<tag_size>, size> storage;

    const std::unordered_map<u32, u32>& memory;
public:
    explicit Cache(const std::unordered_map<u32, u32>& memory) : memory(memory)
    {
        Invalidate();
    }

    void Invalidate()
    {
        for (auto& block : storage) {
            block.valid = false;
        }
    }

    bool Get(u32 address, u32& out)
    {
        u32 pos = address % size;
        auto tag = std::bitset<tag_size>(address >> 32 - tag_size);

        auto& block = storage[pos];

        if (block.tag == tag && block.valid) {
            // HIT
            out = block.data;

            return true;
        } else {
            // miss, bring the data from memory and set valid and tag
            block.valid = true;
            block.tag = tag;
            block.data = memory.at(address);

            return false;
        }
    }

    bool Set(u32 address, u32 val)
    {
        u32 pos = address % size;
        auto tag = std::bitset<tag_size>(address >> 32 - tag_size);

        auto& block = storage[pos];

        if (block.tag == tag && block.valid) {
            // hit, modify
            block.data = val;
            return true;
        } else {
            // miss, set data and tags
            block.valid = true;
            block.tag = tag;
            block.data = val;
            return false;
        }
    }
};
