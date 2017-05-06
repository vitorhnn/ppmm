#include <cstdio>
#include "MIPSCore.hpp"

int main()
{
    FILE *fp = fopen("dump", "rb");

    MIPSCore core;

    size_t i = 0;
    u32 byte = 0;

    while (fread(&byte, sizeof(u32), 1, fp) != 0) {
        core.memory[i] = byte;
        i++;
    }

    while (core.Cycle()) {

    }

    return 0;
}
