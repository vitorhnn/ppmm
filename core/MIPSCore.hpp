#pragma once

#include <cstdint>
#include <array>
#include <unordered_map>

#include "Memory.hpp"
#include "Types.hpp"


struct MIPSCore {
    std::array<u32, 32> gpr;

    u32 hi, lo;

    u32 pc;

    Memory memory;

    explicit MIPSCore(OutputSink* sink);

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
