%token ERROR NEWLINE // internal
%token IF ELSE WHILE // control
%token VOID INT // types
%token RETURN // others
%token PLUS MINUS TIMES OVER ASSIGN EQ LT GT LEQ GEQ DIFFERENT 
%token SEMICOLON COMMA
%token LPAREN RPAREN
%token LBRACKET RBRACKET
%token LBRACES RBRACES
%token ID NUM
%start unit
%debug
%{
static int yyerror(char*);
extern int yylex(void);
extern int yylineno;
%}

%%
unit: external_declaration | unit external_declaration 
external_declaration: function_definition | declaration
function_definition: type_specifier ID LPAREN RPAREN compound_statement 
type_specifier: INT | VOID
declaration: type_specifier ID SEMICOLON | type_specifier ID LBRACKET NUM RBRACKET SEMICOLON 
/* expression_list: expression_list expression_statement | expression_statement */

statement_list: statement_list statement | 
declaration_list: declaration_list declaration | 
compound_statement: LBRACES declaration_list statement_list RBRACES
statement: expression_statement | compound_statement | selection_statement | iteration_statement | jump_statement
expression_statement: expression SEMICOLON | SEMICOLON
// TODO: for now, if and while only accept statements
selection_statement: IF LPAREN expression RPAREN statement | IF LPAREN expression RPAREN statement ELSE statement
iteration_statement: WHILE LPAREN expression RPAREN statement
jump_statement: RETURN SEMICOLON | RETURN expression SEMICOLON

expression: var ASSIGN expression | simple_exp {}
var:        ID | ID LBRACKET NUM RBRACKET {}
simple_exp: sum_exp relational sum_exp | sum_exp
relational: LT | GT | LEQ | GEQ | EQ | DIFFERENT
sum_exp:    sum_exp sum term | term
sum:        PLUS | MINUS
term:       term mult_op factor | factor
mult_op:    TIMES | OVER
factor:     LPAREN expression RPAREN | var | NUM | call
call:       ID LPAREN args RPAREN
args:       arg_list | 
arg_list:   arg_list COMMA expression | expression

%%
int yyerror(char * message)
{ 
    printf("Syntax error at line %d\n",yylineno);
    return 0;
}
