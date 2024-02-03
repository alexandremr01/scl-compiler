%start program
%debug
%{
    #include "ast.h"
    static int yyerror(char*);
    extern int yylex(void);
    extern int yylineno;
    AbstractSyntaxTree *astree;

%}
%union{
    DataType data_type;
    ASTNode *node;
    char* string;
}
%type  <data_type>  type_specifier
%type <node> program units external_declaration function_definition
%type <node> declaration declaration_list compound_statement statement_list expression
%type <node> statement expression_statement selection_statement iteration_statement jump_statement

%token ERROR NEWLINE // internal
%token IF ELSE WHILE // control
%token VOID INT // types
%token RETURN // others
%token PLUS MINUS TIMES OVER ASSIGN EQ LT GT LEQ GEQ DIFFERENT 
%token SEMICOLON COMMA
%token LPAREN RPAREN
%token LBRACKET RBRACKET
%token LBRACES RBRACES
%token <string> ID 
%token NUM 
%%
program: units { 
    ASTNode *root = newASTNode(ROOT_NODE);
    root->firstChild = $1;
    astree = newAbstractSyntaxTree(root);
}

units: external_declaration { $$ = $1; }
    | units external_declaration {
        ASTNode *p = $1;
        while(p->sibling != NULL) 
            p = p->sibling;
        p->sibling = $2; 
        $$ = $1;
    }

external_declaration: function_definition { $$ = $1; }
                    | declaration { $$ = $1; }

function_definition: type_specifier ID LPAREN RPAREN compound_statement {
    $$ = newASTNode(FUNCTION_DEFINITION_NODE);
    $$->type = $1;
    $$->name = $2; 
    $$->firstChild = $5;
}
type_specifier: INT {$$ = INTEGER_TYPE;} | VOID {$$ = VOID_TYPE;} 
declaration: type_specifier ID SEMICOLON  { 
                $$ = newASTNode(DECLARATION_NODE);
                $$->type = $1;
                $$->name = $2; 
            }
            | type_specifier ID LBRACKET NUM RBRACKET SEMICOLON  { 
                $$ = newASTNode(DECLARATION_NODE);
                $$->type = $1;
                $$->name = $2; 
                // TODO: treat array
            }
/* expression_list: expression_list expression_statement | expression_statement */

statement_list: statement_list statement {
    if ($1 == NULL) {
        $$ = $2;
    } else {
        ASTNode *p = $1;
        while(p->sibling != NULL) 
            p = p->sibling;
        p->sibling = $2; 
        $$ = $1;
    }
}| {$$=NULL;}
declaration_list: declaration_list declaration {
    if ($1 == NULL) {
        $$ = $2;
    } else {
        ASTNode *p = $1;
        while(p->sibling != NULL) 
            p = p->sibling;
        p->sibling = $2; 
        $$ = $1;
    }
}| {$$=NULL;}
compound_statement: LBRACES declaration_list statement_list RBRACES {
    if ($2 == NULL) 
        $$ = $3;
    ASTNode *p = $2;
    while(p->sibling != NULL) 
        p = p->sibling;
    p->sibling = $3;
    $$ = $2;
}

statement: expression_statement {$$=$1;}
            | compound_statement {$$=$1;}
            | selection_statement {$$=$1;}
            | iteration_statement {$$=$1;}
            | jump_statement {$$=$1;}
expression_statement: expression SEMICOLON {$$=$1;} | SEMICOLON {$$ = newASTNode(EMPTY_NODE);}
// TODO: for now, if and while only accept statements
selection_statement: IF LPAREN expression RPAREN statement {
                    $$ = newASTNode(IF_NODE);
                    $$->firstChild = $3;
                    $$->firstChild->sibling = $5;
                } 
                    | IF LPAREN expression RPAREN statement ELSE statement{
                    $$ = newASTNode(IF_NODE);
                    $$->firstChild = $3;
                    $3->sibling = $5;
                    $5->sibling = $7;
                } 
iteration_statement: WHILE LPAREN expression RPAREN statement {
                    $$ = newASTNode(WHILE_NODE);
                    $$->firstChild = $3;
                    $3->sibling = $5;
                } 
jump_statement: RETURN SEMICOLON {$$ = newASTNode(RETURN_NODE);} 
                | RETURN expression SEMICOLON {
                    $$ = newASTNode(RETURN_NODE);
                    $$->firstChild = $2;
                } 

expression: var ASSIGN expression {$$ = newASTNode(EXPRESSION_NODE);} 
            | simple_exp {$$ = newASTNode(EXPRESSION_NODE);}
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
int yyerror(char* message) { 
    printf("Syntax error at line %d\n",yylineno);
    return 0;
}
AbstractSyntaxTree* parse(){
    yyparse();
    return astree;
}