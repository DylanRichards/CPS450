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
    /* Check var names are unique */
    int i, j;
    for(i=0; i<numMainBlockLocals-1; i++){
        VarDecl *varDecl = &mainBlockST[i];
        
        for(j=i+1; j<numMainBlockLocals; j++){
            VarDecl *varDecl2 = &mainBlockST[j];
            
            if(strcmp(varDecl->varName, varDecl2->varName)==0){
                printSemanticError("Var declared multiple times", varDecl->varNameLineNumber);
            }
        }
    }
    
    /* check var type is >= -1 */
    for(i=0; i<numMainBlockLocals; i++){
        VarDecl *varDecl = &mainBlockST[i];
        if(varDecl->type < NAT_TYPE){
            printSemanticError("Undefined variable", varDecl->typeLineNumber);
        }
    }
}

int join(int t1, int t2) {
	if (isSubtype(t1, t2) == TRUE) {
		return t2;
	} else if (isSubtype(t2, t1) == TRUE) {
		return t1;
	} else {
        return UNDEFINED_TYPE;
	}
}

/* ====================================== */
/* ====================================== */
/* =========== TYPE CHECK PGM =========== */
/* ====================================== */
/* ====================================== */

void typecheckProgram() {
	/* === Level 2 === */
	checkVarDeclList(mainBlockST, numMainBlockLocals);
	/* === Level 1 === */
	/* typecheck the main block expressions */
	typeExprs(mainExprs);
}

/* Returns the type of the expression AST in the given context.
 *  Also sets t->staticClassNum and t->staticMemberNum attributes as needed.
 *   If classContainingExpr < 0 then this expression is in the main block of
 *    the program; otherwise the expression is in the given class.
 *     */
