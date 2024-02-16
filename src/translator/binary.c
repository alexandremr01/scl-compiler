#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define FUNCT7_DEFAULT 0
#define FUNCT3_MAP_SIZE 33
#define FUNCT7_MAP_SIZE 3
#define MAX_LINE_LENGTH 128

// Function to get the funct3 code
int get_funct3(const char* opcode) {
    static const char* funct3_map[FUNCT3_MAP_SIZE][2] = {
        {"BEQ", "000"}, {"BNE", "001"}, {"BLT", "100"}, {"BGE", "101"}, {"BLTU", "110"},
        {"BGEU", "111"}, {"LB", "000"}, {"LH", "001"}, {"LW", "010"}, {"LBU", "100"},
        {"LHU", "101"}, {"SB", "000"}, {"SH", "001"}, {"SW", "010"}, {"ADDI", "000"},
        {"SLTI", "010"}, {"SLTIU", "011"}, {"XORI", "100"}, {"ORI", "110"}, {"ANDI", "111"},
        {"SLLI", "001"}, {"SRLI", "101"}, {"SRAI", "101"}, {"ADD", "000"}, {"SUB", "000"},
        {"SLL", "001"}, {"SLT", "010"}, {"SLTU", "011"}, {"XOR", "100"}, {"SRL", "101"},
        {"SRA", "101"}, {"OR", "110"}, {"AND", "111"}
    };

    for (int i = 0; i < FUNCT3_MAP_SIZE; ++i) {
        if (strcmp(funct3_map[i][0], opcode) == 0) {
            return atoi(funct3_map[i][1]);
        }
    }
    return 0;
}

// Function to get the funct7 code
        int get_funct7(const char* opcode) {
    static const char* funct7_map[FUNCT7_MAP_SIZE][2] = {
        {"SRAI", "0100000"}, {"SUB", "0100000"}, {"SRA", "0100000"}
    };

    for (int i = 0; i < FUNCT7_MAP_SIZE; ++i) {
        if (strcmp(funct7_map[i][0], opcode) == 0) {
            return atoi(funct7_map[i][1]);
        }
    }
    return FUNCT7_DEFAULT;
}

int int2signedbin(int number, int digits) {
    if (number >= 0) {
        return number;
    } else {
        int xor_mask = (1 << (digits - 1)) - 1; // mask with ones for bit inversion
        int two_complement = (labs(number) ^ xor_mask) + 1; // 2's complements
        return two_complement;
    }
}

