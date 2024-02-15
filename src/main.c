#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "semantic_analysis.h"
#include "asmWriter/riscv.h"
#include "ast.h"
#include "generated/syntax.tab.h"

#include "backend/codegen.h"
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

int main(int argc, char *argv[]) {
    FILE *f_in, *f_asm, *f_bin;
    int print_ast=0, print_symbolic_table=0, includeASMComments=0, keepTemporaries=0;
    if (argc < 3) {
        printf("Usage: scl src_file output_file [--lexical_only] [--print_ast] [--print_symbolic_table]\n");
        return 1;
    }
    if (f_in = fopen(argv[1],"r")) 
        yyin = f_in;
    else {
        printf("FATAL: Could not open %s \n", argv[1]);
        return 1;
    }
    char *asmFileName = (char *) malloc((strlen(argv[2]) + 5)*sizeof(char));
    strcpy(asmFileName, argv[2]);
    strcat(asmFileName, ".asm");
    if (!(f_asm = fopen(asmFileName,"w"))){
        printf("FATAL: Could not open %s \n", argv[2]);
        free(asmFileName);
        return 1;
    }
    if (!(f_bin = fopen(argv[2],"wb"))){
        printf("FATAL: Could not open %s \n", argv[2]);
        free(asmFileName);
        return 1;
    }
    for (int i=3; i < argc; i++){
        if (strcmp(argv[i], "--lexical_only") == 0){
            lexic_parser_only();
            return 0;
        } else if (strcmp(argv[i], "--print_ast") == 0){
            print_ast = 1;
        } else if (strcmp(argv[i], "--print_symbolic_table") == 0){
            print_symbolic_table = 1;
        } else if (strcmp(argv[i], "--debug") == 0){
            yydebug=1;
        } else if (strcmp(argv[i], "--asm_comments") == 0){
            includeASMComments=1;
        } else if (strcmp(argv[i], "--keep_temporaries") == 0){
            keepTemporaries=1;
        } else {
            printf("FATAL: unrecognized command-line option \'%s\'\n", argv[i]);
            free(asmFileName);
            return 1;
        }
    }

    AbstractSyntaxTree *tree = parse();

    SymbolicTable* table = newSymbolicTable();

    int semanticErrors = semanticAnalysis(tree, table, 0);

    // Debug utilities
    if (print_ast) {
        printf("\nAbstract Syntax Tree:\n");
        printTree(tree->root, 0);
        printf("\n\n");
    }
    if (print_symbolic_table) {
        printf("Symbolic Table:\n");
        printSymbolicTable(table);
        printf("\n\n");
    }

    int errors = syntaxErrors + semanticErrors;
    int code = errors>0;
    if (errors > 0) {
        printf("\n%d compile errors\n", errors);
    } else {
        IntermediateRepresentation *ir = codeGen(tree);

        RegisterMapping *rm = keepTemporaries ? NULL : newRegisterMapping(ir);

        printIR(ir, f_asm, f_bin, rm, includeASMComments);
        freeIntermediateRepresentation(ir);
        if (!keepTemporaries) freeRegisterMapping(rm);
        printf("Compilation successful\n");
    }
 
    freeSymbolicTable(table);
    free(asmFileName);  
    fclose(f_asm);
    fclose(f_bin);

    return code;
}
