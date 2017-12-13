#include "Memory.hpp"

Memory::Memory() : l1(storage)
{

}

void Memory::Assign(std::unordered_map<u32, u32>&& memory)
{
    // TODO: invalidate cache
    l1.Invalidate();
    storage = memory;
}

std::vector<std::pair<u32, u32>> Memory::AsVector()
{
    return std::vector<std::pair<u32, u32>>(storage.begin(), storage.end());
}

u32 Memory::Get(u32 address)
{
    // TODO: STUB
    auto l1val = l1.Get(address);
    return storage[address];
}

void Memory::Set(u32 address, u32 val)
{
    l1.Set(address, val);
    storage[address] = val;
}

bool Memory::MaybeGet(u32 address, u32& out)
{
    // TODO: STUB
    auto res = storage.find(address);

    if (res == storage.end()) {
        return false;
    }

    out = res->second;
    return true;
}


