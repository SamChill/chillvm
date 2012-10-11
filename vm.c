#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

struct vm_t {
    void     *mem;
    uint64_t reg[256];
    uint32_t reg_sp[32];
    uint64_t ip; //instruction pointer
};

struct vm_t *create_vm(size_t);
void destroy_vm(struct vm_t *);
uint32_t swap_uint32(uint32_t);
uint64_t swap_uint64(uint64_t);
struct vm_t *loader(int, char **, size_t);
void execute_vm(struct vm_t *, long);


struct vm_t *create_vm(size_t size)
{
    struct vm_t *vm;
    vm = malloc(sizeof(struct vm_t));
    vm->mem = malloc(size);
    vm->ip = 0;

    for (int i=0;i<256;i++) vm->reg[i] = 0;
    for (int i=0;i<32;i++) vm->reg_sp[i] = 0;

    return vm;
}

void destroy_vm(struct vm_t *vm)
{
    free(vm->mem);
    free(vm);
}

uint32_t swap_uint32(uint32_t val)
{
    val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF ); 
    return (val << 16) | (val >> 16);
}

uint64_t swap_uint64(uint64_t val)
{
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | 
          ((val >> 8) & 0x00FF00FF00FF00FFULL );
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | 
          ((val >> 16) & 0x0000FFFF0000FFFFULL );
    return (val << 32) | (val >> 32);
}

void iswap_uint64(uint64_t *val)
{
    *val = ((*val << 8) & 0xFF00FF00FF00FF00ULL ) | 
          ((*val >> 8) & 0x00FF00FF00FF00FFULL );
    *val = ((*val << 16) & 0xFFFF0000FFFF0000ULL ) | 
          ((*val >> 16) & 0x0000FFFF0000FFFFULL );
    *val = (*val << 32) | (*val >> 32);
}

struct vm_t *loader(int argc, char **argv, size_t ram_size)
{
    FILE *fh;
    uint32_t len;
    struct vm_t *vm;

    fh = fopen(argv[0], "r");
    if (fh == NULL) {
        return NULL;
    }
    fread(&len, sizeof(len), 1, fh);
    len = swap_uint32(len);
    printf("loading \"%s\" (%"PRIu32" mem long)\n", argv[0], len);

    if (len*sizeof(uint32_t) > ram_size) {
        printf("error: program larger than main memory\n");
        return NULL;
    }

    vm = create_vm(ram_size);
    fread(vm->mem, sizeof(uint32_t), len, fh);

    vm->reg[0] = swap_uint64(argc);
    vm->reg[1] = swap_uint64(len*sizeof(uint32_t));

    //for (int i=0;i<argc;i++) {
    //    memcpy(vm->mem[ strlen(argv[i]);
    //}

    return vm; 
}

