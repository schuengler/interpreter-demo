g++ assembler.cpp -std=c++17 -include opcode.h -o assembler.o
g++ cpu.cpp -std=c++17 -include opcode.h -o cpu.o