int asmToBinary(char *line) {
    char auxLine[MAX_LINE_LENGTH];
    strcpy(auxLine, line);

    // Remove leading/trailing whitespace and convert to uppercase
    char *p = auxLine;
    while (*p && isspace(*p)) p++;
    size_t len = strlen(p);
    while (len > 0 && isspace(p[len - 1])) len--;
    p[len] = '\0';
    for (char *q = p; *q; q++) {
        *q = toupper(*q);
    }

    // Split the line into words
    char *words[MAX_LINE_LENGTH];
    char *token = strtok(p, " ");
    int word_count = 0;
    while (token != NULL) {
        words[word_count++] = token;
        token = strtok(NULL, " ");
    }

    // Get the opcode
    char *opcode = words[0];
    // skips gotos and empty lines
    if (strcmp(opcode, "") == 0 || opcode[strlen(opcode) - 1] == ':') {
        return -1;
    }

    // Process pseudoinstructions
    if (strcmp(opcode, "MOV") == 0 || strcmp(opcode, "MV") == 0) {
        opcode = "ADDI";
        if ((words[2][0] >= '0') && (words[2][0] <= '9')) {
            words[0] = "ADDI";
            strcpy(words[3], words[2]);
            words[2] = "x0";
            
        } else {
            words[0] = "ADDI";
            words[3] = "0";
        }
        word_count++;
    } else if (strcmp(opcode, "NOP") == 0) {
        opcode = "ADDI";
        words[0] = "ADDI";
        words[1] = "x0";
        words[2] = "x0";
        words[3] = "0";
    }

    int bytecode;

    if (strcmp(opcode, "") == 0) {
        return -1;
    } else if (opcode[strlen(opcode) - 1] == ':') {
        // assert(gotos[opcode]); // Verify if goto corresponds to registered value
    } else {
        if (strcmp(opcode, "LUI") == 0) {
            int rd = atoi(&words[1][1]);
            int imm = atoi(words[2]);
            imm = int2signedbin(imm, 20);
            int opcode_number = 0b0110111;
            bytecode = ((imm & 0xFFFFF) << 12) | ((rd & 0x1F) << 7) | (opcode_number & 0x7F);
        } else if (strcmp(opcode, "AUIPC") == 0) {
            int rd = atoi(&words[1][1]);
            int imm = atoi(words[2]);
            if (words[2][0] == '-') imm *= -1;
            imm = int2signedbin(imm, 20);
            int opcode_number = 0b0010111;
            bytecode = ((imm & 0xFFFFF) << 12) | ((rd & 0x1F) << 7) | (opcode_number & 0x7F) ;
        } else if (strcmp(opcode, "JAL") == 0) {
            int rd = atoi(&words[1][1]);
            int imm = atoi(words[2]);
            imm = int2signedbin(imm, 30);
            int opcode_number = 0b1101111;
            // Generate bytecode
            int binary_imm = (((imm >> 1) & 0x3FF) << 21) | (((imm >> 11) & 0x1) << 20) | (((imm >> 12) & 0xFF) << 12) | (((imm >> 20) & 0x1) << 31);
            bytecode = binary_imm | ((rd & 0x1F) << 7) | (opcode_number & 0x7F);

        } else if (strcmp(opcode, "JALR") == 0) {
            int rd = atoi(&words[1][1]);
            int rs1 = atoi(&words[2][1]);
            int imm = atoi(words[3]);
            imm = int2signedbin(imm, 12);
            int opcode_number = 0b1100111;
            bytecode = ((imm & 0xFFF) << 20) | ((rs1 & 0x1F) << 15) | ((get_funct3(opcode) & 0x7) << 12) | ((rd & 0x1F) << 7) | (opcode_number & 0x7F);

        } else if (strcmp(opcode, "BEQ") == 0 || strcmp(opcode, "BNE") == 0 ||
                    strcmp(opcode, "BLT") == 0 || strcmp(opcode, "BGE") == 0 ||
                    strcmp(opcode, "BLTU") == 0 || strcmp(opcode, "BGEU") == 0) {
            int rs1 = atoi(&words[1][1]);
            int rs2 = atoi(&words[2][1]);
            int imm = atoi(words[3]);
            imm = int2signedbin(imm, 30);
            int opcode_number = 0b1100011;
            int binary_imm1 = (((imm >> 11) & 0x1) << 7) | (((imm >> 1) & 0xF) << 8);
            int binary_imm2 = (((imm >> 5) & 0x3F) << 25) | (((imm >> 12) & 0x1) << 31);
            bytecode = binary_imm1 | binary_imm2 | ((rs2 & 0x1F) << 20) | ((rs1 & 0x1F) << 15) | ((get_funct3(opcode) & 0x7) << 12) | (opcode_number & 0x7F);

        } else if (strcmp(opcode, "LB") == 0 || strcmp(opcode, "LH") == 0 ||
                    strcmp(opcode, "LW") == 0 || strcmp(opcode, "LBU") == 0 ||
                    strcmp(opcode, "LHU") == 0) {
            int rd = atoi(&words[1][1]);
            int rs1 = atoi(&words[2][1]);
            int imm = atoi(words[3]);
            imm = int2signedbin(imm, 12);
            int opcode_number = 0b0000011;
            bytecode = ((imm & 0xFFF) << 20) | ((rs1 & 0x1F) << 15) | ((get_funct3(opcode) & 0x7) << 12) | ((rd & 0x1F) << 7) | (opcode_number & 0x7F);

        } else if (strcmp(opcode, "SB") == 0 || strcmp(opcode, "SH") == 0 ||
                    strcmp(opcode, "SW") == 0) {
            int rs1 = atoi(&words[1][1]);
            int rs2 = atoi(&words[2][1]);
            int imm = atoi(words[3]);
            imm = int2signedbin(imm, 12);
            int opcode_number = 0b0100011;
            bytecode = ((imm & 0x7F) << 25) | ((rs2 & 0x1F) << 20) | ((rs1 & 0x1F) << 15) | ((get_funct3(opcode) & 0x7) << 12) | ((imm & 0x1F) << 7) | (opcode_number & 0x7F);

        } else if (strcmp(opcode, "ADDI") == 0 || strcmp(opcode, "SLTI") == 0 ||
                    strcmp(opcode, "SLTIU") == 0 || strcmp(opcode, "XORI") == 0 ||
                    strcmp(opcode, "ORI") == 0 || strcmp(opcode, "ANDI") == 0) {
            int rd = atoi(&words[1][1]);
            int rs1 = atoi(&words[2][1]);
            int imm = atoi(words[3]);
            imm = int2signedbin(imm, 12);
            int opcode_number = 0b0010011;
            bytecode = ((imm & 0xFFF) << 20) | ((rs1 & 0x1F) << 15) | ((get_funct3(opcode) & 0x7) << 12) | ((rd & 0x1F) << 7) | (opcode_number & 0x7F);

        } else if (strcmp(opcode, "SLLI") == 0 || strcmp(opcode, "SRLI") == 0 ||
                    strcmp(opcode, "SRAI") == 0) {
            int rd = atoi(&words[1][1]);
            int rs1 = atoi(&words[2][1]);
            int imm = atoi(words[3]);
            // Ensure imm >= for shifts
            assert(imm >= 0);
            int opcode_number = 0b0010011;
            bytecode = (get_funct7(opcode) & 0x7F) | ((imm & 0x1F) << 7) | ((rs1 & 0x1F) << 12) | ((get_funct3(opcode) & 0x7) << 17) | ((rd & 0x1F) << 20) | ((opcode_number & 0x7F) << 25);

        } else if (strcmp(opcode, "ADD") == 0 || strcmp(opcode, "SUB") == 0 ||
                    strcmp(opcode, "SLL") == 0 || strcmp(opcode, "SLT") == 0 ||
                    strcmp(opcode, "SLTU") == 0 || strcmp(opcode, "XOR") == 0 ||
                    strcmp(opcode, "SRL") == 0 || strcmp(opcode, "SRA") == 0 ||
                    strcmp(opcode, "OR") == 0 || strcmp(opcode, "AND") == 0) {
            int rd = atoi(&words[1][1]);
            int rs1 = atoi(&words[2][1]);
            int rs2 = atoi(&words[3][1]);
            int opcode_number = 0b0110011;
            bytecode = (get_funct7(opcode) & 0x7F) | ((rs2 & 0x1F) << 20) | ((rs1 & 0x1F) << 15) | ((get_funct3(opcode) & 0x7) << 12) | ((rd & 0x1F) << 7) | (opcode_number & 0x7F);
        }
    }
    return bytecode;
}