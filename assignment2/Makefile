FLEX = flex
BISON = bison
CC = gcc

dj-lex: dj.tab.c
	$(CC) $< -o $@

dj.tab.c: dj.y lex.yy.c
	$(BISON) $<

lex.yy.c: dj.l
	$(FLEX) $<

.PHONY: clean
clean:
	$(RM) dj-lex *.c