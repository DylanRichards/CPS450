
%option yylineno
%option noyywrap

%{
  #define DEBUG 1
  typedef int Token;
  Token scanned(Token t);
%}

wspace	[ \t\r\n]+

%%

{wspace}    { /*skip whitespace*/ }
<<EOF>>     {return scanned(ENDOFFILE);}
.           { if(DEBUG) printf("\n");
             printf("Lex error on line %d: Illegal character %s\n",
                    yylineno,yytext);
             exit(-1);
            }
%%

/* process and return scanned token t */
Token scanned(Token t) {
  if(DEBUG==0) return t;
  switch(t) {
    case ENDOFFILE: printf("ENDOFFILE\n"); return t;
    default: printf("ERROR: invalid token in scanned().\n"); exit(-1);
  }
}

 
