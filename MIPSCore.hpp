#pragma once

#include <cstdint>
#include <array>


using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using s64 = int64_t;
using s32 = int32_t;
using s16 = int16_t;


struct MIPSCore {
    std::array<u32, 32> gpr;

    u32 hi, lo;

    u32 pc;

    std::array<u32, 1024> memory;

    MIPSCore();

    bool Cycle();

private:
    void MULTU();

    void ADDU();

    void BEQ();

    void BNE();

    void ADDIU();

    void ANDI();

    void ORI();
};
