CC = gcc

SRCS = dism.tab.c ast.c interp.c

sim-dism: $(SRCS)
	$(CC) -w $(SRCS) -o $@

.PHONY: clean
clean:
	$(RM) sim-dism