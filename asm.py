#!/usr/bin/env python
from sys import argv, exit
from struct import pack
from binascii import hexlify

opcode_table = {
    'MUL'  : 0x18,
    'SUB'  : 0x24,
    'BP'   : 0x44,
    'SETL' : 0xE3,
}

f = open(argv[1])
instructions = []
for line in f:
    line = line[0:line.rfind(';')]
    fields = line.split()
    opcode = fields[0]
    args = fields[1].split(',')

    if opcode == 'SETL':
        args[0] = args[0].strip('$')
        ins = pack('>BBH', opcode_table[opcode], int(args[0]), int(args[1]))
    elif opcode == 'MUL' or opcode == 'SUB':
        if '$' not in args[-1]:
            opcode_offset = 1
        else:
            opcode_offset = 0

        args = [ arg.strip('$') for arg in args ]

        ins = pack('>BBBB', opcode_table[opcode]+opcode_offset, 
                int(args[0]), int(args[1]), int(args[2]))
    elif opcode == 'BP':
        args[0] = args[0].strip('$')
        if args[1][0] == '-':
            opcode_offset = 1
            args[1] = args[1].strip('-')
        else:
            opcode_offset = 0
        
        ins = pack('>BBH', opcode_table[opcode]+opcode_offset, 
                int(args[0]), int(args[1]))
    else:
        print 'unknown opcode', opcode
        exit(1)

    #print hexlify(ins)
    instructions.append(ins)

fout = open(argv[2],'w')
fout.write(pack('>L', len(instructions)))
fout.write(''.join(instructions))
fout.close()
