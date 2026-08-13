#ifndef OPCODE_H
#define OPCODE_H

#include <array>
#include <cstdint>
#include <string>

enum class OperationType {
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    AND,
    OR,
    XOR,
    CMP,
    NOP,
    LDA,
    STA,
    MOV
};

struct OpcodeEntry {
    OperationType type;
    uint8_t bits;
    const char* mnemonic;
};

inline constexpr std::array<OpcodeEntry, 14> kOpcodeTable = {{
    {OperationType::ADD, 0x00u, "ADD"},
    {OperationType::SUB, 0x01u, "SUB"},
    {OperationType::MUL, 0x02u, "MUL"},
    {OperationType::DIV, 0x03u, "DIV"},
    {OperationType::MOD, 0x04u, "MOD"},
    {OperationType::AND, 0x05u, "AND"},
    {OperationType::OR,  0x06u, "OR" },
    {OperationType::XOR, 0x07u, "XOR"},
    {OperationType::CMP, 0x08u, "CMP"},
    {OperationType::NOP, 0x0Cu, "NOP"},
    {OperationType::LDA, 0x09u, "LDA"},
    {OperationType::STA, 0x0Au, "STA"},
    {OperationType::MOV, 0x0Bu, "MOV"},
    {OperationType::NOP, 0x0Cu, "NOP"}
}};

inline std::string opcodetype_to_mnemonic(OperationType opcode) {
    for (const auto& entry : kOpcodeTable) {
        if (entry.type == opcode) {
            return entry.mnemonic;
        }
    }
    return "NOP";
}

inline OperationType mnemonic_to_opcodetype(std::string mnemonic) {
    for (const auto& entry : kOpcodeTable) {
        if (mnemonic == entry.mnemonic) {
            return entry.type;
        }
    }
    return OperationType::NOP;
}

inline OperationType opcode_to_opcodetype(uint8_t opcode) {
    for (const auto& entry : kOpcodeTable) {
        if (entry.bits == opcode) {
            return entry.type;
        }
    }
    return OperationType::NOP;
}

inline uint8_t opcodetype_to_opcode(OperationType opcode) {
    for (const auto& entry : kOpcodeTable) {
        if (entry.type == opcode) {
            return entry.bits;
        }
    }
    return 0x0Cu;
}

#endif