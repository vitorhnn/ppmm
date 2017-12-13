#include "Memory.hpp"

Memory::Memory() : L1(storage)
{

}

void Memory::Assign(std::unordered_map<u32, u32> memory)
{
    // TODO: invalidate cache
    storage = memory;
}

std::vector<std::pair<u32, u32>> Memory::AsVector()
{
    return std::vector<std::pair<u32, u32>>(storage.begin(), storage.end());
}

u32 Memory::Get(u32 address)
{
    // TODO: STUB
    auto l1 = L1.Get(address);
    return storage[address];
}

void Memory::Set(u32 address, u32 val)
{
    L1.Set(address, val);
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


