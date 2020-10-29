#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "typecheck.h"
#include "symtbl.h"

/* Define static types */
#define ERROR_TYPE -4
#define UNDEFINED_TYPE -3
#define NULL_TYPE -2
#define NAT_TYPE -1 
#define OBJECT_TYPE 0

#define TRUE 1
#define FALSE 0

/* =========== HELPER METHODS =========== */

/**
 * Check variable names are unique
 * Check variable type is >= -1 (NAT_TYPE)
 */
void checkVarDeclList(VarDecl *mainBlockST, int numMainBlockLocals){
	int i, j;
	for (i = 0; i < numMainBlockLocals - 1; i++){
		VarDecl *varDecl = &mainBlockST[i];

		for (j = i+1; j < numMainBlockLocals; j++){
			VarDecl *varDecl2 = &mainBlockST[j];
			if(strcmp(varDecl->varName, varDecl2->varName) == 0){
				printSemanticError("Var declared multiple times", varDecl->varNameLineNumber);
			}
		}
	}

	for (i = 0; i < numMainBlockLocals; i++){
		VarDecl *varDecl = &mainBlockST[i];
		if(varDecl->type < NAT_TYPE){
			printSemanticError("Undefined variable", varDecl->typeLineNumber);
		}
	}
}

int hasCycle(int cType) {
	ClassDecl *classDecl = &classesST[cType];

	if(strcmp(classDecl->className, "Object") != 0){
		while (classDecl->superclass != OBJECT_TYPE) {
			if (classDecl->superclass == cType) {
				return TRUE;
			} else {
				classDecl = &classesST[classDecl->superclass];
			}
		}	
	}
	return FALSE;
}

int join(int t1, int t2) {
	if (isSubtype(t1, t2) == TRUE) {
		return t2;
	} else if (isSubtype(t2, t1) == TRUE) {
		return t1;
	} else {
		if (t1 < OBJECT_TYPE) {
			return UNDEFINED_TYPE;
		}
		return join(classesST[t1].superclass, t2);
	}
}

void setStaticNums(ASTree *astTree, int classNum, int memberNum) {
	astTree->staticClassNum = classNum;
	astTree->staticMemberNum = memberNum;
}

int findIdTypeByClassAndMethod(ASTree *idAst, int classType, int methodType, ASTree *parentAst) {
	int i;
	if (classType <= OBJECT_TYPE) {
		return ERROR_TYPE;
	}

	if (methodType != ERROR_TYPE) {
		VarDecl *parDeclList = classesST[classType].methodList[methodType].paramST;
		int parNum = classesST[classType].methodList[methodType].numParams;
		for (i = 0; i < parNum; i++) {
			VarDecl *parDecl = &parDeclList[i];
			if (strcmp(parDecl->varName, idAst->idVal) == 0) {
				return parDecl->type;
			}
		}


		VarDecl *varDeclList = classesST[classType].methodList[methodType].localST;
		int varNum = classesST[classType].methodList[methodType].numLocals;
		for (i = 0; i < varNum; i++) {
			VarDecl *varDecl = &varDeclList[i];
			if (strcmp(varDecl->varName, idAst->idVal) == 0) {
				return varDecl->type;
			}
		}
	}

	VarDecl *varDeclList = classesST[classType].varList;
	int varNum = classesST[classType].numVars;
	for (i = 0; i < varNum; i++) {
		VarDecl *varDecl = &varDeclList[i];
		if (strcmp(varDecl->varName, idAst->idVal) == 0) {
			setStaticNums(parentAst, classType, i);
			return varDecl->type;
		}
	}
	return findIdTypeByClassAndMethod(idAst, classesST[classType].superclass, ERROR_TYPE, parentAst);
}

MethodDecl findMethodDeclByClassType(ASTree *idAst, int classType, ASTree *parentAst) {
	int i;
	if (classType <= OBJECT_TYPE) {
		printSemanticError("Undefined method call", idAst->lineNumber);
	}
	for (i = 0; i < classesST[classType].numMethods; i++) {
		MethodDecl *methodDecl = &classesST[classType].methodList[i];
		if (strcmp(methodDecl->methodName, idAst->idVal) == 0) {
			setStaticNums(parentAst, classType, i);
			return *methodDecl;
		}
	}
	return findMethodDeclByClassType(idAst, classesST[classType].superclass, parentAst);
}

