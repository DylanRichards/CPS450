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
}

int hasCycle(int cType) {
	ClassDecl *classDecl = &classesST[cType];

	while (classDecl->superclass != OBJECT_TYPE) {
		if (classDecl->superclass == cType) {
			return TRUE;
		} else {
			classDecl = &classesST[classDecl->superclass];
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
/* ====================================== */
/* ====================================== */
/* =========== TYPE CHECK PGM =========== */
/* ====================================== */
/* ====================================== */

void typecheckProgram() {
	/* === Level 3 === */
	//checkClasses();
	/* === Level 2 === */
	checkVarDeclList(mainBlockST, numMainBlockLocals);
	/* === Level 1 === */
	/* typecheck the main block expressions */
	typeExprs(mainExprs, -1, -1);
}

/* Returns the type of the expression AST in the given context.
 *  Also sets t->staticClassNum and t->staticMemberNum attributes as needed.
 *   If classContainingExpr < 0 then this expression is in the main block of
 *    the program; otherwise the expression is in the given class.
 *     */
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

        if (t1 != NAT_TYPE || t2 != NAT_TYPE) {
            printSemanticError("non-nat type in equality", t->lineNumber);
        }
        
        return NAT_TYPE;
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

		if(isSubtype(t1, t2) == TRUE){
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
        }
        printSemanticError("Undeclared var", t->lineNumber);
    }else if (t->typ == NEW_EXPR) {
        // level 3
	} else if (t->typ == THIS_EXPR) {
        // level 3
	} else if (t->typ == DOT_ASSIGN_EXPR) {
        // level 3
	} else if (t->typ == METHOD_CALL_EXPR) {
        // level 3
	} else if (t->typ == DOT_METHOD_CALL_EXPR) {
        // level 3
	} else if (t->typ == DOT_ID_EXPR) {
        // level 3
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
