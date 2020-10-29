#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "codegen.h"
#include "symtbl.h"

#define MAX_DISM_ADDR 65535

/* DISM instructions */
#define MOVE "mov"
#define ADD "add"
#define SUB "sub"
#define MUL "mul"
#define LOAD "lod"
#define STORE "str"
#define JUMP "jmp"
#define BEQ "beq"
#define BLT "blt"
#define RDN "rdn"
#define PTN "ptn"
#define HALT "hlt"

/* Global for the DISM output file */
FILE *fout;
/* Global to remember the next unique label number to use */
unsigned int labelNumber = 0;
/* Declare mutually recursive functions (defs and docs appear below) */
void codeGenExpr(ASTree *t, int classNumber, int methodNumber);
void codeGenExprs(ASTree *expList, int classNumber, int methodNumber);

/**
 * Appends code to fout with params
 */
void appendCode(char *comment, char *code, ...) {
	va_list arglist;
	char buffer[128];
	va_start(arglist, code);
	vsprintf(buffer, code, arglist);
	va_end(arglist);

	fprintf(fout, "%s ; %s\n", buffer, comment);
}

void initPointers() {
	appendCode("init FP", "%s 7 %d", MOVE, MAX_DISM_ADDR);
	appendCode("init SP", "%s 6 %d", MOVE, MAX_DISM_ADDR);
	appendCode("init HP", "%s 5 1", MOVE);
}


/* Generate code that increments the stack pointer */
void incSP() {
	appendCode("start incSP()", "%s 1 1", MOVE);
	appendCode("SP++", "%s 6 6 1", ADD);
}

/* Generate code that decrements the stack pointer */
void decSP() {
	appendCode("Start decSP()", "%s 1 1", MOVE);
	appendCode("SP--", "%s 6 6 1", SUB);
}