int findFieldTypeByClass(int classType, ASTree *idAst, ASTree *parentAst) {
	int i;
	if (classType <= OBJECT_TYPE) {
		printSemanticError("Undefined method call", idAst->lineNumber);
	}
	for (i = 0; i < classesST[classType].numVars; i++) {
		VarDecl *varDecl = &classesST[classType].varList[i];
		if (strcmp(varDecl->varName, idAst->idVal) == 0) {
			setStaticNums(parentAst, classType, i);
			return varDecl->type;
		}
	}
	return findFieldTypeByClass(classesST[classType].superclass, idAst, parentAst);
}

/**
 * Check class has a super-class
 * Check variable names
 */
void checkVarDeclListForSuperClasses(VarDecl *varDeclList, int listSize, int superclassType){
	if (superclassType == OBJECT_TYPE){
		return;
	}
	
	int i, j;
	for(i = 0; i < listSize; i++){
		VarDecl *varDecl = &varDeclList[i];
		for (j = 0; j < classesST[superclassType].numVars; j++){
			VarDecl *varDecl2 = &classesST[superclassType].varList[j];
			if(strcmp(varDecl->varName, varDecl2->varName) == 0){
				printSemanticError("Var declared multiple times in a superclass", varDecl->varNameLineNumber);
			}
		}
	}
	
	checkVarDeclListForSuperClasses(varDeclList, listSize, classesST[superclassType].superclass);
}

/**
 * Check if superclass type is the same as the classtype
 * Check if superclass type is undefined
 */
void checkClassSuperType(ClassDecl classDecl, int classType){
	if(classDecl.superclass == classType){
		printSemanticError("Superclass should have different type", classDecl.superclassLineNumber);
	}

	if(classDecl.superclass < OBJECT_TYPE){
		printSemanticError("Undefined superclass type", classDecl.superclassLineNumber);
	}
}

/**
 * Check each method parameters
 * Check method return type
 * Check method names are unique
 * Check method variables are unique within method and parameter
 * Check method body expressions
 */
void checkClassMethod(ClassDecl classDecl, int classType){
	int i, j, k, m;

	for (i = 0; i < classDecl.numMethods; i++){
		MethodDecl *methodDecl = &classDecl.methodList[i];

		for (j = 0; j < methodDecl->numParams; j++){
			VarDecl *parDecl = &methodDecl->paramST[j];
			for (k = j+1; k < methodDecl->numParams; k++){
				VarDecl *parDecl2 = &methodDecl->paramST[k];
				if(strcmp(parDecl->varName, parDecl2->varName) == 0){
					printSemanticError("Same method arguments", methodDecl->methodNameLineNumber);
				}
			}

			if (parDecl->type < NAT_TYPE){
				printSemanticError("Method parameter should have valid type", parDecl->typeLineNumber);
			}
		}

		if(methodDecl->returnType < NAT_TYPE){
			printSemanticError("Method return parameter should have valid type", methodDecl->returnTypeLineNumber);
		}
	}

	for (i = 0; i < classDecl.numMethods - 1; i++){
		MethodDecl *methodDecl = &classDecl.methodList[i];
		for (j = i + 1; j < classDecl.numMethods; j++){
			MethodDecl *methodDecl2 = &classDecl.methodList[j];
			if(strcmp(methodDecl->methodName, methodDecl2->methodName) == 0){
				printSemanticError("Method declared multiple times", methodDecl->methodNameLineNumber);
			}
		}
	}

	for (i = 0; i < classDecl.numMethods; i++){
		MethodDecl *methodDecl = &classDecl.methodList[i];
		checkVarDeclList(methodDecl->localST, methodDecl->numLocals);

		for (j = 0; j < methodDecl->numLocals; j++){
			VarDecl *varDecl = &methodDecl->localST[j];
			for (k = 0; k < methodDecl->numParams; k++){
				VarDecl *parDecl = &methodDecl->paramST[k];
				if(strcmp(varDecl->varName, parDecl->varName) == 0){
					printSemanticError("Var declared multiple times as parameter", varDecl->varNameLineNumber);
				}
			}
		}

		int methodReturnType = typeExprs(methodDecl->bodyExprs, classType, i);
		if(isSubtype(methodReturnType, methodDecl->returnType) == FALSE){
			printSemanticError("Return type mismatch", methodDecl->returnTypeLineNumber);
		}
	}
}

