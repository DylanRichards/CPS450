/* I pledge my Honor that I have not cheated, and will not cheat, on this assignment. Dylan Richards */
/* DJ PARSER */

%code provides {
  #include "lex.yy.c"

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
    {return 0;}
;

class_decl_list:
      class_decl_list class_decl
    |
;

class_decl:
      CLASS id_exp EXTENDS id_exp LBRACE var_decl_list method_decl_list RBRACE
    | CLASS id_exp EXTENDS id_exp LBRACE var_decl_list RBRACE
;

method_decl_list:
      method_decl_list method_decl
    | method_decl
;

method_decl:
      var_decl LPAREN par_dec_list RPAREN LBRACE var_decl_list exp_list RBRACE 
;

par_dec_list:
      par_dec
    |
;

par_dec:
      par_dec COMMA var_decl
    | var_decl
;

var_decl_list:
      var_decl_list var_decl SEMICOLON
    |
;

var_decl:
      ntyp id_exp
    | id_exp id_exp
;

exp_list:
      exp_list exp SEMICOLON
    | exp SEMICOLON
;

exp:
      exp PLUS exp
    | exp MINUS exp
    | exp TIMES exp
    | exp EQUALITY exp
    | exp LESS exp
    | NOT exp
    | exp AND exp
    | NATLITERAL
    | NUL
    | IF LPAREN exp RPAREN LBRACE exp_list RBRACE ELSE LBRACE exp_list RBRACE
    | WHILE LPAREN exp RPAREN LBRACE exp_list RBRACE
    | NEW id_exp LPAREN RPAREN
    | THIS
    | PRINTNAT LPAREN exp RPAREN
    | READNAT LPAREN RPAREN
    | id_exp
    | exp DOT id_exp
    | id_exp ASSIGN exp
    | exp DOT id_exp ASSIGN exp
    | id_exp LPAREN arg_list RPAREN
    | exp DOT id_exp LPAREN arg_list RPAREN
    | LPAREN exp RPAREN
;

arg_list:
      arg
    |
;

arg:
      arg COMMA exp
    | exp
;
      
id_exp:
      ID
;

ntyp:
      NATTYPE
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
  return yyparse();
}
