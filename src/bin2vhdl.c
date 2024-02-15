#include <stdio.h>
#include <stdlib.h>

// Function to convert a byte to its binary representation as a string
void byteToBinary(char byte, char *binaryStr) {
    for (int i = 7; i >= 0; --i) {
        binaryStr[i] = (byte & 1) + '0';
        byte >>= 1;
    }
    binaryStr[8] = '\0'; 
}

int main(int argc, char *argv[]) {
    FILE *inputFile, *outputFile;

    if (argc < 3) {
        printf("Usage: bin2vhdl bin_file output_file\n");
        return 1;
    }
    if (!(inputFile = fopen(argv[1],"r"))){
        printf("FATAL: Could not open %s \n", argv[1]);
        return 1;
    }
    if (!(outputFile = fopen(argv[2],"w"))) {
        fclose(inputFile);
        printf("FATAL: Could not open %s \n", argv[1]);
        return 1;
    }

    char byte;
    char binaryStr[9];
    int line_number = 0;

    // Read each byte from the binary file
    while (fread(&byte, sizeof(char), 1, inputFile)) {
        byteToBinary(byte, binaryStr);
        fprintf(outputFile, "Data_Rom(%d) <= \"%s\";\n", line_number, binaryStr);
        line_number++;
    }

    fclose(inputFile);
    fclose(outputFile);

    return 0;
}