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

// size gets used as 2^size
template <u32 size>
class Cache {
    static const u32 tag_size = 32 - size + 2;

    std::array<Block<tag_size>, size> storage;

    const std::unordered_map<u32, u32>& memory;
public:
    Cache(const std::unordered_map<u32, u32>& memory)  : memory(memory)
    {
        for (auto& block : storage) {
            block.valid = false;
        }
    }

    u32 Get(u32 address)
    {
        u32 pos = address % size;
        auto tag = std::bitset<tag_size>(address >> 32 - tag_size);

        auto& block = storage[pos];

        if (block.tag == tag && block.valid) {
            // HIT
            // nothing, I think?
        } else {
            // miss, bring the data from memory and set valid and tag
            block.valid = true;
            block.tag = tag;
            block.data = memory.at(address);

            return block.data;
        }

        return block.data;
    }

    void Set(u32 address, u32 val)
    {
        u32 pos = address % size;
        auto tag = std::bitset<tag_size>(address > 32 - tag_size);

        auto &block = storage[pos];

        if (block.tag == tag && block.valid) {
            // hit, modify
            block.data = val;
        } else {
            // miss, set data and tags
            block.valid = true;
            block.tag = tag;
            block.data = val;
        }
    }

    bool MaybeGet(u32 address, u32& out)
    {

    }
};