void checkSuperClassMethods(ClassDecl classDecl, int classType, int superType){
	if(superType == OBJECT_TYPE){
		return;
	}

	int i, j, k;
	for (i = 0; i < classDecl.numMethods; i++){
		MethodDecl *methodDecl = &classDecl.methodList[i];
		for (j = 0; j < classesST[superType].numMethods; j++){
			MethodDecl *methodDecl2 = &classesST[superType].methodList[j];
			if (strcmp(methodDecl->methodName, methodDecl2->methodName) == 0){
				if (methodDecl->numParams != methodDecl2->numParams){
					printSemanticError("Super class method parameter count mismatch", methodDecl->methodNameLineNumber);
				}
			}

			for (k = 0; k < methodDecl->numParams; k++){
				VarDecl *parDecl = &methodDecl->paramST[k];
				VarDecl *parDecl2 = &methodDecl2->paramST[k];
				if(parDecl->type != parDecl2->type){
					printSemanticError("Super class method parameter type mismatch", parDecl->typeLineNumber);
				}
			}

			if (methodDecl->returnType != methodDecl2->returnType){
				printSemanticError("Super class method return type mismatch", methodDecl->returnTypeLineNumber);
			}
		}
		checkSuperClassMethods(classDecl, classType, classesST[superType].superclass);
	}
}


/** 
 * Check class names are unique
 * Check each class is well typed
 * - class variables
 * - variables in class's superclass are unique
 * - super class type
 * - check methods in class
 * - check method in super classes with same name and type
 */
void checkClasses(){
	int i, j;
	for (i = 0; i < numClasses - 1; i++){
		ClassDecl *class1 = &classesST[i];
		for (j = i + 1; j < numClasses; j++){
			ClassDecl *class2 = &classesST[j];
			if (strcmp(class1->className, class2->className) == 0){
				printSemanticError("Class name declared multiple times", class1->classNameLineNumber);
			}
		}
	}

	/* Object class is at position 0 */
	for (i = 1; i < numClasses; i++){
		ClassDecl *class1 = &classesST[i];
		checkVarDeclList(class1->varList, class1->numVars);
		checkVarDeclListForSuperClasses(class1->varList, class1->numVars, class1->superclass);
		checkClassSuperType(*class1, i);
		checkClassMethod(*class1, i);
		checkSuperClassMethods(*class1, i, class1->superclass);
	}
}

/* ====================================== */
/* ====================================== */
/* =========== TYPE CHECK PGM =========== */
/* ====================================== */
/* ====================================== */

void typecheckProgram() {
	/* === Level 3 === */
	checkClasses();
	/* === Level 2 === */
	checkVarDeclList(mainBlockST, numMainBlockLocals);
	/* === Level 1 === */
	/* typecheck the main block expressions */
	typeExprs(mainExprs, -1, -1);
}

/**
 * Returns the type of the expression AST in the given context.
 * Also sets t->staticClassNum and t->staticMemberNum attributes as needed.
 * If classContainingExpr < 0 then this expression is in the main block of
 * the program; otherwise the expression is in the given class.
 */
