#include <stdint.h>
#include <stdio.h>

typedef uint64_t u64;

#define SLOT_BITS 8

#define SLOT_MASK ((1 << SLOT_BITS) - 1)

#define HARDWIRED_BITS 2
#define HARDWIRED_MASK (((1 << HARDWIRED_BITS) - 1) << (SLOT_BITS - HARDWIRED_BITS))

#define ADDR_BITS (64 - (3 * SLOT_BITS))

static int adjustSlotNumber(int slot)
{
    if(slot < 0)
        slot = SLOT_MASK + 1 + slot;
    return slot;
}

static u64 encode(int dst, int op1, int op2, unsigned jmp)
{
    u64 ret = jmp;
    ret = (ret << SLOT_BITS) | adjustSlotNumber(op2);
    ret = (ret << SLOT_BITS) | adjustSlotNumber(op1);
    ret = (ret << SLOT_BITS) | adjustSlotNumber(dst);
    return ret;
}

#define HARDWIRED_SLOT_ZERO -1
#define HARDWIRED_SLOT_POT0 -2
#define HARDWIRED_SLOT_POT1 -3

int main(int argc, char ** argv)
{
    (void)argc;
    (void)argv;

    const u64 code[] = {
        #include "raw.txt"
    };

    const int inscount = sizeof(code) / sizeof(code[0]);

    int slots[1 << SLOT_BITS] = { 0 };
    slots[adjustSlotNumber(HARDWIRED_SLOT_ZERO)] = 0;
    slots[adjustSlotNumber(HARDWIRED_SLOT_POT0)] = 1 << 0;
    slots[adjustSlotNumber(HARDWIRED_SLOT_POT1)] = 1 << 1;

    int pc = 0;
    while(1)
    {
        const u64 ins = code[pc];
        const int dst = (ins >> 0 * SLOT_BITS) & SLOT_MASK;
        const int op1 = (ins >> 1 * SLOT_BITS) & SLOT_MASK;
        const int op2 = (ins >> 2 * SLOT_BITS) & SLOT_MASK;
        const int jmp = (ins >> 3 * SLOT_BITS) & 0xfffffff;
        const int val = slots[op1] - slots[op2];

        printf("%d subleq 0x%08x%08x (%d %d %d %d)\n", pc, (unsigned)(ins >> 32), (unsigned)ins, dst, op1, op2, jmp);
        if((HARDWIRED_MASK & dst) == HARDWIRED_MASK)
        {
            printf("slot[%d] | %d\n", dst, val);
        }
        else
        {
            slots[dst] = val;
            printf("slot[%d] = %d\n", dst, val);
        }

        if(slots[dst] <= 0)
        {
            pc = jmp;
            printf("jumping to %d\n", pc);
        }
        else
        {
            ++pc;
        }

        if(pc >= inscount)
            break;
    } /* while 1 */

    return 0;
}
