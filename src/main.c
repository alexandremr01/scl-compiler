#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "semantic_analysis.h"
#include "ast.h"
#include "syntax.tab.h"

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
    FILE *f_in;
    int print_ast=0, print_symbolic_table=0;
    if (argc < 2) {
        printf("Usage: scl file [--lexical_only] \n");
        return 1;
    }
    if (f_in = fopen(argv[1],"r")) 
        yyin = f_in;
    else {
        printf("FATAL: Could not open %s \n", argv[1]);
        return 1;
    }
    for (int i=2; i < argc; i++){
        if (strcmp(argv[i], "--lexical_only") == 0){
            lexic_parser_only();
            return 0;
        } else if (strcmp(argv[i], "--print_ast") == 0){
            print_ast = 1;
        } else if (strcmp(argv[i], "--print_symbolic_table") == 0){
            print_symbolic_table = 1;
        } else if (strcmp(argv[i], "--debug") == 0){
            yydebug=1;
        } else {
            printf("FATAL: unrecognized command-line option \'%s\'\n", argv[i]);
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
    if (errors > 0) 
        printf("%d compile errors\n", errors);
    else printf("Compilation successful\n");

    return 0;
}
