#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define FUNCT7_DEFAULT 0
#define FUNCT3_MAP_SIZE 50
#define FUNCT7_MAP_SIZE 14
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
        {"SRA", "101"}, {"OR", "110"}, {"AND", "111"}, {"MUL", "000"}, 
        {"FMV.W.X", "000"}, {"FLW", "010"}, {"FSW", "010"},
        {"FEQ.S", "010"}, {"FLT.S", "001"}, {"FLE.S", "000"}, {"FSGNJN.S", "001"},
        {"FSGNJ.S", "000"}, {"FADD.S", "000"}, {"FSUB.S", "000"}, {"FMUL.S", "000"},
        {"STORE", "011"}, {"SETPA", "001"}, {"SETPB", "010"}, {"MACC", "000"}, {"SETACC", "110"}
    };

    for (int i = 0; i < FUNCT3_MAP_SIZE; ++i) {
        if (strcmp(funct3_map[i][0], opcode) == 0) {
            return atoi(funct3_map[i][1]);
        }
    }
    return 0;
}

int get_register_number(const char* name) {
    if (name[0] == 'X' || name[0] == 'x') {
        return atoi(&name[1]);
    }
    // Mapping of ABI names to register numbers
    const char* registers[] = {
        "ZERO", "RA", "SP", "GP", "TP", "T0", "T1", "T2", "S0", "S1",
        "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7",
        "S2", "S3", "S4", "S5", "S6", "S7", "S8", "S9", "S10", "S11",
        "T3", "T4", "T5", "T6"
    };

    const char* floatingPointRegisters[] = {
        "FT0", "FT1", "FT2",  "FT3",  "FT4", "FT5", "FT6",  "FT7",
        "FS0", "FS1", "FA0",  "FA1",  "FA2", "FA3", "FA4",  "FA5",
        "FA6", "FA7", "FS2",  "FS3",  "FS4", "FS5", "FS6",  "FS7",
        "FS8", "FS9", "FS10", "FS11", "FT8", "FT9", "FT10", "FT11"
    };

    // Check each register name for a match
    for (int i = 0; i < 32; ++i) 
        if (strcmp(name, registers[i]) == 0)
            return i;

    // Check each register name for a match
    for (int i = 0; i < 32; ++i) 
        if (strcmp(name, floatingPointRegisters[i]) == 0)
            return i;

    return -1;
}

// Function to get the funct7 code
int get_funct7(const char* opcode) {
    static const char* funct7_map[FUNCT7_MAP_SIZE][2] = {
        {"SRAI", "0100000"}, {"SUB", "0100000"}, {"SRA", "0100000"}, {"MUL", "0000001"},
        {"FMV.W.X", "1111000"}, {"FSGNJ.S", "0010000"}, 
        {"FADD.S", "0000000"}, {"FSUB.S", "0000100"}, {"FMUL.S", "0001000"}, 
        {"FEQ.S", "1010000"}, {"FLT.S", "1010000"}, {"FLE.S", "1010000"},
        {"FSGNJN.S", "0010000"}, {"FCVT.S.W", "1101000"}
    };

    for (int i = 0; i < FUNCT7_MAP_SIZE; ++i) {
        if (strcmp(funct7_map[i][0], opcode) == 0) {
            return strtoul(funct7_map[i][1], NULL, 2);
        }
    }
    return FUNCT7_DEFAULT;
}

