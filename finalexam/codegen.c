#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "codegen.h"
#include "symtbl.h"

#define MAX_DISM_ADDR 65535

/**
 *  * DISM instructions
 *   */
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
void codeGenExpr(ASTree *t);
void codeGenExprs(ASTree *expList);


/**
 *  * Appends code to fout with params
 *   */
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


void codeGenExpr(ASTree *t) {
	if (t->typ == EXPR_LIST) {
		codeGenExprs(t);
	} else if (t->typ == NAT_LITERAL_EXPR) {
        appendCode("R[1] <- natValue", "%s 1 %d", MOVE, t->natVal);
		appendCode("M[SP] <- R[r1] (a nat literal)", "%s 6 0 1", STORE);
		decSP();
	} else if (t->typ == NULL_EXPR) {
        appendCode("M[SP] <- 0 (null)", "%s 6 0 0", STORE);
        decSP();
	} else if (t->typ == PRINT_EXPR) {
        codeGenExpr(t->children->data);
        appendCode("Load value", "%s 1 6 1", LOAD); // R1 = M[SP+1]
        appendCode("print value", "%s 1", PTN); // print R1
	} else if (t->typ == READ_EXPR) {
        appendCode("R[1] <- readNat()", "%s 1", RDN);
        appendCode("M[SP] <- R[1]", "%s 6 0 1", STORE);
        decSP();
	} else if (t->typ == PLUS_EXPR) {
		codeGenExpr(t->children->data);
		codeGenExpr(t->children->next->data);
		appendCode("R1 <- M[R6+2]", "%s 1 6 2", LOAD);
		appendCode("R2 <- M[R6+1]", "%s 2 6 1", LOAD);
		appendCode("R1 <- R1 + R2", "%s 1 1 2", ADD);
		appendCode("M[SP + 2] <- R[1] (for plus)", "%s 6 2 1", STORE);
		incSP();
	} else if (t->typ == MINUS_EXPR) {
        codeGenExpr(t->children->data);
		codeGenExpr(t->children->next->data);
		appendCode("R1 <- M[R6+2]", "%s 1 6 2", LOAD);
		appendCode("R2 <- M[R6+1]", "%s 2 6 1", LOAD);
		appendCode("R1 <- R1 - R2", "%s 1 1 2", SUB);
		appendCode("M[SP + 2] <- R[1] (for minus)", "%s 6 2 1", STORE);
		incSP();
	} else if (t->typ == TIMES_EXPR) {
        codeGenExpr(t->children->data);
		codeGenExpr(t->children->next->data);
		appendCode("R1 <- M[R6+2]", "%s 1 6 2", LOAD);
		appendCode("R2 <- M[R6+1]", "%s 2 6 1", LOAD);
		appendCode("R1 <- R1 * R2", "%s 1 1 2", MUL);
		appendCode("M[SP + 2] <- R[1] (for times)", "%s 6 2 1", STORE);
		incSP();
	} else if (t->typ == LESS_THAN_EXPR) {
        codeGenExpr(t->children->data);
		codeGenExpr(t->children->next->data);
        appendCode("R[1] <- M[R6+2]", "%s 1 6 2", LOAD);
        appendCode("R[2] <- M[R6+1]", "%s 2 6 1", LOAD);
        appendCode("brancj R2 < R1", "%s 1 2 #%d", BLT, labelNumber);
        appendCode("R[3] = 0", "%s 3 0", MOVE);
        appendCode("jump exit of the branch", "%s 0 #%d", JUMP, labelNumber+1);
        appendCode("R[3]=1", "#%d: %s 3 1", labelNumber, MOVE);
        appendCode("M[SP+2] = R[3]", "#%d: %s 6 2 3", labelNumber+1, STORE);
        labelNumber = labelNumber + 2;
        incSP();
	} else if (t->typ == AND_EXPR) {
        codeGenExpr(t->children->data);
        int localLableNumber = labelNumber;
        appendCode("R1 = M[SP+1]", "%s 1 6 1", LOAD);
        appendCode("branch R0<R1", "%s 0 1 #%d", BLT, localLableNumber);
        appendCode("R[2] = 0", "%s 2 0", MOVE);
        appendCode("M[SP+1] = R[2]", "%s 6 1 2", STORE);
        appendCode("jump exit of the branch", "%s 0 #%d", JUMP, localLableNumber+1);
        appendCode("Else label", "#%d: %s 0 0", localLableNumber, MOVE);
        codeGenExpr(t->children->next->data);
        appendCode("R1 = M[SP+1]", "%s 1 6 1", LOAD);
        appendCode("branch R1 == R0", "%s 1 0 #%d", BEQ, localLableNumber+2);
        appendCode("R[2] = 1", "%s 2 1", MOVE);
        appendCode("M[SP+2] = R[2]", "%s 6 2 2", STORE);
        incSP();
        appendCode("jump exit of the branch", "%s 0 #%d", JUMP, localLableNumber+1);
        appendCode("M[SP+2]= 0", "#%d: %s 6 2 0", localLableNumber+2, STORE);
        incSP();
        appendCode("Exit", "#%d: %s 0 0", localLableNumber+1, MOVE);
	} else if (t->typ == NOT_EXPR) {
        // Level 1
        codeGenExpr(t->children->data);
        appendCode("R1 = M[SP+1]", "%s  1 6 1", LOAD);
        appendCode("branch if R1 ==R0", "%s 1 0 #%d", BEQ, labelNumber);
        appendCode("M[SP+1] = 0", "%s 6 1 0", STORE);
        appendCode("jump to the end", "%s 0 #%d", JUMP, labelNumber+1);
        appendCode("else branch, R2 = 1", "#%d: %s 2 1", labelNumber, MOVE);
        appendCode("M[SP+1] = R2 = 1", "%s 6 1 2", STORE);
        appendCode("exit branch", "#%d: %s 0 0", labelNumber+1, MOVE);
        labelNumber=labelNumber+2;
	} else if (t->typ == EQUALITY_EXPR) {
        // Not implemented
	} else if (t->typ == IF_THEN_ELSE_EXPR) {
        // Level 1
        codeGenExpr(t->children->data);
        int localLabelNumber = labelNumber;
		labelNumber = labelNumber + 2;
        appendCode("R1 <- M[R6+1]", "%s 1 6 1", LOAD);
        appendCode("branch R1 == R0", "%s 1 0 #%d", BEQ, localLabelNumber);
        codeGenExpr(t->children->next->data);
        appendCode("R2 <- M[R6+1]", "%s 2 6 1", LOAD);
        appendCode("M[SP + 2] <- R[2]", "%s 6 2 2", STORE);
        appendCode("jump exit of the branch", "%s 0 #%d", JUMP, localLabelNumber + 1);
        appendCode("Else branch begin here", "#%d: %s 0 0", localLabelNumber, MOVE);
        codeGenExpr(t->children->next->next->data);
        appendCode("R2 <- M[R6+1]", "%s 2 6 1", LOAD);
        appendCode("M[SP + 2] <- R[2]", "%s 6 2 2", STORE);
        appendCode("exit branch", "#%d: %s 0 0", localLabelNumber + 1, MOVE);
        
        incSP();
	} else if (t->typ == WHILE_EXPR) {
        // Not implemented
	} else if (t->typ == AST_ID) {
		// Level 2 & 3
        int i;
        // level 2
        for(i=0; i<numMainBlockLocals;i++){
            VarDecl *VarDecl = &mainBlockST[i];
            if(strcmp(VarDecl->varName, t->idVal)==0){
                appendCode("R1 = numMainBlockLocals", "%s 1 %d", MOVE, numMainBlockLocals);
                appendCode("R2 = i", "%s 2 %d", MOVE, i);
                appendCode("R1=R1-R2", "%s 1 1 2", SUB);
                appendCode("R1(Address)  = FP+R1", "%s 1 1 7", ADD);
                appendCode("R1 = M[R1]", "%s 1 1 0", LOAD);
                appendCode("M[SP] = R1", "%s 6 0 1", STORE);
                decSP();
            }
        }
	} else if (t->typ == ID_EXPR) {
		// Level 2
        codeGenExpr(t->children->data);
	} else if (t->typ == ASSIGN_EXPR) {
		// Level 2 & 3
        codeGenExpr(t->children->next->data);
        appendCode("R3 = Right operand", "%s 3 6 1", LOAD);
        int i;
        for(i=0; i<numMainBlockLocals;i++){
            VarDecl *VarDecl = &mainBlockST[i];
            if(strcmp(VarDecl->varName, t->children->data->idVal)==0){
                appendCode("R1 = numMainBlockLocals", "%s 1 %d", MOVE, numMainBlockLocals);
                appendCode("R2 = i", "%s 2 %d", MOVE, i);
                appendCode("R1=R1-R2", "%s 1 1 2", SUB);
                appendCode("R1(Address)  = FP+R1", "%s 1 1 7", ADD);
                break;
            }
        }
        appendCode("M[R1] = R3", "%s 1 0 3", STORE);
	}
}

