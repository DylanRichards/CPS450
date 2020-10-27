/* I pledge my Honor that I have not cheated, and will not cheat, on this assignment. Dylan Richards */
/* DJ PARSER */

%code provides {
  #include "lex.yy.c"
  #include "ast.h"
  #include "symtbl.h"
  #include "typecheck.h"

  ASTree *pgmAST;
  #define YYSTYPE ASTree *

  #define DEBUG_PARSER 0

  /* Function for printing generic syntax-error messages */
  void yyerror(const char *str) {
    printf("Syntax error on line %d at token %s\n",yylineno,yytext);
    printf("(This version of the compiler exits after finding the first ");
    printf("syntax error.)\n");
    exit(-1);
  }
}

%token MAIN CLASS EXTENDS NATTYPE IF ELSE WHILE
%token PRINTNAT READNAT THIS NEW NUL NATLITERAL 
%token ID ASSIGN PLUS MINUS TIMES EQUALITY LESS
%token AND NOT DOT SEMICOLON COMMA LBRACE RBRACE 
%token LPAREN RPAREN ENDOFFILE

%start pgm

%right ASSIGN
%left AND
%left EQUALITY LESS
%left PLUS MINUS
%left TIMES
%right NOT
%left DOT

%%

pgm: class_decl_list MAIN LBRACE var_decl_list exp_list RBRACE ENDOFFILE
    {
      $$ = newAST(PROGRAM, $1, 0, NULL, yylineno);
      appendToChildrenList($$, $4);
      appendToChildrenList($$, $5);
      pgmAST = $$;
      return 0;
    }
;

class_decl_list:
      class_decl_list class_decl
        {
          appendToChildrenList($1, $2);
        }
    |
        {
          $$ = newAST(CLASS_DECL_LIST, NULL, 0, NULL, yylineno);
        }
;

class_decl:
      CLASS id_exp EXTENDS id_exp LBRACE var_decl_list method_decl_list RBRACE
        {
          $$ = newAST(CLASS_DECL, $2, 0, NULL, yylineno);
          appendToChildrenList($$, $4);
          appendToChildrenList($$, $6);
          appendToChildrenList($$, $7);
        }
    | CLASS id_exp EXTENDS id_exp LBRACE var_decl_list RBRACE
        {
          $$ = newAST(CLASS_DECL, $2, 0, NULL, yylineno);
          appendToChildrenList($$, $4);
          appendToChildrenList($$, $6);
          appendToChildrenList($$, newAST(METHOD_DECL_LIST, NULL, 0, NULL, yylineno));
        }
;

method_decl_list:
      method_decl_list method_decl
        {
          appendToChildrenList($1, $2);
        }
    | method_decl
        {
          $$ = newAST(METHOD_DECL_LIST, $1, 0, NULL, yylineno);
        }
;

method_decl:
      ntyp id_exp LPAREN par_dec_list RPAREN LBRACE var_decl_list exp_list RBRACE
        {
          $$ = newAST(METHOD_DECL, $1, 0, NULL, yylineno);
          appendToChildrenList($$, $2);
          appendToChildrenList($$, $4);
          appendToChildrenList($$, $7);
          appendToChildrenList($$, $8);
        }
    | id_exp id_exp LPAREN par_dec_list RPAREN LBRACE var_decl_list exp_list RBRACE
        {
          $$ = newAST(METHOD_DECL, $1, 0, NULL, yylineno);
          appendToChildrenList($$, $2);
          appendToChildrenList($$, $4);
          appendToChildrenList($$, $7);
          appendToChildrenList($$, $8);
        }
;

par_dec_list:
      par_dec
        {
          $$=$1;
        }
    |
        {
          $$ = newAST(PARAM_DECL_LIST, NULL, 0, NULL, yylineno);
        }
;

par_dec:
      par_dec COMMA par
        {
          appendToChildrenList($1, $3);
        }
    | par
        {
          $$ = newAST(PARAM_DECL, $1, 0, NULL, yylineno);
        }
;

par:
      ntyp id_exp
        {
          $$ = newAST(VAR_DECL, $1, 0, NULL, yylineno);
          appendToChildrenList($$, $2);
        }
    | id_exp id_exp
        {
          $$ = newAST(VAR_DECL, $1, 0, NULL, yylineno);
          appendToChildrenList($$, $2);
        }
;

var_decl_list:
      var_decl_list var_decl SEMICOLON
        {
          appendToChildrenList($$, $2);
        }
    |
        {
          $$ = newAST(VAR_DECL_LIST, NULL, 0, NULL, yylineno);
        }
;

var_decl:
      ntyp id_exp
        {
          $$ = newAST(VAR_DECL, $1, 0, NULL, yylineno);
          appendToChildrenList($$, $2);
        }
    | id_exp id_exp
        {
          $$ = newAST(VAR_DECL, $1, 0, NULL, yylineno);
          appendToChildrenList($$, $2);
        }
;

exp_list:
      exp_list exp SEMICOLON
        {
          appendToChildrenList($1, $2);
        }
    | exp SEMICOLON
        {
          $$ = newAST(EXPR_LIST, $1, 0, NULL, yylineno);
        }
