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
	varDecl.type =
			(t->children->data->typ == NAT_TYPE) ?
					NATT_TYPE : typeNameToNumber(t->children->data->idVal);
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

MethodDecl createMethodDeclByAst(ASTree *t) {
	struct mdecls methodDecl = { };
	methodDecl.methodName = t->children->next->data->idVal;
	methodDecl.methodNameLineNumber = t->children->next->data->lineNumber;
	methodDecl.returnType = (t->children->data->typ == NAT_TYPE) ?
			NATT_TYPE : typeNameToNumber(t->children->data->idVal);
	methodDecl.returnTypeLineNumber = t->children->data->lineNumber;
    methodDecl.numParams = getChildrenCount(t->children->next->next->data);
	methodDecl.paramST = createVarDeclListByAst(t->children->next->next->data, methodDecl.numParams);
	methodDecl.numLocals = getChildrenCount(t->children->next->next->next->data);
	methodDecl.localST = createVarDeclListByAst(t->children->next->next->next->data, methodDecl.numLocals);
	methodDecl.bodyExprs = t->children->next->next->next->next->data;
	return methodDecl;
}

MethodDecl* createMethodDeclListByAst(ASTree *t, int size) {
	int i;
	MethodDecl *methodDeclList = malloc(sizeof(MethodDecl) * size);
	ASTList *childListIterator = t->children;
	for (i = 0; i < size; i++) {
		methodDeclList[i] = createMethodDeclByAst(childListIterator->data);
		childListIterator = childListIterator->next;
	}
	return methodDeclList;
}

ClassDecl createClassDeclByAst(ASTree *t) {
	struct classinfo classDecl = { };
	classDecl.className = t->children->data->idVal;
	classDecl.classNameLineNumber = t->children->data->lineNumber;
	classDecl.superclass = typeNameToNumber(t->children->next->data->idVal);
	classDecl.superclassLineNumber = t->children->next->data->lineNumber;
	classDecl.numVars = getChildrenCount(t->children->next->next->data);
	classDecl.varList = createVarDeclListByAst(t->children->next->next->data, classDecl.numVars);
	classDecl.numMethods = getChildrenCount(t->children->next->next->next->data);
	classDecl.methodList = createMethodDeclListByAst(t->children->next->next->next->data,
			classDecl.numMethods);

	return classDecl;
}

void setupClasses(ASTree *classDeclListAst) {
	int i;
	numClasses = getChildrenCount(classDeclListAst);
	numClasses++; /* +1 for object */

	classesST = malloc(sizeof(ClassDecl) * numClasses);

	struct classinfo objectDecl = { .className = "Object", .superclass = UNDEFINED_TYPE };
	classesST[OBJECT_TYPE] = objectDecl;

	ASTList *childListIterator = classDeclListAst->children;
	for (i = 1; i < numClasses; i++) {
		classesST[i] = createClassDeclByAst(childListIterator->data);
		childListIterator = childListIterator->next;
	}
}

void setupSymbolTables(ASTree *fullProgramAST) {
	wholeProgram = fullProgramAST;
	mainExprs = fullProgramAST->children->next->next->data;

	setupClasses(fullProgramAST->children->data);

	numMainBlockLocals = getChildrenCount(fullProgramAST->children->next->data);
	mainBlockST = createVarDeclListByAst(fullProgramAST->children->next->data, numMainBlockLocals);
}

int typeNameToNumber(char *className) {
	if (strcmp("Object", className) == 0) {
		return OBJECT_TYPE;
	}
	ASTree *classListAST = wholeProgram->children->data;
	ASTList *childListIterator = classListAST->children;
	int counter = 1;
	while (childListIterator != NULL && childListIterator->data != NULL) {
		if (strcmp(childListIterator->data->children->data->idVal, className) == 0) {
			return counter;
		}
		childListIterator = childListIterator->next;
		counter++;
	}
	return UNDEFINED_TYPE;
}


void printSymbolTables(){
    printf("========== Symbol Table ========== \n");
    for (int i = 1; i < numClasses; i++) {
        printf("Class \"%s\" has type %d\n",classesST[i].className, i);
        for(int j =0; j<classesST[i].numVars; j++)
        printf("  Varible \"%s\" has type %d\n",classesST[i].varList[j].varName,classesST[i].varList[j].type);
        for(int j =0; j<classesST[i].numMethods; j++){
            printf("  Method \"%s\"  has return type %d\n",classesST[i].methodList[j].methodName,classesST[i].methodList[j].returnType);
            for(int k = 0; k<classesST[i].methodList[j].numParams; k++)
            printf("    Parameter \"%s\" has type %d\n",classesST[i].methodList[j].paramST[k].varName, classesST[i].methodList[j].paramST[k].type);
            for(int k = 0; k<classesST[i].methodList[j].numLocals; k++)
            printf("    Local variable \"%s\" has type %d\n",classesST[i].methodList[j].localST[k].varName, classesST[i].methodList[j].localST[k].type);
        }
    }
    printf("\nMain function has %d varibale(s)\n", numMainBlockLocals);
    for (int i = 0; i < numMainBlockLocals; i++)
    printf("  Varible \"%s\" has type %d\n",mainBlockST[i].varName, mainBlockST[i].type);
    printf("=================================== \n");

}