int typeExpr(ASTree *t, int classContainingExpr, int methodContainingExpr) {

	if (t->typ == EXPR_LIST) {
        // if the current note is an expre_list, then recursively call typeExprs function to check its children
		return typeExprs(t, classContainingExpr, methodContainingExpr);
	} else if (t->typ == NAT_LITERAL_EXPR) {
        // level 1
		return NAT_TYPE;
	} else if (t->typ == NULL_EXPR) {
        // level 1
		return NULL_TYPE;
	} else if (t->typ == PRINT_EXPR) {
        // level 1
		int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);

        if (t1 != NAT_TYPE) {
            printSemanticError("non-nat type in print", t->lineNumber);
        }
        
        return NAT_TYPE;
	} else if (t->typ == READ_EXPR) {
        // level 1
		return NAT_TYPE;
	} else if (t->typ == PLUS_EXPR) {
        // level 1
        int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
        int t2 = typeExpr(t->children->next->data, classContainingExpr, methodContainingExpr);

        if (t1 != NAT_TYPE || t2 != NAT_TYPE) {
            printSemanticError("non-nat type in plus", t->lineNumber);
        }
        
        return NAT_TYPE;
    } else if (t->typ == MINUS_EXPR) {
        // level 1
        int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
        int t2 = typeExpr(t->children->next->data, classContainingExpr, methodContainingExpr);

        if (t1 != NAT_TYPE || t2 != NAT_TYPE) {
            printSemanticError("non-nat type in minus", t->lineNumber);
        }
        
        return NAT_TYPE;
	} else if (t->typ == TIMES_EXPR) {
        // level 1
        int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
        int t2 = typeExpr(t->children->next->data, classContainingExpr, methodContainingExpr);

        if (t1 != NAT_TYPE || t2 != NAT_TYPE) {
            printSemanticError("non-nat type in times", t->lineNumber);
        }
        
        return NAT_TYPE;
	} else if (t->typ == LESS_THAN_EXPR) {
        // level 1
        int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
        int t2 = typeExpr(t->children->next->data, classContainingExpr, methodContainingExpr);

        if (t1 != NAT_TYPE || t2 != NAT_TYPE) {
            printSemanticError("non-nat type in less than", t->lineNumber);
        }
        
        return NAT_TYPE;
	} else if (t->typ == AND_EXPR) {
        // level 1
        int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
        int t2 = typeExpr(t->children->next->data, classContainingExpr, methodContainingExpr);

        if (t1 != NAT_TYPE || t2 != NAT_TYPE) {
            printSemanticError("non-nat type in and", t->lineNumber);
        }
        
        return NAT_TYPE;
	} else if (t->typ == NOT_EXPR) {
        // level 1
        int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);

        if (t1 != NAT_TYPE) {
            printSemanticError("non-nat type in not", t->lineNumber);
        }
        
        return NAT_TYPE;
	} else if (t->typ == EQUALITY_EXPR) {
        // level 1
        int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
        int t2 = typeExpr(t->children->next->data, classContainingExpr, methodContainingExpr);

        if (isSubtype(t1, t2) || isSubtype(t2, t1)) {
			return NAT_TYPE;
        }
		printSemanticError("type mismatch in equal", t->lineNumber);
	} else if (t->typ == IF_THEN_ELSE_EXPR) {
        // level 1
		int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
		int t2 = typeExpr(t->children->next->data, classContainingExpr, methodContainingExpr);
		int t3 = typeExpr(t->children->next->next->data, classContainingExpr, methodContainingExpr);

        if (t1 == NAT_TYPE && join(t2, t3) != UNDEFINED_TYPE) {
			return join(t2, t3);
        }

		printSemanticError("non-nat type in if-else expression", t->lineNumber);        
	} else if (t->typ == WHILE_EXPR) {
        // level 1
		int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);

        if (t1 != NAT_TYPE) {
            printSemanticError("non-nat type in while expression", t->lineNumber);
        }
        
        return NAT_TYPE;
	} else if (t->typ == ASSIGN_EXPR) {
        // level 2
		if(t->children->data->typ != AST_ID){
			printSemanticError("Non-id type in assignment", t->lineNumber);
		}
		int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
		int t2 = typeExpr(t->children->next->data, classContainingExpr, methodContainingExpr);

		if(isSubtype(t2, t1) == TRUE){
			return t1;
		}
		printSemanticError("Type mismatch in assignment", t->lineNumber);
    } else if (t->typ == ID_EXPR) {
        // level 2
		return typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
    } else if (t->typ == AST_ID) {
        // level 2
        int i;
        // Examing variable is in main function
        if (classContainingExpr < 0 && methodContainingExpr < 0) {
            for (i = 0; i < numMainBlockLocals; i++) {
                VarDecl *varDecl = &mainBlockST[i];
                if (strcmp(varDecl->varName, t->idVal) == 0) {
                    return varDecl->type;
                }
            }
        } else {
			// Examing variable is in classes
			// level 3
			int idType = findIdTypeByClassAndMethod(t, classContainingExpr, methodContainingExpr, t);
			if(idType > UNDEFINED_TYPE){
				return idType;
			}
        }
        printSemanticError("Undeclared var", t->lineNumber);
    } else if (t->typ == NEW_EXPR) {
        // level 3
		char *idName = t->children->data->idVal;
		int idType = typeNameToNumber(idName);
		if(idType == NULL_TYPE || idType == UNDEFINED_TYPE){
			printSemanticError("Creating undefined object", t->lineNumber);
		}
		return idType;
	} else if (t->typ == THIS_EXPR) {
        // level 3
		if (methodContainingExpr < 0 && classContainingExpr < 0){
			printSemanticError("Ill typed this expression", t->lineNumber);
		} else {
			return classContainingExpr;
		}
	} else if (t->typ == DOT_ASSIGN_EXPR) {
        // level 3
		int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
		if (t1 <= OBJECT_TYPE) {
			printSemanticError("Non-object field access", t->lineNumber);
		}
		if (t->children->next->data->typ != AST_ID){
			printSemanticError("non-id type in assignment", t->children->next->data->lineNumber);
		}
		int t2 = findFieldTypeByClass(t1, t->children->next->data, t);
		int t3 = typeExpr(t->children->next->next->data, classContainingExpr, methodContainingExpr);
		if(isSubtype(t3, t2) == TRUE){
			return t2;
		}
		printSemanticError("type mismatch in assignment", t->lineNumber);
	} else if (t->typ == METHOD_CALL_EXPR) {
        // level 3
		MethodDecl methodDecl = findMethodDeclByClassType(t->children->data, classContainingExpr, t);
		if (typeParams(t->children->next->data, classContainingExpr, methodContainingExpr, &methodDecl) == TRUE){
			return methodDecl.returnType;
		}
		printSemanticError("Method call type mismatch", t->lineNumber);
	} else if (t->typ == DOT_METHOD_CALL_EXPR) {
        // level 3
		int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
		if (t1 <= OBJECT_TYPE) {
			printSemanticError("Non-object field access", t->lineNumber);
		}
		
		MethodDecl methodDecl = findMethodDeclByClassType(t->children->next->data, t1, t);
		if (typeParams(t->children->next->next->data, classContainingExpr, methodContainingExpr, &methodDecl) == TRUE){
			return methodDecl.returnType;
		}
		printSemanticError("Method call type mismatch", t->lineNumber);
	} else if (t->typ == DOT_ID_EXPR) {
        // level 3
		int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
		if (t1 <= OBJECT_TYPE) {
			printSemanticError("Non-object field access", t->lineNumber);
		}
		return findFieldTypeByClass(t1, t->children->next->data, t);
    }

	return printSemanticError("Not a valid expression", t->lineNumber);
}

