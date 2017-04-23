#include <cstdio>
#include "MIPSCore.hpp"

int main()
{
    FILE *fp = fopen("dump", "rb");

    MIPSCore core;

    fread(&core.memory[0], sizeof(u32), 4, fp);

    while (core.Cycle()) {

    }

    return 0;
}