void execute_vm(struct vm_t *vm, long max_steps)
{
    uint8_t opcode, X, Y, Z;
    uint16_t YZ;
    uint32_t XYZ;
    long step=0;

    #define SWAP_X iswap_uint64(&vm->reg[X])
    #define SWAP_Y iswap_uint64(&vm->reg[Y])
    #define SWAP_Z iswap_uint64(&vm->reg[Z])
    #define SWAP_XY \
            SWAP_X; \
            if (Y != X) SWAP_Y;
    #define SWAP_YZ \
            SWAP_Y; \
            if (Z != Y) SWAP_Z;
    #define SWAP_XYZ \
            SWAP_X; \
            if (Y != X) SWAP_Y; \
            if (Z != Y || Z != X) SWAP_Z;

    while (1) {
        if (step == max_steps) {
            break;
        }
        step++;

        //Decode instruction
        
        //opcode is first byte
        opcode =  (uint8_t)((uint32_t *)vm->mem)[vm->ip];

        //Three argument form, 1 byte X, 1 byte Y, 1 byte Z
        X      =  (uint8_t)((uint8_t  *)vm->mem)[4*vm->ip+1];
        Y      =  (uint8_t)((uint8_t  *)vm->mem)[4*vm->ip+2];
        Z      =  (uint8_t)((uint8_t  *)vm->mem)[4*vm->ip+3];

        //Two argument form, 1 byte X, 2 byte YZ
        YZ     = (uint16_t)((uint16_t *)vm->mem)[2*vm->ip+1];
        YZ     = (YZ << 8) | (YZ >> 8 );

        //XXX: untested
        //XYZ    = (uint32_t)((uint32_t *)vm->mem)[vm->ip] << 8*1;

        switch (opcode) {
            case 0x00:
                printf("opcode: TRAP %"PRIu8" %"PRIu8" %"PRIu8"\n",
                        X, Y, Z);
                return;
            //case 0xF0:
            //    printf("opcode: JMP %u\n", XYZ);
            //    vm->ip += XYZ;
            //    break;
            //case 0xF1:
            //    printf("opcode: JMP -%u\n", XYZ);
            //    vm->ip -= XYZ;
            //    break;
            case 0x18:
                printf("opcode: MUL $%"PRIu8",$%"PRIu8",$%"PRIu8"\n", X, Y, Z);
                SWAP_YZ;
                vm->reg[X] = vm->reg[Y] * vm->reg[Z];
                SWAP_XYZ;
                break;
            case 0x19:
                printf("opcode: MUL $%"PRIu8",$%"PRIu8",%"PRIu8"\n", X, Y, Z);
                SWAP_Y;
                printf("        MUL %"PRIu64" %"PRIu64" %"PRIu8"\n",
                        vm->reg[X], vm->reg[Y], Z);
                vm->reg[X] = vm->reg[Y] * Z;
                SWAP_XY;
                break;
            case 0x20:
                printf("opcode: ADD $%"PRIu8",$%"PRIu8",$%"PRIu8"\n", X, Y, Z);
                SWAP_YZ;
                vm->reg[X] = vm->reg[Y] + vm->reg[Z];
                SWAP_XYZ;
                break;
            case 0x21:
                printf("opcode: ADD $%"PRIu8",$%"PRIu8",%"PRIu8"\n", X, Y, Z);
                SWAP_Y;
                vm->reg[X] = vm->reg[Y] + Z;
                SWAP_XY;
                break;
            case 0x24:
                printf("opcode: SUB $%"PRIu8",$%"PRIu8",$%"PRIu8"\n", X, Y, Z);
                SWAP_YZ;
                vm->reg[X] = vm->reg[Y] - vm->reg[Z];
                SWAP_XYZ;
                break;
            case 0x25:
                printf("opcode: SUB $%"PRIu8",$%"PRIu8",%"PRIu8"\n", X, Y, Z);
                SWAP_Y;
                printf("        SUB %"PRIu64",%"PRIu64",%"PRIu8"\n", 
                        vm->reg[X], vm->reg[Y], Z);
                vm->reg[X] = vm->reg[Y] - Z;
                SWAP_XY;
                break;
            //case 0x40:
            //    printf("opcode: BN  $%"PRIu8",%"PRIu16"\n", X, YZ);
            //    if (vm->reg[X] < 0) vm->ip += YZ;
            //    break;
            //case 0x41:
            //    printf("opcode: BN  $%"PRIu8",-%"PRIu16"\n", X, YZ);
            //    if (vm->reg[X] < 0) vm->ip -= YZ;
            //    break;
            case 0x42:
                printf("opcode: BZ  $%"PRIu8",%"PRIu16"\n", X, YZ);
                SWAP_X;
                if (vm->reg[X] == 0) vm->ip += YZ;
                SWAP_X;
                break;
            case 0x43:
                printf("opcode: BZ  $%"PRIu8",-%"PRIu16"\n", X, YZ);
                SWAP_X;
                if (vm->reg[X] == 0) vm->ip -= YZ;
                SWAP_X;
                break;
            case 0x44:
                printf("opcode: BP  $%"PRIu8",%"PRIu16"\n", X, YZ);
                SWAP_X;
                if (vm->reg[X] > 0) vm->ip += YZ;
                SWAP_X;
                break;
            case 0x45:
                printf("opcode: BP  $%"PRIu8",-%"PRIu16"\n", X, YZ);
                SWAP_X;
                printf("        BP  %"PRIu64",-%"PRIu16"\n", vm->reg[X], YZ);
                if (vm->reg[X] > 0) vm->ip -= YZ;
                SWAP_X;
                break;
            case 0xE3:
                printf("opcode: SETL $%"PRIu8",%"PRIu16"\n", X, YZ);
                SWAP_X;
                printf("        SETL %"PRIu64",%"PRIu16"\n", vm->reg[X], YZ);
                vm->reg[X] = YZ;
                SWAP_X;
                break;
            default:
                printf("error: unknown opcode: %.2"PRIx8"\n", opcode);
                exit(1);
        }

        vm->ip++;
    }
    #undef SWAP_X
    #undef SWAP_Y
    #undef SWAP_XY
    #undef SWAP_YZ
    #undef SWAP_XYZ
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s EXE\n", argv[0]);
        return 1;
    }
    struct vm_t *vm = loader(argc-1, argv+1, 1024*1024);

    if (vm == NULL) {
        printf("error: problem loading %s\n", argv[1]);
        return 1;
    }

    execute_vm(vm, 10000);

    printf("reg[0] = %"PRIu64"\n", swap_uint64(vm->reg[0]));

    destroy_vm(vm);

    return 0;
}
