set APR_LIB=c:\apr-dist

gcc -DWIN32 "-I%APR_LIB%\\include" -O0 -g3 -Wall -c -fmessage-length=0 -o "main.o" "main.c" 
gcc "-L%APR_LIB%\\lib" -o disassembler.exe "main.o" -llibapr-1