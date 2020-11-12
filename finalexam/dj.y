%code provides {
  #include <stdio.h>
  #include "lex.yy.c"
  #include "ast.h"
  #include "symtbl.h"
  #include "typecheck.h"
  #include "codegen.h"
  
  ASTree * pgmAST;
  #define YYSTYPE ASTree *
  
  /* Function for printing generic syntax-error messages */
  void yyerror(const char *str) {
    printf("Syntax error on line %d at token %s\n", yylineno,yytext);
    printf("Halting compiler.\n");
    exit(-1);
  }
}

%token MAIN NATTYPE IF ELSE WHILE
%token PRINTNAT READNAT NUL NATLITERAL
%token ID ASSIGN PLUS MINUS TIMES EQUALITY LESS
%token AND NOT SEMICOLON LBRACE RBRACE
%token LPAREN RPAREN ENDOFFILE
%token INCREMENT DECREMENT FOR

%start pgm

%right ASSIGN

%left AND
%left EQUALITY LESS
%left PLUS MINUS
%left TIMES
%right NOT
%left INCREMENT
%left DECREMENT

%%

pgm : MAIN LBRACE var_decl_list expr_list RBRACE ENDOFFILE
     {$$ = newAST(PROGRAM, NULL, 0, NULL, yylineno);
      appendToChildrenList($$, $3);
      appendToChildrenList($$, $4);
      pgmAST = $$;
      return 0;}
    ;

var_decl_list: var_decl_list var_decl SEMICOLON
              {appendToChildrenList($1, $2);}
             |
              {$$ = newAST(VAR_DECL_LIST, NULL, 0, NULL, yylineno);}
             ;

var_decl: n id_exp
         {$$ = newAST(VAR_DECL, $1, 0, NULL, yylineno);
         appendToChildrenList($$, $2);
         }
        | id_exp id_exp
         {$$ = newAST(VAR_DECL, $1, 0, NULL, yylineno);
         appendToChildrenList($$, $2);
         }
        ;
 
expr_list : expr_list exp SEMICOLON
           {appendToChildrenList($1, $2);}
          | exp SEMICOLON
           {$$ = newAST(EXPR_LIST, $1, 0, NULL, yylineno);}
          ;

exp : id_exp INCREMENT
      {
        $$ = newAST(INCREMENT_EXPR, $1, 0, NULL, yylineno);
      }
    | id_exp DECREMENT
      {
        $$ = newAST(DECREMENT_EXPR, $1, 0, NULL, yylineno);
      }
    | FOR LPAREN exp SEMICOLON exp SEMICOLON exp RPAREN LBRACE expr_list RBRACE
      {
        $$ = newAST(FOR_EXPR, $3, 0, NULL, yylineno);
        appendToChildrenList($$, $5);
        appendToChildrenList($$, $7);
        appendToChildrenList($$, $10);
      }
    | exp PLUS exp
     {$$ = newAST(PLUS_EXPR, NULL, 0, NULL, yylineno);
      appendToChildrenList($$, $1);
      appendToChildrenList($$, $3);}
    | exp MINUS exp
      {$$ = newAST(MINUS_EXPR, NULL, 0, NULL, yylineno);
      appendToChildrenList($$, $1);
      appendToChildrenList($$, $3);}
    | exp TIMES exp
     {$$ = newAST(TIMES_EXPR, $1, 0, NULL, yylineno);
     appendToChildrenList($$, $3);}
    | exp EQUALITY exp
     {$$ = newAST(EQUALITY_EXPR, $1, 0, NULL, yylineno);
     appendToChildrenList($$, $3);}
    | exp LESS exp
     {$$ = newAST(LESS_THAN_EXPR, $1, 0, NULL, yylineno);
      appendToChildrenList($$, $3);}
    | NOT exp
     {$$ = newAST(NOT_EXPR, $2, 0, NULL, yylineno); }
    | exp AND exp
     {$$ = newAST(AND_EXPR, $1, 0, NULL, yylineno);
     appendToChildrenList($$, $3);}
    | NATLITERAL
     {$$ = newAST(NAT_LITERAL_EXPR, NULL, atoi(yytext), NULL, yylineno);}
    | id_exp
     {$$ = newAST(ID_EXPR, $1, 0, NULL, yylineno);
     }
    | id_exp ASSIGN exp
     {$$ = newAST(ASSIGN_EXPR, $1, 0, NULL, yylineno);
      appendToChildrenList($$, $3);
     }
    | WHILE LPAREN exp RPAREN LBRACE expr_list RBRACE
     {$$ = newAST(WHILE_EXPR, $3, 0, NULL, yylineno);
      appendToChildrenList($$, $6);
     }
    | IF LPAREN exp RPAREN LBRACE expr_list RBRACE ELSE LBRACE expr_list RBRACE
     {$$ = newAST(IF_THEN_ELSE_EXPR, NULL, 0, NULL, yylineno);
      appendToChildrenList($$, $3);
      appendToChildrenList($$, $6);
      appendToChildrenList($$, $10);
     }
    | PRINTNAT LPAREN exp RPAREN
     {$$ = newAST(PRINT_EXPR, $3, 0, NULL, yylineno);}
    | LPAREN exp RPAREN
      {$$ = $2;}
    | NUL
     {$$ = newAST(NULL_EXPR, NULL, 0, NULL, yylineno);}
    | READNAT LPAREN RPAREN
     {$$ = newAST(READ_EXPR, NULL, 0, NULL, yylineno);}
    ;

id_exp : ID
        {$$ = newAST(AST_ID, NULL, 0, yytext, yylineno);}
       ;

n : NATTYPE
   {$$ = newAST(NAT_TYPE, NULL, 0, NULL, yylineno);}
  ;

%%

int main(int argc, char **argv) {
  if(argc!=2) {
    printf("Usage: dism-parse filename\n");
    exit(-1);
  }
  yyin = fopen(argv[1],"r");
  if(yyin==NULL) {
    printf("ERROR: could not open file %s\n",argv[1]);
    exit(-1);
  }
  
  yyparse();
  
  printAST(pgmAST);
  
  setupSymbolTables(pgmAST);
  
  printSymbolTables();
  
  typecheckProgram();
  
  int len = strlen(argv[1]);
  argv[1][len-2] = '\0';
  strcat(argv[1],"dism");
  FILE *fp;
  fp=fopen(argv[1], "w+");

  generateDISM(fp);
  
  return 0;
}

