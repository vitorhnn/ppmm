#pragma once

#include <cstdint>
#include <array>
#include <unordered_map>


using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8  = uint8_t;

using s64 = int64_t;
using s32 = int32_t;
using s16 = int16_t;
using s8 = int8_t;

struct OutputSink {
    virtual ~OutputSink() {}

    virtual void Print(std::string line) = 0;
};

struct MIPSCore {
    std::array<u32, 32> gpr;

    u32 hi, lo;

    u32 pc;

    std::unordered_map<u32, u32> memory;

    MIPSCore(OutputSink* sink);

    bool Cycle();

    void Reset();

private:
    OutputSink* sink;

    u32 instruction;

    void SLL();

    void SRL();

    void JR();

    void SYSCALL();

    void J();

    void JAL();

    void MFHI();

    void MFLO();

    void MULT();

    void MULTU();

    void ADD();

    void ADDU();

    void SUB();

    void AND();

    void OR();

    void XOR();

    void NOR();

    void SLT();

    void BEQ();

    void BNE();

    void BLEZ();

    void BGTZ();

    void ADDI();

    void ADDIU();

    void SLTI();

    void SLTIU();

    void ANDI();

    void ORI();

    void XORI();

    void LUI();

    void LB();

    void LH();

    void LW();

    void SW();
};
