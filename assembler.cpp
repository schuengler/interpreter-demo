#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "opcode.h"

static uint32_t parse_operand_as_u32(const std::string& operand) {
    std::string token = operand;
    for (char& ch : token) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }

    std::string digits;
    if (token.rfind("reg", 0) == 0) {
        digits = token.substr(3);
    } else if (token.rfind("r", 0) == 0 && token.size() > 1) {
        digits = token.substr(1);
    } else {
        digits = token;
    }

    if (digits.empty()) {
        return 0u;
    }

    try {
        return static_cast<uint32_t>(std::stoul(digits, nullptr, 0)) & 0x1Fu;
    } catch (...) {
        return 0u;
    }
}

struct Instruction {
    std::string mnemonic;
    OperationType opcode;
    std::vector<std::string> operands;
};

class Assembler {
public:
    Assembler() = default;

    void load_assembler(std::string program) {
        std::ifstream file(program);

        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << program << "\n";
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#' || (line.length() >= 2 && line.substr(0, 2) == "//")) {
                continue;
            }

            Instruction instr{};
            if (parse_instruction_line(line, instr)) {
                std::cout << "mnemonic=" << instr.mnemonic
                          << " opcode=" << static_cast<int>(instr.opcode)
                          << " operands=";
                for (const auto& operand : instr.operands) {
                    std::cout << operand << " ";
                }
                std::cout << "\n";

                uint32_t word = encode_instruction_word(instr);
                std::cout << "word=" << std::hex << word << std::dec << "\n";
            }
        }
    }

    bool parse_instruction_line(const std::string& line, Instruction& instruction) {
        static const std::regex two_operand_pattern(
            R"(^\s*([A-Za-z]+)\s+([A-Za-z0-9_]+)\s*,\s*([A-Za-z0-9_]+)\s*$)"
        );
        static const std::regex three_operand_pattern(
            R"(^\s*([A-Za-z]+)\s+([A-Za-z0-9_]+)\s*,\s*([A-Za-z0-9_]+)\s*,\s*([A-Za-z0-9_]+)\s*$)"
        );

        std::smatch match;
        instruction.operands.clear();

        if (std::regex_match(line, match, three_operand_pattern)) {
            instruction.mnemonic = match[1].str();
            instruction.opcode = mnemonic_to_opcodetype(instruction.mnemonic);
            instruction.operands.push_back(match[2].str());
            instruction.operands.push_back(match[3].str());
            instruction.operands.push_back(match[4].str());
            return true;
        }

        if (std::regex_match(line, match, two_operand_pattern)) {
            instruction.mnemonic = match[1].str();
            instruction.opcode = mnemonic_to_opcodetype(instruction.mnemonic);
            instruction.operands.push_back(match[2].str());
            instruction.operands.push_back(match[3].str());
            return true;
        }

        return false;
    }

    uint32_t encode_instruction_word(const Instruction& instruction) const {
        uint32_t word = 0;
        const uint32_t opcode_bits = static_cast<uint32_t>(opcodetype_to_opcode(instruction.opcode)) & 0x3Fu;
        word |= (opcode_bits << 26);

        if (instruction.operands.size() >= 1) {
            word |= (parse_operand_as_u32(instruction.operands[0]) & 0x1Fu) << 21;
        }

        if (instruction.operands.size() >= 2) {
            word |= (parse_operand_as_u32(instruction.operands[1]) & 0x1Fu) << 16;
        }

        if (instruction.operands.size() >= 3) {
            word |= (parse_operand_as_u32(instruction.operands[2]) & 0x1Fu) << 11;
        }

        return word;
    }

    bool assemble(const std::string& source_path, const std::string& output_path) {
        std::ifstream input(source_path);
        if (!input.is_open()) {
            std::cerr << "Error: could not open source file: " << source_path << "\n";
            return false;
        }

        std::ofstream output(output_path);
        if (!output.is_open()) {
            std::cerr << "Error: could not open output file: " << output_path << "\n";
            return false;
        }

        std::string line;
        while (std::getline(input, line)) {
            if (line.empty() || line[0] == '#' || (line.length() >= 2 && line.substr(0, 2) == "//")) {
                continue;
            }

            Instruction instr{};
            if (!parse_instruction_line(line, instr)) {
                std::cerr << "Warning: could not parse line: " << line << "\n";
                continue;
            }

            const uint32_t encoded = encode_instruction_word(instr);
            output << "0x" << std::hex << std::uppercase << encoded << std::nouppercase << std::dec << "\n";
        }

        input.close();
        output.close();
        return true;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0] << " <source_file> [destination_file]\n";
        return 1;
    }

    const std::string source = argv[1];
    const std::string output = (argc == 3) ? argv[2] : "machine_code.txt";

    Assembler assembler;
    const bool ok = assembler.assemble(source, output);

    if (!ok) {
        return 1;
    }

    std::cout << "Assembled: " << source << " -> " << output << "\n";
    return 0;
}
