# Test dj.l

Compile
```console
$ flex dj.l
$ bison dj.y
$ gcc dj.tab.c -o dj-lex
```
Test DJ files
```console
$ ./dj-lex test.dj
```
