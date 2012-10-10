#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdlib.h>

struct vm_t {
    uint32_t *ops;
    uint64_t ops_size;
    uint64_t reg[256];
    uint32_t reg_sp[32];
    uint64_t instr_ptr;
};

struct vm_t *create_vm(size_t program_size)
{
    struct vm_t *vm;
    vm = malloc(sizeof(struct vm_t));
    vm->ops = malloc(program_size * sizeof(vm->ops));
    vm->ops_size = program_size;
    vm->instr_ptr = 0;
    return vm;
}

void destroy_vm(struct vm_t *vm)
{
    free(vm->ops);
    free(vm);
}

struct vm_t *loader(char *filename)
{
    FILE *fh;
    uint64_t len, i;
    struct vm_t *vm;

    fh = fopen(filename, "r");
    if (fh == NULL) {
        return NULL;
    }
    fread(&len, sizeof(len), 1, fh);
    printf("loading \"%s\" (%li ops long)\n", filename, len);

    vm = create_vm(len);

    for (i=0;i<len;i++) {
        fread(vm->ops+i, sizeof(uint32_t), 1, fh);
    }

    return vm; 
}

void execute_vm(struct vm_t *vm, long max_steps)
{
    uint8_t opcode, X, Y, Z;
    uint16_t YZ;
    uint32_t XYZ;
    long step=0;

    while (1) {
        if (vm->instr_ptr >= vm->ops_size) {
            printf("info: reached the end of the program\n");
            break;
        }
            
        if (step == max_steps) {
            break;
        }
        step++;

        opcode = vm->ops[vm->instr_ptr] >> 8*0;
        X = vm->ops[vm->instr_ptr] >> 8*1;
        Y = vm->ops[vm->instr_ptr] >> 8*2;
        Z = vm->ops[vm->instr_ptr] >> 8*3;

        YZ = vm->ops[vm->instr_ptr] >> 8*2;
        XYZ = vm->ops[vm->instr_ptr] >> 8*1;

        switch (opcode) {
            case 0xF0:
                printf("opcode: JMP %u\n", XYZ);
                vm->instr_ptr += XYZ;
                break;
            case 0xF1:
                printf("opcode: JMP -%u\n", XYZ);
                vm->instr_ptr -= XYZ;
                break;
            case 0x18:
                printf("opcode: MUL $%hhu,$%hhu,$%hhu\n", X, Y, Z);
                vm->reg[X] = vm->reg[Y] * vm->reg[Z];
                break;
            case 0x19:
                printf("opcode: MUL $%hhu,$%hhu,%hhu\n", X, Y, Z);
                vm->reg[X] = vm->reg[Y] * Z;
                break;
            case 0x20:
                printf("opcode: ADD $%hhu,$%hhu,$%hhu\n", X, Y, Z);
                vm->reg[X] = vm->reg[Y] + vm->reg[Z];
                break;
            case 0x21:
                printf("opcode: ADD $%hhu,$%hhu,%hhu\n", X, Y, Z);
                vm->reg[X] = vm->reg[Y] + Z;
                break;
            case 0x24:
                printf("opcode: SUB $%hhu,$%hhu,$%hhu\n", X, Y, Z);
                vm->reg[X] = vm->reg[Y] - vm->reg[Z];
                break;
            case 0x25:
                printf("opcode: SUB $%hhu,$%hhu,%hhu\n", X, Y, Z);
                vm->reg[X] = vm->reg[Y] - Z;
                break;
            case 0x40:
                printf("opcode: BN  $%hhu,%hu\n", X, YZ);
                if (vm->reg[X] < 0) vm->instr_ptr += YZ;
                break;
            case 0x41:
                printf("opcode: BN  $%hhu,-%hu\n", X, YZ);
                if (vm->reg[X] < 0) vm->instr_ptr -= YZ;
                break;
            case 0x42:
                printf("opcode: BZ  $%hhu,%hu\n", X, YZ);
                if (vm->reg[X] == 0) vm->instr_ptr += YZ;
                break;
            case 0x43:
                printf("opcode: BZ  $%hhu,-%hu\n", X, YZ);
                if (vm->reg[X] == 0) vm->instr_ptr -= YZ;
                break;
            case 0x44:
                printf("opcode: BP  $%hhu,%hu\n", X, YZ);
                if (vm->reg[X] > 0) vm->instr_ptr += YZ;
                break;
            case 0x45:
                printf("opcode: BP  $%hhu,-%hu\n", X, YZ);
                if (vm->reg[X] > 0) vm->instr_ptr -= YZ;
                break;
            case 0xE3:
                printf("opcode: SETL $%hhu,%hu\n", X, YZ);
                vm->reg[X] = YZ;
                break;
            default:
                printf("error: unknown opcode: %.2hhx\n", opcode);
                exit(1);
        }

        vm->instr_ptr++;
    }
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("usage: %s EXE\n", argv[0]);
        return 1;
    }
    struct vm_t *vm = loader(argv[1]);

    if (vm == NULL) {
        printf("error: problem loading %s\n", argv[1]);
        return 1;
    }

    execute_vm(vm, 10000);

    destroy_vm(vm);

    return 0;
}
