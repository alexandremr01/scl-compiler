%start program
%debug
%{
    #include "ast.h"
    static int yyerror(char*);
    extern int yylex();
    extern int yylineno;
    AbstractSyntaxTree *astree;
    extern int getToken();
    extern int syntaxErrors;
    extern char* yytext;
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
%type <node> simple_exp sum_exp term factor call var args arg_list
%type <node> parameters parameter_list parameter

%token ERROR NEWLINE // internal
%token IF ELSE WHILE // control
%token VOID INT // types
%token RETURN // others
%token PLUS MINUS TIMES OVER ASSIGN EQ LT GT LEQ GEQ DIFFERENT 
%token SEMICOLON COMMA
%token LPAREN RPAREN
%token LBRACKET RBRACKET
%token LBRACES RBRACES
%token <string> ID NUM 

%%
program: units { 
    ASTNode *root = newASTNode(ROOT_NODE);
    root->firstChild = $1;
    astree = newAbstractSyntaxTree(root);
}

units: external_declaration { $$ = $1; }
    | units external_declaration { $$ = appendSibling($1, $2);}

external_declaration: function_definition { $$ = $1; }
                    | declaration { $$ = $1; }

function_definition: type_specifier ID LPAREN parameters RPAREN compound_statement {
    $$ = newASTNode(FUNCTION_DEFINITION_NODE);
    $$->type = $1;
    $$->name = $2; 
    $$->firstChild = $6;
    $6->sibling = $4;
    $$->line_number = yylineno;
}
parameters: parameter_list {$$=$1;} | %empty {$$=NULL;}
parameter_list: parameter_list COMMA parameter {$$ = appendSibling($1, $3);}| parameter {$$=$1;} 
parameter: type_specifier ID  { 
        $$ = newASTNode(DECLARATION_NODE);
        $$->type = $1;
        $$->name = $2; 
        $$->line_number = yylineno;
    }
 | type_specifier ID LBRACKET NUM RBRACKET SEMICOLON  { 
        $$ = newASTNode(DECLARATION_NODE);
        $$->type = $1;
        $$->name = $2; 
        $$->line_number = yylineno;
        // TODO: treat array
    }

type_specifier: INT {$$ = INTEGER_TYPE;} | VOID {$$ = VOID_TYPE;} 
declaration: type_specifier ID SEMICOLON  { 
                $$ = newASTNode(DECLARATION_NODE);
                $$->type = $1;
                $$->name = $2; 
                $$->line_number = yylineno;
            }
            | type_specifier ID LBRACKET NUM RBRACKET SEMICOLON  { 
                $$ = newASTNode(DECLARATION_NODE);
                $$->type = $1;
                $$->name = $2; 
                $$->line_number = yylineno;
                // TODO: treat array
            }
            | error SEMICOLON {yyerrok; $$ = NULL;}
/* expression_list: expression_list expression_statement | expression_statement */

statement_list: statement_list statement { $$ = appendSibling($1, $2);}
                | %empty {$$=NULL;}
declaration_list: declaration_list declaration { $$ = appendSibling($1, $2);}
                | %empty {$$=NULL;}
compound_statement: LBRACES declaration_list statement_list RBRACES {
    $$ = newASTNode(COMPOUND_STATEMENT_NODE);
    $$->line_number = yylineno;
    
    if ($2 == NULL) 
        $$->firstChild = $3;
    else { 
        ASTNode *p = $2;
        while(p->sibling != NULL) 
            p = p->sibling;
        p->sibling = $3;
        $$->firstChild = $2;
    }
}

statement: expression_statement {$$=$1;}
            | compound_statement {$$=$1;}
            | selection_statement {$$=$1;}
            | iteration_statement {$$=$1;}
            | jump_statement {$$=$1;}
            | error SEMICOLON {yyerrok; $$ = NULL;}
expression_statement: expression SEMICOLON {$$=$1;} | SEMICOLON {$$ = newASTNode(EMPTY_NODE);}
// TODO: for now, if and while only accept statements
selection_statement: IF LPAREN expression RPAREN statement {
                    $$ = newASTNode(IF_NODE);
                    $$->firstChild = $3;
                    $$->firstChild->sibling = $5;
                    $$->line_number = yylineno;
                } 
                    | IF LPAREN expression RPAREN statement ELSE statement{
                    $$ = newASTNode(IF_NODE);
                    $$->firstChild = $3;
                    $3->sibling = $5;
                    $5->sibling = $7;
                    $$->line_number = yylineno;
                } 
iteration_statement: WHILE LPAREN expression RPAREN statement {
                    $$ = newASTNode(WHILE_NODE);
                    $$->firstChild = $3;
                    $3->sibling = $5;
                    $$->line_number = yylineno;
                } 
jump_statement: RETURN SEMICOLON {$$ = newASTNode(RETURN_NODE);} 
                | RETURN expression SEMICOLON {
                    $$ = newASTNode(RETURN_NODE);
                    $$->firstChild = $2;
                    $$->line_number = yylineno;
                } 

expression: var ASSIGN expression {
                $$ = newASTNode(ASSIGNMENT_NODE);
                $$->firstChild = $1;
                $1->sibling = $3;
                $$->line_number = yylineno;
                $$->type = VOID_TYPE;
            } 
            | simple_exp { $$ = $1; }
var:        ID {
                $$ = newASTNode(VAR_REFERENCE_NODE);
                $$->name = $1;
                $$->line_number = yylineno;
            }
            | ID LBRACKET NUM RBRACKET {
                $$ = newASTNode(VAR_REFERENCE_NODE);
                $$->name = $1;
                $$->line_number = yylineno;
                // TODO: how to treat vector access??
            }
simple_exp: sum_exp relational sum_exp {
                $$ = newASTNode(EXPRESSION_NODE);
                $$->firstChild = $1;
                $1->sibling = $3;
                $$->line_number = yylineno;
            } | sum_exp {$$ = $1;}
relational: LT | GT | LEQ | GEQ | EQ | DIFFERENT
sum_exp:    sum_exp sum term {
                $$ = newASTNode(SUM_NODE);
                $$->firstChild = $1;
                $1->sibling = $3;
                $$->line_number = yylineno;
            }| term {$$ = $1;}
sum:        PLUS | MINUS
term:       term mult_op factor {
                $$ = newASTNode(MULTIPLICATION_NODE);
                $$->firstChild = $1;
                $1->sibling = $3;
                $$->line_number = yylineno;
            }| factor {$$ = $1;}
mult_op:    TIMES | OVER
factor:     LPAREN expression RPAREN {$$=$2;}
             | var {$$=$1;}
             | NUM {
                $$ = newASTNode(CONSTANT_NODE);
                $$->name = $1;
                $$->line_number = yylineno;
                $$->type = INTEGER_TYPE;
            }
             | call {$$=$1;}
call:       ID LPAREN args RPAREN {
                $$ = newASTNode(CALL_NODE);
                $$->name = $1;
                $$->firstChild = $3;
                $$->line_number = yylineno;
            }
args:       arg_list {$$=$1;} | %empty {$$=NULL;}
arg_list:   arg_list COMMA expression { $$ = appendSibling($1, $3); }
            | expression  {$$=$1;} 

%%
int yyerror(char* message) { 
    printf("Line %d: Error at token \'%s\'.\n",yylineno,yytext);
    syntaxErrors += 1;
    return 0;
}
AbstractSyntaxTree* parse(){
    yyparse();
    return astree;
}