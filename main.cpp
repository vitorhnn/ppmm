#include <cstdio>
#include "MIPSCore.hpp"

int main()
{
    FILE *fp = fopen("dump", "rb");

    MIPSCore core;

    u32 i = core.pc = 0x400000 / 4,
        byte = 0;

    while (fread(&byte, sizeof(u32), 1, fp) != 0) {
        core.memory[i] = byte;
        i++;
    }

    while (core.Cycle()) {

    }

    for (size_t i = 0; i < 32; ++i) {
        printf("%zu: %u\n", i, core.gpr[i]);
    }

    return 0;
}