/* Returns nonzero iff sub is a subtype of super */
int isSubtype(int sub, int super) {
	if (sub == NULL_TYPE && (super == NULL_TYPE || super >= OBJECT_TYPE)) {
		return TRUE;
	} else if (sub == NAT_TYPE && super == NAT_TYPE) {
		return TRUE;
	} else if (sub >= OBJECT_TYPE && super == OBJECT_TYPE) {
		return TRUE;
	} else if (sub > UNDEFINED_TYPE && sub == super) {
		return TRUE;
	} else if (sub >= OBJECT_TYPE) {
		/* Walk on the symbol table */
		ClassDecl *classDecl = &classesST[sub];
		if (classDecl->superclass == super) {
			return TRUE;
		} else if (classDecl->superclass != OBJECT_TYPE) {
			if (hasCycle(sub) == FALSE) {
				return isSubtype(classDecl->superclass, super);
			} else {
				return printSemanticError("Cycle in super class declaration",
						classDecl->superclassLineNumber);
			}
		}
	}

	return FALSE;
}

int typeParams(ASTree *t, int classContainingExprs, int methodContainingExprs, MethodDecl *methodDecl) {
	int count = 0;

	ASTList *childListIterator = t->children;
	while (childListIterator != NULL) {
		if (childListIterator->data != NULL) {
			count++;
		}
		childListIterator = childListIterator->next;
		
	}

	if (count != methodDecl-> numParams) {
		return printSemanticError("Method paramters count mismatch",
						t->lineNumber);
	}

	if (methodDecl-> numParams == 0) {
		return TRUE;
	}
    
	ASTList *childListIterator2 = t->children;
	count = 0;

	while (childListIterator2 != NULL) {
		int t1 = typeExpr(childListIterator2->data, classContainingExprs, methodContainingExprs);
		VarDecl *varDecl = &methodDecl->paramST[count];
		if (isSubtype(t1, varDecl->type) == TRUE) {
			return TRUE;
		} else {
			printSemanticError("Method call type mismatch", t->lineNumber);
		}
		childListIterator2 = childListIterator2->next;
		count++;
	}

	return FALSE;
}

/* Returns the type of the EXPR_LIST AST in the given context. */
int typeExprs(ASTree *t, int classContainingExprs, int methodContainingExprs) {
	int returnType;

	ASTList *childListIterator = t->children;
	while (childListIterator != NULL) {
		returnType = typeExpr(childListIterator->data, classContainingExprs, methodContainingExprs);
		childListIterator = childListIterator->next;
	}

	return returnType;
}

int printSemanticError(char *message, int lineNumber) {
	printf("Semantic error on line %d\n%s\n", lineNumber, message);
	exit(0);
}
