FLEX = flex
BISON = bison
PERL = perl
CC = gcc

BISONFLAGS = -v
PERLFLAGS = -pi -e

dj-tc: dj.tab.c ast.c symtbl.c typecheck.c
	$(PERL) $(PERLFLAGS) 's/extern YYSTYPE yylval;/ /' $<
	$(CC) $^ -o $@

dj.tab.c: dj.y lex.yy.c
	$(BISON) $(BISONFLAGS) $<

lex.yy.c: dj.l
	$(FLEX) $<


.PHONY: clean
clean:
	$(RM) dj-tc lex.yy.c dj.tab.c dj.output
