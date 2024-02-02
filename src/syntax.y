%token ERROR NEWLINE // internal
%token IF ELSE WHILE // control
%token VOID INT // types
%token RETURN // others
%token PLUS MINUS TIMES OVER ASSIGN EQ LT GT LEQ GEQ DIFFERENT SEMICOLON
%token LPAREN RPAREN
%token LBRACKET RBRACKET
%token LBRACES RBRACES
%token LCOMMENT RCOMMENT
%token ID NUM
%start program

%%
program: assignment
var: ID | ID LBRACKET NUM RBRACKET {printf("variable");}
assignment: var ASSIGN NUM SEMICOLON {printf("assignment");}
%%