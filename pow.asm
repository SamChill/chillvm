SETL $0,2     ;set register $0 to 2
SETL $1,2     ;set register $1 to 2
SETL $2,9     ;set register $2 to 9
MUL  $0,$0,2  ;multiply $0 by $2 and store in $0
SUB  $2,$2,1  ;decrement loop counter by 1
BP   $2,-3    ;if $2 is positive go back three instructions
SETL $255,288
TRAP 0,7,1    ;write string pointed to in $255 to stdout
TRAP 0,0,0
string: BYTE "Hello, world!\n"  
