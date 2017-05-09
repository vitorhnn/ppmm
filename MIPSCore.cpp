#include "MIPSCore.hpp"
#include "MIPSException.hpp"

#define DECODE_I u32 rs = (instruction >> 21) & 0x1F, \
                        rt = (instruction >> 16) & 0x1F; \
                        s32 imm = static_cast<s16>(instruction & 0xFFFF);

#define DECODE_R u32 rs = (instruction >> 21) & 0x1F, \
                        rt = (instruction >> 16) & 0x1F, \
                        rd = (instruction >> 11) & 0x1F, \
                        shamt = (instruction >> 6) & 0x1F;

MIPSCore::MIPSCore() :
    gpr({}),
    hi(0),
    lo(0),
    pc(0),
    memory()
{
}

bool MIPSCore::Cycle()
{
    instruction = memory[pc];
    if (instruction == 0) {
        return false;
    }

    // first 6 bits on a MIPS instruction are the opcode
    u32 opcode = (instruction >> 26);

    switch (opcode) {
        case 0x00: {
            // R instructions
            u32 funct = (instruction & 0x3F);
            switch (funct) {
                case 0x0C:
                    SYSCALL();
                case 0x10:
                    MFHI();
                    break;
                case 0x12:
                    MFLO();
                    break;
                case 0x18:
                    MULT();
                    break;
                case 0x19:
                    MULTU();
                    break;
                case 0x21:
                    ADDU();
                    break;
                case 0x22:
                    SUB();
                    break;
                case 0x24:
                    AND();
                    break;
                case 0x25:
                    OR();
                    break;
                case 0x27:
                    NOR();
                    break;
                case 0x2A:
                    SLT();
                    break;
                default:
                    throw UndefinedInstructionException(opcode, funct, pc);
            }
        }
        break;
        case 0x02:
            J();
            break;
        case 0x04:
            BEQ();
            break;
        case 0x05:
            BNE();
            break;
        case 0x09:
            ADDIU();
            break;
        case 0x0A:
            SLTI();
            break;
        case 0x0B:
            SLTIU();
            break;
        case 0x0C:
            ANDI();
            break;
        case 0x0D:
            ORI();
            break;
        case 0x0F:
            LUI();
            break;
        case 0x23:
            LW();
            break;
        case 0x2B:
            SW();
            break;
        default:
            throw UndefinedInstructionException(opcode, 0, pc);
    }
    gpr[0] = 0; // $0 is ALWAYS zero. (at least to an external viewer)

    return true;
}

void MIPSCore::SYSCALL()
{
    // TODO: create a sink system for this

    switch (gpr[2]) {
        case 1: {
            printf("%d\n", gpr[4]);
            break;
        }
    }
}

void MIPSCore::J()
{
    u32 address = (pc & 0xF0000000) | ((instruction & 0xFFFFFF) << 2);

    pc = address / 4;
}

void MIPSCore::MFHI()
{
    DECODE_R;

    gpr[rd] = hi;

    pc++;
}

void MIPSCore::MFLO()
{
    DECODE_R;

    gpr[rd] = lo;

    pc++;
}

void MIPSCore::MULT()
{
    DECODE_R;

    s64 res = static_cast<s32>(gpr[rs]) * static_cast<s32>(gpr[rt]);

    hi = static_cast<u32>(res >> 32);
    lo = static_cast<u32>(res & 0xFFFFFFF);

    pc++;
}

void MIPSCore::MULTU()
{
    DECODE_R;

    u64 res = static_cast<u64>(gpr[rs]) * static_cast<u64>(gpr[rt]);

    hi = static_cast<u32>(res >> 32);
    lo = static_cast<u32>(res & 0xFFFFFFF);

    pc++;
}

void MIPSCore::ADDU()
{
    DECODE_R;

    gpr[rd] = gpr[rs] + gpr[rt];

    pc++;
}

void MIPSCore::SUB()
{
    // TODO: throw on overflow
    DECODE_R;

    gpr[rd] = gpr[rs] - gpr[rt];

    pc++;
}

void MIPSCore::AND()
{
    DECODE_R;

    gpr[rd] = gpr[rs] & gpr[rt];

    pc++;
}

void MIPSCore::OR()
{
    DECODE_R;

    gpr[rd] = gpr[rs] | gpr[rt];

    pc++;
}

void MIPSCore::NOR()
{
    DECODE_R;

    gpr[rd] = static_cast<u32>(!(gpr[rs] | gpr[rt]));

    pc++;
}

void MIPSCore::SLT()
{
    DECODE_R;

    gpr[rd] = static_cast<u32>(static_cast<s32>(gpr[rs]) < static_cast<s32>(gpr[rt]));

    pc++;
}

void MIPSCore::BEQ()
{
    DECODE_I;

    if (gpr[rs] == gpr[rt]) {
        pc += imm;
    }

    pc++;
}

void MIPSCore::BNE()
{
    DECODE_I;

    if (gpr[rs] != gpr[rt]) {
        pc += imm;
    }

    pc++;
}

void MIPSCore::ADDIU()
{
    DECODE_I;

    gpr[rt] = gpr[rs] + imm;

    pc++;
}

void MIPSCore::SLTI()
{
    DECODE_I;

    gpr[rt] = static_cast<u32>(static_cast<s32>(gpr[rs]) < imm);

    pc++;
}

void MIPSCore::SLTIU()
{
    DECODE_I;

    gpr[rt] = static_cast<u32>(gpr[rs] < static_cast<u16>(imm));

    pc++;
}

void MIPSCore::ANDI()
{
    DECODE_I;

    gpr[rt] = gpr[rs] & imm;

    pc++;
}

void MIPSCore::ORI()
{
    DECODE_I;

    gpr[rt] = gpr[rs] | imm;

    pc++;
}

void MIPSCore::LUI()
{
    DECODE_I;

    gpr[rt] = static_cast<u32>(imm) << 16;

    pc++;
}

void MIPSCore::LW()
{
    DECODE_I;

    if (((gpr[rs] + imm) % 4) != 0) {
        throw UnalignedAccessException(pc, gpr[rs] + imm);
    }

    gpr[rt] = memory[(gpr[rs] + imm) / 4];

    pc++;
}

void MIPSCore::SW()
{
    DECODE_I;

    if (((gpr[rs] + imm) % 4) != 0) {
        throw UnalignedAccessException(pc, gpr[rs] + imm);
    }

    memory[(gpr[rs] + imm) / 4] = gpr[rt];

    pc++;
}

#undef DECODE_I
#undef DECODE_R
