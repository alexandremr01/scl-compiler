digit       [0-9]
number      {digit}+
letter      [a-zA-Z]
identifier  {letter}+
newline     \n
whitespace  [ \t]+
%{
#include "global.h"
%}
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
"/*"            {return LCOMMENT;}
"*/"            {return RCOMMENT;}
{number}        {return NUM;}
{identifier}    {return ID;}
{newline}       {return NEWLINE;}
{whitespace}    ;
.               {return ERROR;}
%%
