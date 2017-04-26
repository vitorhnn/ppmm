#include "MIPSCore.hpp"
#include "MIPSException.hpp"

#define DECODE_I u32 rs = (memory[pc] >> 21) & 0x1F, \
                        rt = (memory[pc] >> 16) & 0x1F; \
                        s32 imm = static_cast<s16>(memory[pc] & 0xFFFF);

#define DECODE_R u32 rs = (memory[pc] >> 21) & 0x1F, \
                        rt = (memory[pc] >> 16) & 0x1F, \
                        rd = (memory[pc] >> 11) & 0x1F, \
                        shamt = (memory[pc] >> 6) & 0x1F;

MIPSCore::MIPSCore() :
    gpr({0}),
    hi(0),
    lo(0),
    pc(0),
    memory({0})
{
}

bool MIPSCore::Cycle()
{
    if (memory[pc] == 0) {
        return false;
    }

    // first 6 bits on a MIPS instruction are the opcode
    u32 opcode = (memory[pc] >> 26);

    switch (opcode) {
        case 0x00: {
            // R instructions
            u32 funct = (memory[pc] & 0x3F);
            switch (funct) {
                case 0x18:
                    MULT();
                    break;
                case 0x19:
                    MULTU();
                    break;
                case 0x21:
                    ADDU();
                    break;
                default:
                    throw UndefinedInstructionException(opcode, funct);
            }
        }
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
        default:
            throw UndefinedInstructionException(opcode, 0);
    }
    gpr[0] = 0; // $0 is ALWAYS zero. (at least to an external viewer)

    return true;
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

#undef DECODE_I
#undef DECODE_R
