#include "MIPSCore.hpp"
#include "../MIPSException.hpp"

#define DECODE_I u32 rs = (instruction >> 21) & 0x1F, \
                        rt = (instruction >> 16) & 0x1F; \
                        s32 imm = static_cast<s16>(instruction & 0xFFFF);

#define DECODE_R u32 rs = (instruction >> 21) & 0x1F, \
                        rt = (instruction >> 16) & 0x1F, \
                        rd = (instruction >> 11) & 0x1F, \
                        shamt = (instruction >> 6) & 0x1F;

MIPSCore::MIPSCore(OutputSink* sink) :
    gpr({}),
    hi(0),
    lo(0),
    pc(0x400000 / 4),
    memory(),
    sink(sink)
{
    gpr[28] = 0x10008000; // $gp;
    gpr[29] = 0x7FFFEFFC; // $sp
}

void MIPSCore::Reset()
{
    gpr.fill(0);

    gpr[28] = 0x10008000; // $gp;
    gpr[29] = 0x7FFFEFFC; // $sp

    hi = 0;
    lo = 0;
    pc = 0x400000 / 4;
}

bool MIPSCore::Cycle()
{
    bool success = memory.MaybeGet(pc, instruction);

    if (!success) {
        return false;
    }

    // first 6 bits on a MIPS instruction are the opcode
    u32 opcode = (instruction >> 26);

    switch (opcode) {
        case 0x00: {
            // R instructions
            u32 funct = (instruction & 0x3F);
            switch (funct) {
                case 0x00:
                    SLL();
                    break;
                case 0x02:
                    SRL();
                    break;
                case 0x08:
                    JR();
                    break;
                case 0x0C:
                    SYSCALL();
                    break;
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
                case 0x20:
                    ADD();
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
                case 0x26:
                    XOR();
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
        case 0x03:
            JAL();
            break;
        case 0x04:
            BEQ();
            break;
        case 0x05:
            BNE();
            break;
        case 0x06:
            BLEZ();
            break;
        case 0x07:
            BGTZ();
            break;
        case 0x08:
            ADDI();
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
        case 0x0E:
            XORI();
            break;
        case 0x0F:
            LUI();
            break;
        case 0x20:
            LB();
            break;            
        case 0x21:
            LH();
            break;
        case 0x22: // fallthrough            
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

void MIPSCore::SLL()
{
    DECODE_R;

    gpr[rd] = gpr[rt] << shamt;

    pc++;
}

void MIPSCore::SRL()
{
    DECODE_R;

    gpr[rd] = gpr[rt] >> shamt;

    pc++;
}

void MIPSCore::JR()
{
    DECODE_R;

    pc = gpr[rs] / 4;
}

void MIPSCore::SYSCALL()
{
    // TODO: create a sink system for this

    switch (gpr[2]) {
        case 1: {
//            printf("%d\n", gpr[4]);
            sink->Print(std::to_string(gpr[4]));
            break;
        }
        /*case 4: {
            printf("%s\n", gpr[4]);
            break;
        }
        case 5: {
            scanf("%d\n", &gpr[2]);
            break;
        }
        case 8: {
            fgets(gpr[4], gpr[5], stdin);
            break;
        }*/
    }

    pc++;
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

void MIPSCore::ADD()
{
    // TODO: throw on overflow    
    DECODE_R;

    gpr[rd] = gpr[rs] + gpr[rt];

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

void MIPSCore::XOR()
{
    DECODE_R;

    gpr[rd] = gpr[rs] ^ gpr[rt];

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

void MIPSCore::J()
{
    u32 address = (pc & 0xF0000000) | ((instruction & 0xFFFFFF) << 2);

    pc = address / 4;
}

void MIPSCore::JAL()
{
    gpr[31] = (pc + 1) * 4;

    u32 address = (pc & 0xF0000000) | ((instruction & 0xFFFFFF) << 2);

    pc = address / 4;
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

void MIPSCore::BLEZ()//
{
    DECODE_I;

    if (gpr[rs] <= 0) {
        pc += imm;
    }

    pc++;
}

void MIPSCore::BGTZ()//
{
    DECODE_I;

    if (gpr[rs] > 0) {
        pc += imm;
    }

    pc++;
}

void MIPSCore::ADDI()
{
    DECODE_I;

    gpr[rt] = gpr[rs] + imm;

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

void MIPSCore::XORI()//
{
    DECODE_I;

    gpr[rt] = gpr[rs] ^ imm;

    pc++;
}

void MIPSCore::LUI()
{
    DECODE_I;

    gpr[rt] = static_cast<u32>(imm) << 16;

    pc++;
}

void MIPSCore::LB()//
{
    DECODE_I;

    uint16_t offset = imm / 4;

    u32 word = memory.Get((gpr[rs] + offset) / 4);

    uint16_t little_box = imm % 4;

    u8 byte = (word >> (24 - little_box * 8)) & 0xFF;

    s32 value = static_cast<s32>(static_cast<s8>(byte));

    gpr[rt] = value;

    pc++;
}

void MIPSCore::LH()//
{
    DECODE_I;

    if (((gpr[rs] + imm) % 2) != 0) {
        throw UnalignedAccessException(pc, gpr[rs] + imm);
    }

    uint16_t offset = imm / 4;

    u32 word = memory.Get((gpr[rs] + offset) / 4);

    uint16_t little_box = imm % 4;

    u16 half = (word >> (16 - little_box * 16)) & 0xFFFF;

    s32 value = static_cast<s32>(static_cast<s16>(half));

    gpr[rs] = value;

    pc++;
}

void MIPSCore::LW()
{
    DECODE_I;

    if (((gpr[rs] + imm) % 4) != 0) {
        throw UnalignedAccessException(pc, gpr[rs] + imm);
    }

    gpr[rt] = memory.Get((gpr[rs] + imm) / 4);

    pc++;
}

void MIPSCore::SW()
{
    DECODE_I;

    if (((gpr[rs] + imm) % 4) != 0) {
        throw UnalignedAccessException(pc, gpr[rs] + imm);
    }

    memory.Set((gpr[rs] + imm) / 4, gpr[rt]);

    pc++;
}

#undef DECODE_I
#undef DECODE_R