int int2signedbin(int number, int digits) {
    if (number >= 0) {
        return number;
    } else {
        int xor_mask = (1 << (digits)) - 1; // mask with ones for bit inversion
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
    char *token = strtok(p, "  (),");
    int word_count = 0;
    while (token != NULL) {
        words[word_count++] = token;
        token = strtok(NULL, " (),");
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
            words[2] = "X0";
            
        } else {
            words[0] = "ADDI";
            words[3] = "0";
        }
        word_count++;
    } else if (strcmp(opcode, "NOP") == 0) {
        opcode = "ADDI";
        words[0] = "ADDI";
        words[1] = "X0";
        words[2] = "X0";
        words[3] = "0";
    }

    int bytecode;
    if (strcmp(opcode, "") == 0) {
        return -1;
    } else if (opcode[strlen(opcode) - 1] == ':') {
        // assert(gotos[opcode]); // Verify if goto corresponds to registered value
    } else {
        if (strcmp(opcode, "LUI") == 0) {
            int rd = get_register_number(words[1]);
            int imm = atoi(words[2]);
            imm = int2signedbin(imm, 20);
            int opcode_number = 0b0110111;
            bytecode = ((imm & 0xFFFFF) << 12) | ((rd & 0x1F) << 7) | (opcode_number & 0x7F);
        } else if (strcmp(opcode, "AUIPC") == 0) {
            int rd = get_register_number(words[1]);
            int imm = atoi(words[2]);
            if (words[2][0] == '-') imm *= -1;
            imm = int2signedbin(imm, 20);
            int opcode_number = 0b0010111;
            bytecode = ((imm & 0xFFFFF) << 12) | ((rd & 0x1F) << 7) | (opcode_number & 0x7F) ;
        } else if (strcmp(opcode, "JAL") == 0) {
            int rd = get_register_number(words[1]);
            int imm = atoi(words[2]);
            imm = int2signedbin(imm, 30);
            int opcode_number = 0b1101111;
            // Generate bytecode
            int binary_imm = (((imm >> 1) & 0x3FF) << 21) | (((imm >> 11) & 0x1) << 20) | (((imm >> 12) & 0xFF) << 12) | (((imm >> 20) & 0x1) << 31);
            bytecode = binary_imm | ((rd & 0x1F) << 7) | (opcode_number & 0x7F);

        } else if (strcmp(opcode, "JALR") == 0) {
            int rd = get_register_number(words[1]);
            int rs1 = get_register_number(words[3]);
            int imm = atoi(words[2]);
            imm = int2signedbin(imm, 12);
            int opcode_number = 0b1100111;
            bytecode = ((imm & 0xFFF) << 20) | ((rs1 & 0x1F) << 15) | ((get_funct3(opcode) & 0x7) << 12) | ((rd & 0x1F) << 7) | (opcode_number & 0x7F);

        } else if (strcmp(opcode, "BEQ") == 0 || strcmp(opcode, "BNE") == 0 ||
                    strcmp(opcode, "BLT") == 0 || strcmp(opcode, "BGE") == 0 ||
                    strcmp(opcode, "BLTU") == 0 || strcmp(opcode, "BGEU") == 0) {
            int rs1 = get_register_number(words[1]);
            int rs2 = get_register_number(words[2]);
            int imm = atoi(words[3]);
            imm = int2signedbin(imm, 30);
            int opcode_number = 0b1100011;
            int binary_imm1 = (((imm >> 11) & 0x1) << 7) | (((imm >> 1) & 0xF) << 8);
            int binary_imm2 = (((imm >> 5) & 0x3F) << 25) | (((imm >> 12) & 0x1) << 31);
            bytecode = binary_imm1 | binary_imm2 | ((rs2 & 0x1F) << 20) | ((rs1 & 0x1F) << 15) | ((get_funct3(opcode) & 0x7) << 12) | (opcode_number & 0x7F);

        } else if (strcmp(opcode, "LB") == 0 || strcmp(opcode, "LH") == 0 ||
                    strcmp(opcode, "LW") == 0 || strcmp(opcode, "LBU") == 0 ||
                    strcmp(opcode, "LHU") == 0) {
            int rd = get_register_number(words[1]);
            int rs1 = get_register_number(words[3]);
            int imm = atoi(words[2]);
            imm = int2signedbin(imm, 12);
            int opcode_number = 0b0000011;
            bytecode = ((imm & 0xFFF) << 20) | ((rs1 & 0x1F) << 15) | ((get_funct3(opcode) & 0x7) << 12) | ((rd & 0x1F) << 7) | (opcode_number & 0x7F);

        } else if (strcmp(opcode, "SB") == 0 || strcmp(opcode, "SH") == 0 ||
                    strcmp(opcode, "SW") == 0) {
        
            int rs2 = get_register_number(words[1]);
            int rs1 = get_register_number(words[3]);
            int imm = atoi(words[2]);
            imm = int2signedbin(imm, 12);
            int opcode_number = 0b0100011;
            bytecode = (((imm >> 5) & 0x7F) << 25) | ((rs2 & 0x1F) << 20) | ((rs1 & 0x1F) << 15) | ((get_funct3(opcode) & 0x7) << 12) | ((imm & 0x1F) << 7) | (opcode_number & 0x7F);
        
        } else if (strcmp(opcode, "ADDI") == 0 || strcmp(opcode, "SLTI") == 0 ||
                    strcmp(opcode, "SLTIU") == 0 || strcmp(opcode, "XORI") == 0 ||
                    strcmp(opcode, "ORI") == 0 || strcmp(opcode, "ANDI") == 0) {
            int rd = get_register_number(words[1]);
            int rs1 = get_register_number(words[2]);
            int imm = atoi(words[3]);
            imm = int2signedbin(imm, 12);
            int opcode_number = 0b0010011;
            bytecode = ((imm & 0xFFF) << 20) | ((rs1 & 0x1F) << 15) | ((get_funct3(opcode) & 0x7) << 12) | ((rd & 0x1F) << 7) | (opcode_number & 0x7F);

        } else if (strcmp(opcode, "SLLI") == 0 || strcmp(opcode, "SRLI") == 0 ||
                    strcmp(opcode, "SRAI") == 0) {
            int rd = get_register_number(words[1]);
            int rs1 = get_register_number(words[2]);
            int imm = atoi(words[3]);
            // Ensure imm >= for shifts
            assert(imm >= 0);
            int opcode_number = 0b0010011;
            bytecode = ((get_funct7(opcode) & 0x7F) << 25) | ((imm & 0x1F) << 7) | ((rs1 & 0x1F) << 12) | ((get_funct3(opcode) & 0x7) << 17) | ((rd & 0x1F) << 20) | ((opcode_number & 0x7F) << 25);

        } else if (strcmp(opcode, "ADD") == 0 || strcmp(opcode, "SUB") == 0 ||
                    strcmp(opcode, "SLL") == 0 || strcmp(opcode, "SLT") == 0 ||
                    strcmp(opcode, "SLTU") == 0 || strcmp(opcode, "XOR") == 0 ||
                    strcmp(opcode, "SRL") == 0 || strcmp(opcode, "SRA") == 0 ||
                    strcmp(opcode, "OR") == 0 || strcmp(opcode, "AND") == 0 || strcmp(opcode, "MUL") == 0) {
            int rd = get_register_number(words[1]);
            int rs1 = get_register_number(words[2]);
            int rs2 = get_register_number(words[3]);
            int opcode_number = 0b0110011;
            bytecode = ((get_funct7(opcode) & 0x7F) << 25) | ((rs2 & 0x1F) << 20) | ((rs1 & 0x1F) << 15) | ((get_funct3(opcode) & 0x7) << 12) | ((rd & 0x1F) << 7) | (opcode_number & 0x7F);
        } else if (strcmp(opcode, "FMV.W.X") == 0 || strcmp(opcode, "FCVT.S.W") == 0 ) {
            int rd = get_register_number(words[1]);
            int rs1 = get_register_number(words[2]);
            int rs2 = 0;
            int opcode_number = 0b1010011;
            bytecode = ((get_funct7(opcode) & 0x7F) << 25) | ((rs2 & 0x1F) << 20) | ((rs1 & 0x1F) << 15) | ((get_funct3(opcode) & 0x7) << 12) | ((rd & 0x1F) << 7) | (opcode_number & 0x7F);
        } else if (strcmp(opcode, "CSRRW") == 0) {
            int rd = get_register_number(words[1]);
            int csr = atoi(words[2]);
            int rs1 = get_register_number(words[3]);
            int opcode_number = 0b1110011;
            bytecode = (csr << 20) | ((rs1 & 0x1F) << 15) | ((0b001 & 0x7) << 12) | ((rd & 0x1F) << 7) | (opcode_number & 0x7F);
        } else if (strcmp(opcode, "FSGNJ.S") == 0 || strcmp(opcode, "FADD.S") == 0 || 
                    strcmp(opcode, "FSUB.S") == 0 || strcmp(opcode, "FMUL.S") == 0 ||
                    strcmp(opcode, "FEQ.S") == 0 || strcmp(opcode, "FLT.S") == 0 ||
                    strcmp(opcode, "FLE.S") == 0 || strcmp(opcode, "FSGNJN.S") == 0) {
            int rd = get_register_number(words[1]);
            int rs1 = get_register_number(words[2]);
            int rs2 = get_register_number(words[3]);
            int opcode_number = 0b1010011;
            bytecode = ((get_funct7(opcode) & 0x7F) << 25) | ((rs2 & 0x1F) << 20) | ((rs1 & 0x1F) << 15) | ((get_funct3(opcode) & 0x7) << 12) | ((rd & 0x1F) << 7) | (opcode_number & 0x7F);
        } else if (strcmp(opcode, "FLW") == 0) {
            int rd = get_register_number(words[1]);
            int rs1 = get_register_number(words[3]);
            int imm = atoi(words[2]);
            imm = int2signedbin(imm, 12);
            int opcode_number = 0b0000111;
            bytecode = ((imm & 0xFFF) << 20) | ((rs1 & 0x1F) << 15) | ((get_funct3(opcode) & 0x7) << 12) | ((rd & 0x1F) << 7) | (opcode_number & 0x7F);
        } else if (strcmp(opcode, "FSW") == 0) {
            int rs2 = get_register_number(words[1]);
            int rs1 = get_register_number(words[3]);
            int imm = atoi(words[2]);
            imm = int2signedbin(imm, 16);
            int opcode_number = 0b0100111;
            bytecode = (((imm >> 5) & 0x7F) << 25) | ((rs2 & 0x1F) << 20) | ((rs1 & 0x1F) << 15) | ((get_funct3(opcode) & 0x7) << 12) | ((imm & 0x1F) << 7) | (opcode_number & 0x7F);
        
        } else if (strcmp(opcode, "SETPA") == 0 || strcmp(opcode, "SETPB") == 0 || strcmp(opcode, "SETACC") == 0) {
            int imm = atoi(words[1]);
            imm = int2signedbin(imm, 20);
            int opcode_number = 0X0B;
            bytecode = (((imm) & 0xFFFFF) << 12) | ((get_funct3(opcode) & 0x7) << 7) | (opcode_number & 0x7F);
            printf("for instruction %s generating %X", opcode, bytecode);
        } else if (strcmp(opcode, "MACC") == 0) {
            int opcode_number = 0X0B;
            bytecode = opcode_number;
        } else if (strcmp(opcode, "STORE") == 0) {
            int imm = atoi(words[1]);
            imm = int2signedbin(imm, 20);
            int opcode_number = 0X0B;
            bytecode = (((imm) & 0xFFFFF) << 12) | ((get_funct3(opcode) & 0x7) << 7) | (opcode_number & 0x7F);        
        } 
    }
    return bytecode;
}