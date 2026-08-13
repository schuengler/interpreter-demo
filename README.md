A simple demonstration for implementing an interpreter in C++. 

# cpu.cpp 
- models a simple processor with alu and cu with limited instruction set and RISC-like 3-address-instructions and load-store paradigm
- interprets machine code 
- example: ./cpu <machine-code-file>

# assembler.cpp
- a basic assembler for the implemented cpu architecture
- converts assembler code into machine code
- example: ./assembler example/prog1.asm example/prog1.txt

# build
- run build.sh