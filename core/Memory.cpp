#include "Memory.hpp"

Memory::Memory() :
        l1(storage),
        l2(storage),
        misses(0),
        hits(0)
{
}

void Memory::Assign(std::unordered_map<u32, u32>&& memory)
{
    l1.Invalidate();
    l2.Invalidate();
    storage = memory;
    misses = 0;
}

std::vector<std::pair<u32, u32>> Memory::AsVector()
{
    return std::vector<std::pair<u32, u32>>(storage.begin(), storage.end());
}

u32 Memory::Get(u32 address)
{
    u32 data;

    auto found = l1.Get(address, data);

    if (found) {
        ++hits;
        return data;
    }

    ++misses;

    found = l2.Get(address, data);

    if (found) {
        ++hits;
        return data;
    }

    ++misses;

    return storage[address];
}

void Memory::Set(u32 address, u32 val)
{
    // write through because we're lazy
    l1.Set(address, val);
    l2.Set(address, val);
    storage[address] = val;
}

bool Memory::FetchInstruction(u32 address, u32& out)
{
    auto res = storage.find(address);

    if (res == storage.end()) {
        return false;
    }

    out = res->second;
    return true;
}


