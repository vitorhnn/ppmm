#pragma once

#include <exception>
#include <cstdint>
#include <string>
#include <sstream>

struct MIPSException : public std::exception {
    virtual ~MIPSException() throw () {};

    virtual const char* what() const throw() override = 0;
};

struct UndefinedInstructionException : public MIPSException {
    UndefinedInstructionException(uint32_t opcode, uint32_t funct) :
        opcode(opcode),
        funct(funct)
    {
        std::stringstream stream;

        stream << "Unknown opcode " << opcode << " or funct " << funct;

        msg = stream.str();
    }

    virtual ~UndefinedInstructionException() throw () {};

    virtual const char* what() const throw() override
    {
        return msg.c_str();
    }

private:
    uint32_t opcode;

    uint32_t funct;

    std::string msg;
};