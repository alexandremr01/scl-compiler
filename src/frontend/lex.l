digit       [0-9]
number      {digit}+
letter      [a-zA-Z]
string      \".*\"
identifier  {letter}+
newline     \n
whitespace  [ \t]+
single_line_comment    \/\/.*
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
{newline}       ;
{whitespace}    ;
{single_line_comment} ;
{multi_line_comment} ;
.               {return ERROR;}
%%

