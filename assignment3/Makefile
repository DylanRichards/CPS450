FLEX = flex
BISON = bison
CC = gcc

BISONFLAGS = -v

dj-parser: dj.tab.c
	$(CC) $< -o $@

dj.tab.c: dj.y lex.yy.c
	$(BISON) $(BISONFLAGS) $<

lex.yy.c: dj.l
	$(FLEX) $<

.PHONY: clean
clean:
	$(RM) dj-parser *.output *.c