#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "opcode.h"

const static size_t N_REGISTER = 32;

class ALU {
public:
    ALU(std::array<uint32_t, N_REGISTER>* registers, std::vector<uint32_t>* memory)
        : m_registers(registers), m_memory(memory) {};

    // Arithmetic and Logic Unit methods
    // op1, op2, and op3 are register indices
    // op1 and op2 are the source registers, and op3 is the destination register
    void add(uint32_t op1, uint8_t op2, uint8_t op3) {
        (*m_registers)[op3] = (*m_registers)[op1] + (*m_registers)[op2];
    };

    void sub(uint32_t op1, uint32_t op2, uint32_t op3) {
        (*m_registers)[op3] = (*m_registers)[op1] - (*m_registers)[op2];
    };

    void mul(uint32_t op1, uint32_t op2, uint32_t op3) {
        (*m_registers)[op3] = (*m_registers)[op1] * (*m_registers)[op2];
    };

    void div(uint32_t op1, uint32_t op2, uint32_t op3) {
        (*m_registers)[op3] = (*m_registers)[op1] / (*m_registers)[op2];
    };

    void mod(uint32_t op1, uint32_t op2, uint32_t op3) {
        (*m_registers)[op3] = (*m_registers)[op1] % (*m_registers)[op2];
    };

    void and_op(uint32_t op1, uint32_t op2, uint32_t op3) {
        (*m_registers)[op3] = (*m_registers)[op1] & (*m_registers)[op2];
    };

    void or_op(uint32_t op1, uint32_t op2, uint32_t op3) {
        (*m_registers)[op3] = (*m_registers)[op1] | (*m_registers)[op2];
    };

    void xor_op(uint32_t op1, uint32_t op2, uint32_t op3) {
        (*m_registers)[op3] = (*m_registers)[op1] ^ (*m_registers)[op2];
    };

    void cmp(uint32_t op1, uint32_t op2, uint32_t op3) {
        (*m_registers)[op3] = (op1 == op2) ? 0 : (op1 < op2) ? 1 : 2;
    };

    void lda(uint32_t op1, uint32_t op2, uint32_t op3) {
        (*m_registers)[op3] = (*m_memory)[op1];
    };

    void sta(uint32_t op1, uint32_t op2, uint32_t op3) {
        (*m_memory)[op3] = (*m_registers)[op1];
    };

    void mov(uint32_t op1, uint32_t op2, uint32_t op3) {
        (*m_registers)[op3] = op1;
    };

    void compute(uint32_t op1, uint32_t op2, uint32_t op3, OperationType opcode) {
        switch (opcode) {
            case OperationType::ADD:
                return add(op1, op2, op3);
            case OperationType::SUB:
                return sub(op1, op2, op3);
            case OperationType::MUL:
                return mul(op1, op2, op3);
            case OperationType::DIV:
                return div(op1, op2, op3);
            case OperationType::MOD:
                return mod(op1, op2, op3);
            case OperationType::AND:
                return and_op(op1, op2, op3);
            case OperationType::OR:
                return or_op(op1, op2, op3);
            case OperationType::XOR:
                return xor_op(op1, op2, op3);
            case OperationType::CMP:
                return cmp(op1, op2, op3);
            case OperationType::LDA:
                return lda(op1, op2, op3);
            case OperationType::STA:
                return sta(op1, op2, op3);
            case OperationType::MOV:
                return mov(op1, op2, op3);
            default:
                return; // Default case for NOP or unrecognized operation
        }
    };

private:
    std::vector<uint32_t>* m_memory = nullptr; // Main memory of the CPU
    std::array<uint32_t, N_REGISTER>* m_registers = nullptr; // General-purpose registers of the CPU
};

class CU {
public:
    CU(std::array<uint32_t, N_REGISTER>* registers, std::vector<uint32_t>* memory, uint32_t* pc, uint32_t* ir) : m_memory(memory), m_registers(registers), m_pc(pc), m_ir(ir), m_alu(registers, memory) {};

    // Control Unit methods
    void fetch() {
        uint32_t instruction = (*m_memory)[*m_pc];
        *m_ir = instruction;
        (*m_pc)++;

        std::cout << "Fetch: " << std::hex << instruction << std::dec << " from memory address: " << *m_pc - 1 << std::endl;
    }; 

    void decode() {
        decode_opcode();
        decode_operands();
    }

    void execute() {
        m_alu.compute(m_op1, m_op2, m_op3, m_current_opcode);
        
        std::cout << "Execute: " << opcodetype_to_mnemonic(m_current_opcode) << " (" << static_cast<int>(m_current_opcode) << ")" << ", OP1: " << m_op1 << ", OP2: " << m_op2 << ", OP3: " << m_op3 << std::endl;
    }

    void fetch_decode_execute() {
        std::cout << "========================================" << std::endl;
        fetch();
        decode();
        execute();
        std::cout << "========================================" << std::endl;
    }

