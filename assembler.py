import sys

SLOT_BITS = 8
SLOT_MASK = ((1 << SLOT_BITS) - 1)

HARDWIRED_BITS = 2
HARDWIRED_MASK = (((1 << HARDWIRED_BITS) - 1) << (SLOT_BITS - HARDWIRED_BITS))

ADDR_BITS = (64 - (3 * SLOT_BITS))

HARDWIRED_SLOT_ZERO = -1
HARDWIRED_SLOT_POT1 = -2

f = open(sys.argv[1], 'r', encoding='ascii')

labels = {}
pc = 0
output = []
instructions = []
for line in map(str.strip, f):
    parts = line.split()
    if not parts or parts[0].startswith('#'):
        continue

    if parts[0].endswith(':'):
        assert parts[0] not in labels
        labels[parts[0][:-1]] = pc

    if parts[0] == 'jmp':
        instructions.append(line)
        goal = parts[1]
        if goal.startswith('+') or goal.startswith('-'):
            offset = int(goal)
            goal = pc + offset
        output.append((HARDWIRED_SLOT_ZERO, HARDWIRED_SLOT_ZERO, HARDWIRED_SLOT_ZERO, goal))
        pc += 1

    if parts[0] == 'nop':
        instructions.append(line)
        output.append((HARDWIRED_SLOT_POT1, HARDWIRED_SLOT_ZERO, HARDWIRED_SLOT_ZERO, 0))
        pc += 1

    if parts[0] == 'subleq':
        instructions.append(line)
        goal = parts[4]
        if goal.startswith('+') or goal.startswith('-'):
            offset = int(goal)
            goal = pc + offset
        output.append((int(parts[1]), int(parts[2]), int(parts[3]), goal))
        pc += 1


def adjustSlotNumber(slot):
    if slot < 0:
        slot = SLOT_MASK + 1 + slot
    return slot

def encode(dst, op1, op2, jmp):
    ret = jmp
    ret = (ret << SLOT_BITS) | adjustSlotNumber(op2)
    ret = (ret << SLOT_BITS) | adjustSlotNumber(op1)
    ret = (ret << SLOT_BITS) | adjustSlotNumber(dst)
    return ret

for i, o in enumerate(output):
    if type(o[3]) is int:
        jmp = o[3]
    else:
        jmp = labels[o[3]]
    print(*o[0:3], jmp, '#' + str(i), instructions[i], file=sys.stderr)
    output[i] = encode(*o[0:3], jmp)

print(',\n'.join(map(hex, output)))
