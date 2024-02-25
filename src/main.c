#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "frontend/semantic_analysis.h"
#include "translator/riscv.h"
#include "datastructures/ast.h"
#include "generated/syntax.tab.h"

#include "backend/codegen.h"
#include "backend/linker.h"
#include "backend/register_assignment.h"

extern FILE *yyin;
extern int yylex (void);
extern AbstractSyntaxTree* parse();
int syntaxErrors = 0;
extern void lexic_parser_only();

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
    int asmDialect = 1;
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
        } else if (strcmp(argv[i], "--dialect=ABI") == 0){
            asmDialect=2;
        } else if (strcmp(argv[i], "--dialect=raw") == 0){
            asmDialect=1;
        } else {
            printf("FATAL: unrecognized command-line option \'%s\'\n", argv[i]);
            return 1;
        }
    }

    // Frontend
    AbstractSyntaxTree *tree = parse();
    SymbolicTable* table = newSymbolicTable();

    int compileErrors = syntaxErrors;
    if (compileErrors == 0 && tree != NULL) {
        compileErrors += semanticAnalysis(tree, table);

        if (print_ast) {
            printf("\nAbstract Syntax Tree:\n");
            printTree(tree->root, 0);
            printf("\n\n");
        }
    }

    // if tree=NULL should only happen if there is syntaxErrors>0. 
    // the second condition is only to double check
    if (compileErrors > 0 || tree == NULL) {
        printf("\n%d compile errors\n", compileErrors);
        freeSymbolicTable(table);
        fclose(f_asm);
        fclose(f_bin);
        fclose(f_in);
        return 1;
    } 

    // Backend
    IntermediateRepresentation *ir = codeGen(tree);
    int linkErrors = link(ir, table);
    if (linkErrors > 0) {
        printf("\n%d linker errors\n", linkErrors);
        freeIntermediateRepresentation(ir); 
        freeSymbolicTable(table);
        fclose(f_asm);
        fclose(f_bin);
        fclose(f_in);
        return 1;
    }

    RegisterAssignment *registerAssignment = keepTemporaries ? NULL : newRegisterAssignment(ir);
    ObjectCode *obj = translateIRToObj(ir, registerAssignment, includeASMComments, asmDialect);
    writeAssembly(obj, f_asm);
    writeBinary(obj, f_bin);

    printf("Compilation successful\n");

    freeObjectCode(obj);
    freeIntermediateRepresentation(ir); 
    freeSymbolicTable(table);
    freeRegisterAssignment(registerAssignment);

    fclose(f_asm);
    fclose(f_bin);
    fclose(f_in);

    return 0;
}