void codeGenExpr(ASTree *t, int classNumber, int methodNumber) {
	if (t->typ == EXPR_LIST) {
        // if the current note is an expre_list, then recursively call codeGenExpr function to check its children
		codeGenExprs(t, classNumber, methodNumber);
	} else if (t->typ == NAT_LITERAL_EXPR) {
        // Level 1
        appendCode("R1 <- natValue", "%s 1 %d", MOVE, t->natVal);
		appendCode("M[SP] <- R1 (a nat literal)", "%s 6 0 1", STORE);
		decSP();
	} else if (t->typ == NULL_EXPR) {
        // Level 1
		appendCode("M[SP] <- 0 (null)", "%s 6 0 0", STORE);
		decSP();
	} else if (t->typ == PRINT_EXPR) {
        // Level 1
		codeGenExpr(t->children->data, classNumber, methodNumber);
		appendCode("Load value", "%s 1 6 1", LOAD);
		appendCode("Print value", "%s 1", PTN);
	} else if (t->typ == READ_EXPR) {
        // Level 1
		appendCode("R1 <- readNat()", "%s 1", RDN);
		appendCode("M[SP] <- R1", "%s 6 0 1", STORE);
		decSP();
	} else if (t->typ == PLUS_EXPR) {
        // Level 1
		codeGenExpr(t->children->data, classNumber, methodNumber);
		codeGenExpr(t->children->next->data, classNumber, methodNumber);
		appendCode("R1 <- M[R6+2]", "%s 1 6 2", LOAD);
		appendCode("R2 <- M[R6+1]", "%s 2 6 1", LOAD);
		appendCode("R1 <- R1 + R2", "%s 1 1 2", ADD);
		appendCode("M[SP+2] <- R1 (for plus)", "%s 6 2 1", STORE);
		incSP();
	} else if (t->typ == MINUS_EXPR) {
        // Level 1
		codeGenExpr(t->children->data, classNumber, methodNumber);
		codeGenExpr(t->children->next->data, classNumber, methodNumber);
		appendCode("R1 <- M[R6+2]", "%s 1 6 2", LOAD);
		appendCode("R2 <- M[R6+1]", "%s 2 6 1", LOAD);
		appendCode("R1 <- R1 - R2", "%s 1 1 2", SUB);
		appendCode("M[SP+2] <- R1 (for subtract)", "%s 6 2 1", STORE);
		incSP();
	} else if (t->typ == TIMES_EXPR) {
        // Level 1
		codeGenExpr(t->children->data, classNumber, methodNumber);
		codeGenExpr(t->children->next->data, classNumber, methodNumber);
		appendCode("R1 <- M[R6+2]", "%s 1 6 2", LOAD);
		appendCode("R2 <- M[R6+1]", "%s 2 6 1", LOAD);
		appendCode("R1 <- R1 * R2", "%s 1 1 2", MUL);
		appendCode("M[SP+2] <- R[1] (for multiply)", "%s 6 2 1", STORE);
		incSP();
	} else if (t->typ == LESS_THAN_EXPR) {
        // Level 1
		codeGenExpr(t->children->data, classNumber, methodNumber);
		codeGenExpr(t->children->next->data, classNumber, methodNumber);
		appendCode("R1 <- M[R6+2]", "%s 1 6 2", LOAD);
		appendCode("R2 <- M[R6+1]", "%s 2 6 1", LOAD);
		appendCode("branch R1 < R2", "%s 1 2 #%d", BLT, labelNumber);
		appendCode("R3 = 0", "%s 3 0", MOVE);
		appendCode("jump exit branch", "%s 0 #%d", JUMP, labelNumber+1);
		appendCode("R3 = 1", "#%d: %s 3 1", labelNumber, MOVE);
		appendCode("M[SP+2] <- R3", "#%d: %s 6 2 3", labelNumber+1, STORE);
		labelNumber += 2;
		incSP();
	} else if (t->typ == AND_EXPR) {
        // Level 1
		int localLabelNumber = labelNumber;
		codeGenExpr(t->children->data, classNumber, methodNumber);
		appendCode("R1 <- M[SP+1]", "%s 1 6 1", LOAD);
		appendCode("branch R0<R1", "%s 0 1 #%d", BLT, localLabelNumber);
		appendCode("R2 = 0", "%s 2 0", MOVE);
		appendCode("M[SP+1] <- R[2]", "%s 6 1 2", STORE);
		appendCode("jump exit branch", "%s 0 #%d", JUMP, localLabelNumber + 1);
		appendCode("Else label", "#%d: %s 0 0", localLabelNumber, MOVE);
		codeGenExpr(t->children->next->data, classNumber, methodNumber);
		appendCode("R1 <- M[SP+1]", "%s 1 6 1", LOAD);
		appendCode("branch R1 == R0", "%s 1 0 #%d", BEQ, localLabelNumber + 2);
		appendCode("R2 = 1", "%s 2 1", MOVE);
		appendCode("M[SP+2] <- R2", "%s 6 2 2", STORE);
		incSP();
		appendCode("jump exit branch", "%s 0 #%d", JUMP, localLabelNumber + 1);
		appendCode("M[SP+2] <- 0", "#%d: %s 6 2 0", localLabelNumber + 2, STORE);
		incSP();
		appendCode("Exit", "#%d: %s 0 0", localLabelNumber + 1, MOVE);
	} else if (t->typ == NOT_EXPR) {
        // Level 1
		codeGenExpr(t->children->data, classNumber, methodNumber);
		appendCode("R1 <- M[SP+1]", "%s 1 6 1", LOAD);
		appendCode("Branch R1 == 0", "%s 1 0 #%d", BEQ, labelNumber);
		appendCode("R1 = 0", "%s 1 0", MOVE);
		appendCode("jump exit branch", "%s 0 #%d", JUMP, labelNumber + 1);
		appendCode("R1 = 1", "#%d: %s 1 1", labelNumber, MOVE);
		appendCode("M[SP+1] <- R1", "#%d: %s 6 1 1", labelNumber + 1, STORE);
		labelNumber += 2;
	} else if (t->typ == EQUALITY_EXPR) {
        // Level 1
		codeGenExpr(t->children->data, classNumber, methodNumber);
		codeGenExpr(t->children->next->data, classNumber, methodNumber);
		appendCode("R1 <- M[R6+2]", "%s 1 6 2", LOAD);
		appendCode("R2 <- M[R6+1]", "%s 2 6 1", LOAD);
		appendCode("branch R1 == R2", "%s 1 2 #%d", BEQ, labelNumber);
		appendCode("R3 = 0", "%s 3 0", MOVE);
		appendCode("jump exit branch", "%s 0 #%d", JUMP, labelNumber+1);
		appendCode("R3 = 1", "#%d: %s 3 1", labelNumber, MOVE);
		appendCode("M[SP+2] <- R3", "#%d: %s 6 2 3", labelNumber+1, STORE);
		labelNumber += 2;
		incSP();
	} else if (t->typ == IF_THEN_ELSE_EXPR) {
        // Level 1
	} else if (t->typ == WHILE_EXPR) {
		// Level 1
	} else if (t->typ == AST_ID) {
		// Level 2 & 3
	} else if (t->typ == ID_EXPR) {
		// Level 2
	} else if (t->typ == NEW_EXPR) {
		// Level 3
	} else if (t->typ == THIS_EXPR) {
		// Level 3
	} else if (t->typ == ASSIGN_EXPR) {
		// Level 2 & 3
	} else if (t->typ == DOT_ID_EXPR) {
		// Level 3
	} else if (t->typ == DOT_ASSIGN_EXPR) {
		// Level 3
	} else if (t->typ == METHOD_CALL_EXPR) {
		// Level 3
    } else if (t->typ == DOT_METHOD_CALL_EXPR) {
		// Level 3
	} else if (t->typ == ARG_LIST) {
		// Level 3
	} 
}

void codeGenExprs(ASTree *expList, int classNumber, int methodNumber) {
	ASTList *childListIterator = expList->children;
	while (childListIterator != NULL) {
		codeGenExpr(childListIterator->data, classNumber, methodNumber);
		childListIterator = childListIterator->next;
		if (childListIterator != NULL) {
			incSP();
		}
	}
}

/* Generate DISM code as the prologue to the given method or main
 * block. If classNumber < 0 then methodNumber may be anything and we
 * assume we are generating code for the program's main block.
 */
void genPrologue(int classNumber, int methodNumber) {
	initPointers();
	if (classNumber < 0) {
		if (numMainBlockLocals > 0) {
			appendCode("Allocate frame space for main locals", "%s 1 %d", MOVE, numMainBlockLocals);
			appendCode("FP <- FP - R1", "%s 7 7 1", SUB);
			appendCode("R1 <- 1", "%s 1 1", MOVE);
			appendCode("SP <- FP - 1", "%s 6 7 1", SUB);
		}
	}
}

/* Generate DISM code as the epilogue to the given method or main
 * block. If classNumber < 0 then methodNumber may be anything and we
 * assume we are generating code for the program's main block.
 */
void genEpilogue(int classNumber, int methodNumber) {
	if (classNumber < 0) {
		/* Terminate the program */
		appendCode("Normal termination at the end of the main block", "%s 0", HALT);
	}
}

void generateDISM(FILE *outputFile) {
	fout = outputFile;
    
	genPrologue(-1, -1);

	codeGenExprs(mainExprs, -1, -1);
 
    genEpilogue(-1, -1);

	fclose(fout);
}
