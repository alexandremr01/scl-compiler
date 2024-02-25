digit       [0-9]
number      {digit}+
string      \".*\"
identifier  [a-zA-Z][0-9a-zA-Z_]*
newline     \n
whitespace  [ \t]+
single_line_comment    \/\/.*\n
multi_line_comment     \/\*.+\*\/
%{
#include "../datastructures/ast.h"
#include "syntax.tab.h"
%}
%option yylineno
%%
"if"            {return IF;}
"else"          {return ELSE;}
"while"         {return WHILE;}
"void"          {return VOID;}
"int"           {return INT;}
"return"        {return RETURN;}
"asm"           {return ASM;}
"+"             {return PLUS;}
"-"             {return MINUS;}
"*"             {return TIMES;}
"/"             {return OVER;}
"="             {return ASSIGN;}
"=="            {return EQ;}
"<"             {return LT;}
">"             {return GT;}
"<="            {return LEQ;}
">="            {return GEQ;}
";"             {return SEMICOLON;}
"!="            {return DIFFERENT;}
"("             {return LPAREN;}
")"             {return RPAREN;}
"["             {return LBRACKET;}
"]"             {return RBRACKET;}
"{"             {return LBRACES;}
"}"             {return RBRACES;}
","             {return COMMA;}
{string}        {
    // remove quotes: yyleng-2 characters + null terminator
    yylval.string = malloc(yyleng-1);
    // start from second character and copy all yyleng-1 from it
    memcpy(yylval.string, yytext+1, yyleng-2); 
    *(yylval.string+yyleng-2) = '\0';
    return STRING;
}
{number}       {
    yylval.string = malloc(yyleng + 1);   
    memcpy(yylval.string, yytext, yyleng + 1); 
    return NUM;
}
{identifier}    {
    yylval.string = malloc(yyleng + 1);   
    memcpy(yylval.string, yytext, yyleng + 1); 
    return ID;
}
{newline}      ;
{whitespace}    ;
{single_line_comment} ;
{multi_line_comment} ;
.               {return ERROR;}
%%


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
        case ASM: printf("Operator: asm\n"); break;
        case STRING: printf("String\n"); break;
        default: printf("Unknown token\n"); break;
    }
}

void lexic_parser_only(){
    int i=0;
    int line_number=-1;
    while ((i = yylex ())){
        if (yylineno != line_number) {
            line_number = yylineno;
            printf("\nLine %d\n", line_number);
        }
        printToken(i);
        if (i == ID || i == NUM || i == STRING)
            free(yylval.string);
    }
    return;
}