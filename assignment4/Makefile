FLEX = flex
BISON = bison
PERL = perl
CC = gcc

PERLFLAGS = -pi -e

dj-ast: dj.tab.c ast.c
	$(PERL) $(PERLFLAGS) 's/extern YYSTYPE yylval;/ /' $<
	$(CC) $^ -o $@

dj.tab.c: dj.y lex.yy.c
	$(BISON) $<

lex.yy.c: dj.l
	$(FLEX) $<


.PHONY: clean
clean:
	$(RM) dj-ast lex.yy.c dj.tab.c