int typeExpr(ASTree *t) {

	if (t->typ == EXPR_LIST) {
        // if the current note is an expre_list, then recursively call typeExprs function to check its children
		return typeExprs(t);
	} else if (t->typ == NAT_LITERAL_EXPR) {
        // level 1
		return NAT_TYPE;
	} else if (t->typ == NULL_EXPR) {
        // level 1
		return NULL_TYPE;
	} else if (t->typ == PRINT_EXPR) {
        // level 1
        int t1 = typeExpr(t->children->data);
        
        if (t1 != NAT_TYPE) {
            printSemanticError("non-nat type in printNat", t->lineNumber);
        }
        
        return NAT_TYPE;
        
	} else if (t->typ == READ_EXPR) {
        // level 1
        return NAT_TYPE;
	} else if (t->typ == PLUS_EXPR) {
        // level 1
        // typeExpr returns the type of the examing node
        int t1 = typeExpr(t->children->data);
        int t2 = typeExpr(t->children->next->data);

        // for plus operation, the two operands must be nat type, otherwise, print an error message
        if (t1 != NAT_TYPE || t2 != NAT_TYPE) {
            printSemanticError("non-nat type in plus", t->lineNumber);
        }
        
        return NAT_TYPE;
    } else if (t->typ == MINUS_EXPR) {
        // level 1
        // typeExpr returns the type of the examing node
        int t1 = typeExpr(t->children->data);
        int t2 = typeExpr(t->children->next->data);

        // for minus operation, the two operands must be nat type, otherwise, print an error message
        if (t1 != NAT_TYPE || t2 != NAT_TYPE) {
            printSemanticError("non-nat type in minus", t->lineNumber);
        }
        
        return NAT_TYPE;
	} else if (t->typ == TIMES_EXPR) {
        // level 1
        // typeExpr returns the type of the examing node
        int t1 = typeExpr(t->children->data);
        int t2 = typeExpr(t->children->next->data);

        // for times operation, the two operands must be nat type, otherwise, print an error message
        if (t1 != NAT_TYPE || t2 != NAT_TYPE) {
            printSemanticError("non-nat type in times", t->lineNumber);
        }
        
        return NAT_TYPE;
	} else if (t->typ == LESS_THAN_EXPR) {
        // level 1
        // typeExpr returns the type of the examing node
        int t1 = typeExpr(t->children->data);
        int t2 = typeExpr(t->children->next->data);

        // for LESS_THAN_EXPR operation, the two operands must be nat type, otherwise, print an error message
        if (t1 != NAT_TYPE || t2 != NAT_TYPE) {
            printSemanticError("non-nat type in less than", t->lineNumber);
        }
        
        return NAT_TYPE;
	} else if (t->typ == AND_EXPR) {
        // level 1
        // typeExpr returns the type of the examing node
        int t1 = typeExpr(t->children->data);
        int t2 = typeExpr(t->children->next->data);

        // for AND_EXPR operation, the two operands must be nat type, otherwise, print an error message
        if (t1 != NAT_TYPE || t2 != NAT_TYPE) {
            printSemanticError("non-nat type in and", t->lineNumber);
        }
        
        return NAT_TYPE;
	} else if (t->typ == NOT_EXPR) {
        // level 1
        int t1 = typeExpr(t->children->data);
        
        if (t1 != NAT_TYPE ) {
            printSemanticError("non-nat type in not", t->lineNumber);
        }
        return NAT_TYPE;
	} else if (t->typ == EQUALITY_EXPR) {
        // level 1
        int t1 = typeExpr(t->children->data);
        int t2 = typeExpr(t->children->next->data);

        if (isSubtype(t1, t2) || isSubtype(t2, t1)) {
			return NAT_TYPE;
		}
        
        printSemanticError("type mismatch in equal", t->lineNumber);
        
	} else if (t->typ == IF_THEN_ELSE_EXPR) {
        // level 1
        int t1 = typeExpr(t->children->data);
        int t2 = typeExpr(t->children->next->data);
        int t3 = typeExpr(t->children->next->next->data);
        
        if (t1 == NAT_TYPE && join(t2,t3) > UNDEFINED_TYPE){
            return join(t2,t3);
        }

        printSemanticError("non-nat type in if-else", t->lineNumber);
        
	} else if (t->typ == WHILE_EXPR) {
        // level 1
        
        int t1 = typeExpr(t->children->data);
        
        typeExpr(t->children->next->data);
        
        if (t1 == NAT_TYPE)
        {
            return NAT_TYPE;
        }
        
        printSemanticError("non-nat type in while", t->lineNumber);
        
	} else if (t->typ == ASSIGN_EXPR) {
        // level 2
        if(t->children->data->typ != AST_ID){
            printSemanticError("Non-id type in assignment", t->lineNumber);
        }
        
        int t1 = typeExpr(t->children->data);
        int t2 = typeExpr(t->children->next->data);
        if(isSubtype(t2,t1) == TRUE){
            return t1;
        }
        printSemanticError("Type mismatch in assignment", t->lineNumber);
        
    } else if (t->typ == ID_EXPR) {
        // level 2
        return typeExpr(t->children->data);
    } else if (t->typ == AST_ID) {
        // level 2
        int i;
        // Examing variablke is in main function
       
        for (i = 0; i < numMainBlockLocals; i++) {
            VarDecl *varDecl = &mainBlockST[i];
            if (strcmp(varDecl->varName, t->idVal) == 0) {
                return varDecl->type;
            }
        }
        printSemanticError("Undeclared var", t->lineNumber);
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
	}
	return FALSE;
}

/* Returns the type of the EXPR_LIST AST in the given context. */
int typeExprs(ASTree *t) {
	int returnType;

	ASTList *childListIterator = t->children;
	while (childListIterator != NULL) {
		returnType = typeExpr(childListIterator->data);
		childListIterator = childListIterator->next;
	}

	return returnType;
}

int printSemanticError(char *message, int lineNumber) {
	printf("Semantic error on line %d\n%s\n", lineNumber, message);
	exit(0);
}

