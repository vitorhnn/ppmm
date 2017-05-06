#pragma once

#include <exception>
#include <cstdint>
#include <string>
#include <sstream>

struct MIPSException : public std::exception {
    virtual ~MIPSException() noexcept {};

    virtual const char* what() const noexcept override = 0;
};

struct UndefinedInstructionException : public MIPSException {
    UndefinedInstructionException(uint32_t opcode, uint32_t funct, uint32_t pc) :
        opcode(opcode),
        funct(funct),
        pc(pc)
    {
        std::stringstream stream;

        stream << std::hex << "PC " << pc << ": Unknown opcode " << opcode << " or funct " << funct;

        msg = stream.str();
    }

    virtual ~UndefinedInstructionException() noexcept {};

    virtual const char* what() const noexcept override
    {
        return msg.c_str();
    }

    uint32_t opcode;

    uint32_t funct;

    uint32_t pc;

    std::string msg;
};

struct UnalignedAccessException : public MIPSException {
    UnalignedAccessException(uint32_t pc, uint32_t address) :
        pc(pc),
        address(address)
    {
        std::stringstream stream;

        stream << std::hex << "PC " << pc << ": Attempted to access unaligned memory address " << address;

        msg = stream.str();
    }

    virtual ~UnalignedAccessException() noexcept {};

    virtual const char* what() const noexcept override
    {
        return msg.c_str();
    }

    uint32_t pc;
    uint32_t address;
    std::string msg;
};