    // Decode methods
    void decode_opcode() {
        if (m_ir == nullptr) {
            m_current_opcode =  OperationType::NOP;
        }

        m_current_opcode =  decode_opcode_from_bits(*m_ir);
    }

    void decode_operands() {
        if(m_current_opcode == OperationType::LDA || m_current_opcode == OperationType::STA || m_current_opcode == OperationType::MOV) {
            std::cout << "Decode: (2) operands for ALU operation." << std::endl;
            m_op1 = (*m_ir & 0x3E00000u) >> 21;
            m_op2 = 0; // For LDA/STA/MOV, the second operand is not used -> maintain convention that op3 is the destination register
            m_op3 = (*m_ir & 0x1F0000u) >> 16;
        }
        else {
            std::cout << "Decode: (3) operands for ALU operation." << std::endl;
            m_op1 = (*m_ir & 0x3E00000u) >> 21;
            m_op2 = (*m_ir & 0x1F0000u) >> 16;
            m_op3 = (*m_ir & 0xF800u) >> 11;
        }
    }

private:
    OperationType decode_opcode_from_bits(uint32_t instruction) {
        uint8_t opcode = (instruction & 0xFC000000u) >> 26;
        return opcode_to_opcodetype(opcode);
    }

private:
    ALU m_alu; // ALU instance for performing arithmetic and logic operations

    std::vector<uint32_t>* m_memory = nullptr; // Main memory of the CPU

    // Registers and control signals
    std::array<uint32_t, N_REGISTER>* m_registers = nullptr; // General-purpose registers of the CPU
    uint32_t* m_pc = nullptr; // Program Counter
    uint32_t* m_ir = nullptr; // Instruction Register

    OperationType m_current_opcode;
    uint32_t m_op1;
    uint32_t m_op2;
    uint32_t m_op3;
};

class CPU {
public:
    CPU() : m_registers{}, m_cu(&m_registers, &m_memory, &m_pc, &m_ir), m_pc(0), m_ir(0) {}

    void run() {
        while (m_pc < m_memory.size()) {
            m_cu.fetch_decode_execute();
        }
    }

    void load_program(std::string program) {
        // Function to read a text file line by line and parse each line as a 32-bit hex/decimal value
        std::vector<uint32_t> values;
        std::ifstream file(program);

        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << program << "\n";
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            // Skip empty lines or lines starting with a comment (# or //)
            if (line.empty() || line[0] == '#' || (line.length() >= 2 && line.substr(0, 2) == "//")) {
                continue;
            }

            std::stringstream ss(line);
            uint32_t value = 0;

            // std::stoul automatically handles both standard decimal 
            // and hexadecimal (if prefixed with "0x")
            try {
                // Check if the string starts with "0x" to explicitly handle hex, 
                // or let std::stoul handle base 0 (auto-detects 0x for hex)
                size_t idx;
                unsigned long parsed = std::stoul(line, &idx, 0);
                
                if (parsed <= UINT32_MAX) {
                    m_memory.push_back(static_cast<uint32_t>(parsed)); // Store the full 32-bit value
                } else {
                    std::cerr << "Warning: Value out of 32-bit range: " << line << "\n";
                }
            } catch (const std::invalid_argument& e) {
                std::cerr << "Warning: Invalid number format in line: " << line << "\n";
            } catch (const std::out_of_range& e) {
                std::cerr << "Warning: Number out of range in line: " << line << "\n";
            }
        }

        file.close();
    }

    void print_registers() const {
        std::cout << "Registers:\n";
        for (size_t i = 0; i < N_REGISTER; ++i) {
            std::cout << "  R" << i << " = 0x" << std::hex << std::setw(8) << std::setfill('0')
                      << m_registers[i] << " (" << std::dec << m_registers[i] << ")\n";
        }
        std::cout << std::dec;
    }

    void print_memory() const {
        std::cout << "Memory:\n";
        if (m_memory.empty()) {
            std::cout << "  <empty>\n";
            return;
        }

        for (size_t i = 0; i < m_memory.size(); ++i) {
            std::cout << "  M[" << i << "] = 0x" << std::hex << std::setw(8) << std::setfill('0')
                      << m_memory[i] << " (" << std::dec << m_memory[i] << ")\n";
        }
        std::cout << std::dec;
    }

    void print_state() const {
        print_registers();
        print_memory();
    }

private: 
    CU m_cu; // Control Unit instance for managing instruction execution

    std::vector<uint32_t> m_memory; // Main memory of the CPU
    std::array<uint32_t, N_REGISTER> m_registers; // General-purpose registers of the CPU
    
    uint32_t m_pc = 0;
    uint32_t m_ir = 0;
};

int main(int argc, char* argv[]) {
    std::string program = (argc > 1) ? argv[1] : "prog1.txt";

    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [program_file]\n";
        return 1;
    }

    CPU cpu;

    cpu.load_program(program);
    cpu.run();
    cpu.print_state();

    return 0;
}