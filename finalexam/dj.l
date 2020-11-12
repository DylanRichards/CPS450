
%option yylineno
%option noyywrap

%{
  #define DEBUG 0
  typedef int Token;
  Token scanned(Token t);
%}

digit	  [0-9]*
wspace	[ \t\r\n]+ 
id      [a-zA-Z][a-z0-9A-Z_]*

%%

main  {return scanned(MAIN);}
nat  {return scanned(NATTYPE);}
printNat {return scanned(PRINTNAT);}
readNat {return scanned(READNAT);}
if {return scanned(IF);}
else {return scanned(ELSE);}
while {return scanned(WHILE);}
null {return scanned(NUL);}
"{" {return scanned(LBRACE);}
"}" {return scanned(RBRACE);}
"(" {return scanned(LPAREN);}
")" {return scanned(RPAREN);}
";" {return scanned(SEMICOLON);}
"="  {return scanned(ASSIGN);}
"+"  {return scanned(PLUS);}
"&&" {return scanned(AND);}
"!" {return scanned(NOT);}
"-"  {return scanned(MINUS);}
"*"  {return scanned(TIMES);}
"==" {return scanned(EQUALITY);}
"<"  {return scanned(LESS);}
{digit}  {return scanned(NATLITERAL);}
{id}  {return scanned(ID);}




"//" { char c = input(); while(c!='\n' && c!=EOF) c=input();}
{wspace} { /*skip whitespace*/ }
<<EOF>> {return scanned(ENDOFFILE);}
. { if(DEBUG) printf("\n"); printf("Lex error on line %d: Illegal character %s\n", yylineno,yytext); exit(-1);}
%%

/* process and return scanned token t */
Token scanned(Token t) {
  if(DEBUG==0) return t;
  switch(t) {
    case MAIN: printf("MAIN "); return t;
    case NATTYPE: printf("NATTYPE "); return t;
    case PRINTNAT: printf("PRINTNAT "); return t;
    case READNAT: printf("READNAT "); return t;
    case NUL: printf("NUL "); return t;
    case ASSIGN: printf("ASSIGN "); return t;
    case LBRACE: printf("LBRACE "); return t;
    case RBRACE: printf("RBRACE "); return t;
    case LPAREN: printf("LPAREN "); return t;
    case RPAREN: printf("RPAREN "); return t;
    case AND: printf("AND "); return t;
    case NOT: printf("NOT "); return t;
    case IF: printf("IF "); return t;
    case ELSE: printf("ELSE "); return t;
    case WHILE: printf("WHILE "); return t;
    case ID: printf("ID(%s) ",yytext); return t;
    case PLUS: printf("PLUS "); return t;
    case MINUS: printf("MINUS "); return t;
    case TIMES: printf("TIMES "); return t;
    case EQUALITY: printf("EQUALITY "); return t;
    case LESS: printf("LESS "); return t;
    case NATLITERAL: printf("NATLITERAL(%s) ",yytext); return t;
    case SEMICOLON: printf("SEMICOLON "); return t;
    case ENDOFFILE: printf("ENDOFFILE\n"); return t;
    default: printf("ERROR: invalid token in scanned().\n"); exit(-1);
  }
}

 