;

exp:
      exp PLUS exp
        {
          $$ = newAST(PLUS_EXPR, $1, 0, NULL, yylineno);
          appendToChildrenList($$, $3);
        }
    | exp MINUS exp
        {
          $$ = newAST(MINUS_EXPR, $1, 0, NULL, yylineno);
          appendToChildrenList($$, $3);
        }
    | exp TIMES exp
        {
          $$ = newAST(TIMES_EXPR, $1, 0, NULL, yylineno);
          appendToChildrenList($$, $3);
        }
    | exp EQUALITY exp
        {
          $$ = newAST(EQUALITY_EXPR, $1, 0, NULL, yylineno);
          appendToChildrenList($$, $3);
        }
    | exp LESS exp
        {
          $$ = newAST(LESS_THAN_EXPR, $1, 0, NULL, yylineno);
          appendToChildrenList($$, $3);
        }
    | NOT exp
        {
          $$ = newAST(NOT_EXPR, $2, 0, NULL, yylineno);
        }
    | exp AND exp
        {
          $$ = newAST(AND_EXPR, $1, 0, NULL, yylineno);
          appendToChildrenList($$, $3);
        }
    | NATLITERAL
        {
          $$ = newAST(NAT_LITERAL_EXPR, NULL, atoi(yytext), NULL, yylineno);
        }
    | NUL
        {
          $$ = newAST(NULL_EXPR, NULL, 0, NULL, yylineno);
        }
    | IF LPAREN exp RPAREN LBRACE exp_list RBRACE ELSE LBRACE exp_list RBRACE
        {
          $$ = newAST(IF_THEN_ELSE_EXPR, $3, 0, NULL, yylineno);
          appendToChildrenList($$, $6);
          appendToChildrenList($$, $10);
        }
    | WHILE LPAREN exp RPAREN LBRACE exp_list RBRACE
        {
          $$ = newAST(WHILE_EXPR, $3, 0, NULL, yylineno);
          appendToChildrenList($$, $6);
        }
    | NEW id_exp LPAREN RPAREN
        {
          $$ = newAST(NEW_EXPR, $2, 0, NULL, yylineno);
        }
    | THIS
        {
          $$ = newAST(THIS_EXPR, NULL, 0, NULL, yylineno);
        }
    | PRINTNAT LPAREN exp RPAREN
        {
          $$ = newAST(PRINT_EXPR, $3, 0, NULL, yylineno);
        }
    | READNAT LPAREN RPAREN
        {
          $$ = newAST(READ_EXPR, NULL, 0, NULL, yylineno);
        }
    | id_exp
        {
          $$ = newAST(ID_EXPR, $1, 0, NULL, yylineno);
        }
    | exp DOT id_exp
        {
          $$ = newAST(DOT_ID_EXPR, $1, 0, NULL, yylineno);
          appendToChildrenList($$, $3);
        }
    | id_exp ASSIGN exp
        {
          $$ = newAST(ASSIGN_EXPR, $1, 0, NULL, yylineno);
          appendToChildrenList($$, $3);
        }
    | exp DOT id_exp ASSIGN exp
        {
          $$ = newAST(DOT_ASSIGN_EXPR, $1, 0, NULL, yylineno);
          appendToChildrenList($$, $3);
          appendToChildrenList($$, $5);
        }
    | id_exp LPAREN arg_list RPAREN
        {
          $$ = newAST(METHOD_CALL_EXPR, $1, 0, NULL, yylineno);
          appendToChildrenList($$, $3);
        }
    | exp DOT id_exp LPAREN arg_list RPAREN
        {
          $$ = newAST(DOT_METHOD_CALL_EXPR, $1, 0, NULL, yylineno);
          appendToChildrenList($$, $3);
          appendToChildrenList($$, $5);
        }
    | LPAREN exp RPAREN
        {
          $$ = $2;
        }
;

arg_list:
      arg
        {
          $$ = $1;
        }
    |
        {
          $$ = newAST(ARG_LIST, NULL, 0, NULL, yylineno);
        }
;

arg:
      arg COMMA exp
        {
          appendToChildrenList($1, $3);
        }
    | exp
        {
          $$ = newAST(ARG_LIST, $1, 0, NULL, yylineno);
        }
;
      
id_exp:
      ID
        {
          $$ = newAST(AST_ID, NULL, 0, yytext, yylineno);
        }
;

ntyp:
      NATTYPE
        {
          $$ = newAST(NAT_TYPE, NULL, 0, NULL, yylineno);
        }
;


%%

int main(int argc, char **argv) {
  if(argc!=2) {
    printf("Usage: dj-parse filename\n");
    exit(-1);
  }
  yyin = fopen(argv[1],"r");
  if(yyin==NULL) {
    printf("ERROR: could not open file %s\n",argv[1]);
    exit(-1);
  }
  /* parse the input program */
  yyparse();

  if(DEBUG_PARSER){
    printAST(pgmAST);
  }
  
  setupSymbolTables(pgmAST);
  printSymbolTables();
  typecheckProgram();
  return 0;
}
