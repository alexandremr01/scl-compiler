digit       [0-9]
number      {digit}+
letter      [a-zA-Z]
identifier  {letter}+
newline     \n
whitespace  [ \t]+
single_line_comment    \/\/.*
multi_line_comment     \/\*.+\*\/
%{
#include "ast.h"
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
{number}        {return NUM;}
{identifier}    {return ID;}
{newline}       ;
{whitespace}    ;
{single_line_comment} ;
{multi_line_comment} ;
.               {return ERROR;}
%%
