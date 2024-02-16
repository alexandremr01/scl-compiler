#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "frontend/semantic_analysis.h"
#include "asmWriter/riscv.h"
#include "datastructures/ast.h"
#include "generated/syntax.tab.h"

#include "backend/codegen.h"
#include "backend/linker.h"
#include "backend/register_mapping.h"

extern FILE *yyin;
extern int yylex (void);
extern AbstractSyntaxTree* parse();
int syntaxErrors = 0;

void printToken(yytoken_kind_t token) {
    switch (token) {
        case ERROR: printf("Error\n"); break;
        case NEWLINE: break;
        case IF: printf("Keyword: if\n"); break;
        case ELSE: printf("Keyword: else\n"); break;
        case WHILE: printf("Keyword: while\n"); break;
        case VOID: printf("Type: void\n"); break;
        case INT: printf("Type: int\n"); break;
        case RETURN: printf("Keyword: return\n"); break;
        case PLUS: printf("Operator: +\n"); break;
        case MINUS: printf("Operator: -\n"); break;
        case TIMES: printf("Operator: *\n"); break;
        case OVER: printf("Operator: /\n"); break;
        case ASSIGN: printf("Operator: =\n"); break;
        case EQ: printf("Operator: ==\n"); break;
        case LT: printf("Operator: <\n"); break;
        case GT: printf("Operator: >\n"); break;
        case LEQ: printf("Operator: <=\n"); break;
        case GEQ: printf("Operator: >=\n"); break;
        case DIFFERENT: printf("Operator: !=\n"); break;
        case SEMICOLON: printf("Symbol: ;\n"); break;
        case LPAREN: printf("Symbol: (\n"); break;
        case RPAREN: printf("Symbol: )\n"); break;
        case LBRACKET: printf("Symbol: [\n"); break;
        case RBRACKET: printf("Symbol: ]\n"); break;
        case LBRACES: printf("Symbol: {\n"); break;
        case RBRACES: printf("Symbol: }\n"); break;
        case ID: printf("Identifier\n"); break;
        case NUM: printf("Number\n"); break;
        default: printf("Unknown token\n"); break;
    }
}

void lexic_parser_only(){
    int i=0;
    int line_number=0;
    printf("Line 0\n");
    while ((i = yylex ())){
        if (i == NEWLINE) {
            line_number += 1;
            printf("\nLine %d\n", line_number);
        }
        printToken(i);
    }
    return;
}

char *appendAsm(char *filename){
    int filenameLength = strlen(filename);
    int newLength = filenameLength + 5;
    char *asmFileName = (char *) malloc(newLength*sizeof(char));
    strcpy(asmFileName, filename);
    strcat(asmFileName, ".asm");
    return asmFileName;
}

int main(int argc, char *argv[]) {
    int print_ast=0, includeASMComments=0, keepTemporaries=0;
    if (argc < 3) {
        printf("Usage: scl src_file output_file [--lexical_only] [--print_ast]\n");
        return 1;
    }

    // open files
    FILE *f_in = fopen(argv[1],"r");
    if (f_in == NULL) {
        printf("FATAL: Could not open %s \n", argv[1]);
        return 1;
    }
    else yyin = f_in;

    FILE *f_bin = fopen(argv[2],"wb");
    if (f_bin == NULL){
        printf("FATAL: Could not open %s \n", argv[2]);
        fclose(f_in);
        return 1;
    }

    char *asmFileName = appendAsm(argv[2]);
    FILE *f_asm = fopen(asmFileName,"w");
    if (f_asm == NULL){
        printf("FATAL: Could not open %s \n", argv[2]);
        free(asmFileName);
        return 1;
    }
    free(asmFileName);
    
    // parse command line arguments
    for (int i=3; i < argc; i++){
        if (strcmp(argv[i], "--lexical_only") == 0){
            lexic_parser_only();
            return 0;
        } else if (strcmp(argv[i], "--print_ast") == 0){
            print_ast = 1;
        } else if (strcmp(argv[i], "--debug") == 0){
            yydebug=1;
        } else if (strcmp(argv[i], "--asm_comments") == 0){
            includeASMComments=1;
        } else if (strcmp(argv[i], "--keep_temporaries") == 0){
            keepTemporaries=1;
        } else {
            printf("FATAL: unrecognized command-line option \'%s\'\n", argv[i]);
            return 1;
        }
    }

    // Frontend
    AbstractSyntaxTree *tree = parse();
    SymbolicTable* table = newSymbolicTable();
    int semanticErrors = semanticAnalysis(tree, table, 0);

    if (print_ast) {
        printf("\nAbstract Syntax Tree:\n");
        printTree(tree->root, 0);
        printf("\n\n");
    }

    int linkErrors = 0;
    int compileErrors = syntaxErrors + semanticErrors;
    IntermediateRepresentation *ir = NULL;
    if (compileErrors > 0) {
        printf("\n%d compile errors\n", compileErrors);
    } else {
        // if no compiler errors, runs backend: code generation + linking
        ir = codeGen(tree);
        linkErrors = link(ir);
        if (linkErrors > 0) 
            printf("\n%d linker errors\n", linkErrors);
    }
    
    // any error in the previous steps: free everything that was used and quit
    if (linkErrors || compileErrors) {
        if (ir!=NULL) freeIntermediateRepresentation(ir); 
        freeSymbolicTable(table);
        fclose(f_asm);
        fclose(f_bin);
        fclose(f_in);
        return 1;
    }

    RegisterMapping *registerMapping = keepTemporaries ? NULL : newRegisterMapping(ir);
    wirteIR(ir, f_asm, f_bin, registerMapping, includeASMComments);

    printf("Compilation successful\n");

    freeIntermediateRepresentation(ir); 
    freeSymbolicTable(table);
    freeRegisterMapping(registerMapping);

    fclose(f_asm);
    fclose(f_bin);
    fclose(f_in);

    return 0;
}
