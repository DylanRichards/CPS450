# CPS 450 - Programming Language Concepts

## Simulate DISM Program

To run a DISM program compile the simulater (you only need to do this once):
```console
$ cd sim-dism
$ make
```
Then execute sim-dism with the DISM file you want to run as the parameter.
Example:
```console
$ cd assignment1
$ ../sim-dism/sim-dism qr.dism
```
#### Make sure the DISM file has an empty line at the end. Simulator will not work otherwise!

## Assignment 1
1. To get familiar with DJ and DISM
2. To implement small DJ and DISM programs for storing, retrieving, and dividing numbers.
3. To become familiar with and able to use a DISM simulator.

## Assignment 2
1. To become familiar with flex, a popular program used to generate lexical analyzers.
2. To implement a lexical analyzer for programs written in DJ.
3. To understand which strings constitute valid tokens in DJ.
4. To practice writing regular expressions by specifying valid DJ tokens.

## Assignment 3
1. To become familiar with bison (a popular, yacc-compatible parser generator)
2. To implement a parser for programs written in DJ.
3. To practice writing context-free grammars (CFGs) by specifying DJ’s grammar.

## Assignment 4
1. To gain experience using bison.
2. To understand the format of abstract syntax trees (ASTs) in DISM and DJ.
3. To implement an AST-building parser for DJ programs.
4. To practice writing semantic actions in bison CFGs.
