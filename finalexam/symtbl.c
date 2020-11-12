#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symtbl.h"

#define UNDEFINED_TYPE -3
#define NULL_TYPE -2
#define NATT_TYPE -1
#define OBJECT_TYPE 0

#define TRUE 1
#define FALSE 0

int getChildrenCount(ASTree *astTree) {
	int count = 0;
	ASTList *childListIterator = astTree->children;
	while (childListIterator != NULL && childListIterator->data != NULL) {
		count++;
		childListIterator = childListIterator->next;
	}

	return count;
}
VarDecl createVarDeclByAst(ASTree *t) {
	struct vdecls varDecl = { };
	varDecl.varName = t->children->next->data->idVal;
	varDecl.varNameLineNumber = t->children->next->data->lineNumber;
	varDecl.type = NATT_TYPE;
	varDecl.typeLineNumber = t->children->data->lineNumber;
	return varDecl;
}

VarDecl* createVarDeclListByAst(ASTree *t, int size) {
	int i;
	VarDecl *varDeclList = malloc(sizeof(VarDecl) * size);
	ASTList *childListIterator = t->children;
	for (i = 0; i < size; i++) {
		varDeclList[i] = createVarDeclByAst(childListIterator->data);
		childListIterator = childListIterator->next;
	}
	return varDeclList;
}

void setupSymbolTables(ASTree *fullProgramAST) {
	wholeProgram = fullProgramAST;
	mainExprs = fullProgramAST->children->next->data;
	numMainBlockLocals = getChildrenCount(fullProgramAST->children->data);
	mainBlockST = createVarDeclListByAst(fullProgramAST->children->data, numMainBlockLocals);
}

void printSymbolTables(){
    printf("========== Symbol Table ========== \n");

    printf("\nMain function has %d varibale(s)\n", numMainBlockLocals);
    for (int i = 0; i < numMainBlockLocals; i++)
    printf("  Varible \"%s\" has type %d\n",mainBlockST[i].varName, mainBlockST[i].type);
    printf("=================================== \n");

}
