# CPS450
CPS 450 Assignments

## Simulate DISM Program

To run a DISM program compile the simulater (you only need to do this once):
```console
$ cd sim-dism
$ flex dism.l
$ yacc dism.y
$ gcc -w dism.tab.c ast.c interp.c -osim-dism
```
Then execute sim-dism with the DISM file you want to run as the parameter.
Example:
```console
$ cd assignment1
$ ../sim-dism/sim-dism qr.dism
```
#### Make sure the DISM file has an empty line at the end. Simulator will not work otherwise!
