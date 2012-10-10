CFLAGS=-pedantic -std=c99
PROGRAMS=vm pow
all: $(PROGRAMS)
vm: vm.c
pow: pow.asm
	./asm.py $< $@ 
.PHONY: clean
clean:
	@rm -f $(PROGRAMS)