void codeGenExprs(ASTree *expList) {
	ASTList *childListIterator = expList->children;
	while (childListIterator != NULL) {
		codeGenExpr(childListIterator->data);
		childListIterator = childListIterator->next;
		if (childListIterator != NULL) {
			incSP();
		}
	}
}

/* Generate DISM code as the prologue to the given method or main
 *  block. If classNumber < 0 then methodNumber may be anything and we
 *   assume we are generating code for the program's main block. */
void genPrologue() {
	initPointers();
    if (numMainBlockLocals > 0) {
        appendCode("Allocate frame space for main locals", "%s 1 %d", MOVE, numMainBlockLocals);
        appendCode("FP <- FP - R1", "%s 7 7 1", SUB);
        appendCode("R1 <- 1", "%s 1 1", MOVE);
        appendCode("SP <- FP - 1", "%s 6 7 0", SUB);
    }
}

/* Generate DISM code as the epilogue to the given method or main
 *  block. If classNumber < 0 then methodNumber may be anything and we
 *   assume we are generating code for the program's main block. */
void genEpilogue() {
    /* Terminate the program */
    appendCode("Normal termination at the end of the main block", "%s 0", HALT);
}

void generateDISM(FILE *outputFile) {
	fout = outputFile;
	genPrologue();
	codeGenExprs(mainExprs);
    genEpilogue();
	fclose(fout);
}

