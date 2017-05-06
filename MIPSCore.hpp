#pragma once

#include <cstdint>
#include <array>
#include <unordered_map>


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

    std::unordered_map<u32, u32> memory;

    MIPSCore();

    bool Cycle();

private:
    void MULT();

    void MULTU();

    void ADDU();

    void SUB();

    void AND();

    void OR();

    void NOR();

    void SLT();

    void BEQ();

    void BNE();

    void ADDIU();

    void SLTI();

    void SLTIU();

    void ANDI();

    void ORI();

    void LUI();

    void LW();

    void SW();
};
