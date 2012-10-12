#!/usr/bin/env python
from sys import argv, exit
from struct import pack
import re

def tokenize(s):
    re_base   = r'(?:(?P<label>\w+):)?\s?(?P<opcode>\w+)\s+'

    re_3args  = r'([$\d\w]+),([$\d\w]+),([$\d\w]+)'
    re_2args  = r'([$\d\w]+),([-$\d\w]+)'
    re_1arg   = r'([$\d\w]+)'
    re_quotes = r'(?<!\\)(?:\\\\)*"((?:\\.|[^\\"])*)"'
    arg_patterns = [re_3args, re_2args, re_1arg, re_quotes]

    m = re.match(re_base, s)

    if m == None:
        return

    label  = m.group('label')
    opcode = m.group('opcode')
    tokens = {'label':label, 'opcode':opcode}
    end = m.end()

    for pattern in arg_patterns:

        m = re.match(pattern, s[end:])
        if m:
            tokens['args'] = list(m.groups())
            return tokens
    return None

opcode_table = {
    'TRAP' : 0x00,
    'MUL'  : 0x18,
    'SUB'  : 0x24,
    'BP'   : 0x44,
    'SETL' : 0xE3,
}

f = open(argv[1])
instructions = []
for i,line in enumerate(f):
    line = line.strip()
    comment = line.rfind(';')
    if comment != -1:
        line = line[:comment]
    if len(line) == 0: continue
    fields = line.split()

    tokens = tokenize(line)
    if tokens == None:
        print 'error parsing line %i: %s' % (i, line)
    opcode = tokens['opcode']
    args = tokens['args']

    if opcode == 'SETL':
        args[0] = args[0].strip('$')
        ins = pack('>BBH', opcode_table[opcode], int(args[0]), int(args[1]))
        instructions.append(ins)
    elif opcode == 'MUL' or opcode == 'SUB':
        if '$' not in args[-1]:
            opcode_offset = 1
        else:
            opcode_offset = 0

        args = [ arg.strip('$') for arg in args ]

        ins = pack('>BBBB', opcode_table[opcode]+opcode_offset, 
                int(args[0]), int(args[1]), int(args[2]))
        instructions.append(ins)

    elif opcode == 'BP':
        args[0] = args[0].strip('$')
        if args[1][0] == '-':
            opcode_offset = 1
            args[1] = args[1].strip('-')
        else:
            opcode_offset = 0
        
        ins = pack('>BBH', opcode_table[opcode]+opcode_offset, 
                int(args[0]), int(args[1]))
        instructions.append(ins)
    elif opcode == 'TRAP':
        ins = pack('>BBBB', opcode_table[opcode], 
                int(args[0]), int(args[1]), int(args[2]))
        instructions.append(ins)
    elif opcode == 'BYTE':
        s = args[0]
        s = s.replace('\\n','\n')
        pad_len = 4-len(s)%4
        if pad_len > 0:
            s += pad_len*"\0"
        for i in xrange(len(s)/8):
            instructions.append(s[i*8:(i*8)+8])
    else:
        print 'error parsing line %i: unknown opcode: %s' % (i, line)
        exit(1)

fout = open(argv[2],'w')
data = ''.join(instructions)
fout.write(pack('>L', len(data)))
fout.write(data)
fout.close()
