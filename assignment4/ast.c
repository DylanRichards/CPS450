#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ast.h"

void printError(char *reason) {
  printf("AST Error: %s\n", reason);
  exit(-1);
}

ASTree *newAST(ASTNodeType t, ASTree *child, unsigned int natAttribute, 
  char *idAttribute, unsigned int lineNum) {
  ASTree *ast = malloc(sizeof(ASTree));
  if(ast==NULL) printError("malloc in newAST()");
  ast->typ = t;

  ASTList *childList = malloc(sizeof(ASTList));
  if(childList==NULL) printError("malloc in newAST()");
  childList->data = child;
  childList->next = NULL;
  ast->children = childList;
  ast->childrenTail = childList;

  if(idAttribute==NULL) ast->idVal=NULL;
  else {
    char *copyStr = malloc(strlen(idAttribute)+1);
    if(copyStr==NULL) printError("malloc in newAST()");
    strcpy(copyStr, idAttribute);
    ast->idVal = copyStr;
  }
  
  ast->natVal = natAttribute;
  ast->lineNumber = lineNum;

  return ast;
}


void appendToChildrenList(ASTree *parent, ASTree *newChild) {
	if(parent==NULL) printError("append called with null parent");
	if(parent->children==NULL || parent->childrenTail==NULL)
    	printError("append called with bad parent");
	if(newChild==NULL) printError("append called with null newChild");
	if(parent->childrenTail->data==NULL) //replace empty tail with new child
    	parent->childrenTail->data = newChild;
  	else { //tail of children is currently nonempty; append new child to list
	    ASTList *newList = malloc(sizeof(ASTList));
	    if(newList==NULL) printError("malloc in appendAST()");
	    newList->data = newChild;
	    newList->next = NULL;
	    parent->childrenTail->next = newList;
	    parent->childrenTail = newList;
  }
}

/* Print the type of this node and any node attributes */
void printNodeTypeAndAttribute(ASTree *t) {
  if(t==NULL) return;
  switch(t->typ) {
    case PROGRAM: printf("PROGRAM"); break;
    case CLASS_DECL_LIST: printf("CLASS_DECL_LIST"); break;
    case CLASS_DECL: printf("CLASS_DECL"); break;
    case VAR_DECL_LIST: printf("VAR_DECL_LIST"); break;
    case VAR_DECL: printf("VAR_DECL"); break;
    case METHOD_DECL_LIST: printf("METHOD_DECL_LIST"); break;
    case PARAM_DECL: printf("PARAM_DECL"); break;
    case PARAM_DECL_LIST: printf("PARAM_DECL_LIST"); break;
    case METHOD_DECL: printf("METHOD_DECL"); break;
    case EXPR_LIST: printf("EXPR_LIST"); break;
    case DOT_METHOD_CALL_EXPR: printf("DOT_METHOD_CALL_EXPR"); break;
    case METHOD_CALL_EXPR: printf("METHOD_CALL_EXPR"); break;
    case DOT_ASSIGN_EXPR: printf("DOT_ASSIGN_EXPR"); break;
    case ASSIGN_EXPR: printf("ASSIGN_EXPR"); break;
    case PLUS_EXPR: printf("PLUS_EXPR"); break;
    case MINUS_EXPR: printf("MINUS_EXPR"); break;
    case TIMES_EXPR: printf("TIMES_EXPR"); break;
    case EQUALITY_EXPR: printf("EQUALITY_EXPR"); break;
    case LESS_THAN_EXPR: printf("LESS_THAN_EXPR"); break;
    case NOT_EXPR: printf("NOT_EXPR"); break;
    case AND_EXPR: printf("AND_EXPR"); break;
    case IF_THEN_ELSE_EXPR: printf("IF_THEN_ELSE_EXPR"); break;
    case WHILE_EXPR: printf("WHILE_EXPR"); break;
    case PRINT_EXPR: printf("PRINT_EXPR"); break;
    case READ_EXPR: printf("READ_EXPR"); break;
    case THIS_EXPR: printf("THIS_EXPR"); break;
    case NEW_EXPR: printf("NEW_EXPR"); break;
    case NULL_EXPR: printf("NULL_EXPR"); break;
    case ARG_LIST: printf("ARG_LIST"); break;

    case NAT_LITERAL_EXPR: printf("NAT_LITERAL_EXPR(%d)",t->natVal); break;
    case NAT_TYPE: printf("NAT_TYPE"); break;
    case AST_ID: printf("AST_ID(%s)",t->idVal); break;
    case DOT_ID_EXPR: printf("DOT_ID_EXPR"); break;
    case ID_EXPR: printf("ID_EXPR"); break;
    default: printError("unknown node type in printNodeTypeAndAttribute()");
  }  

  printf(" (ends on line %d)", t->lineNumber);
}

/* print tree in preorder */
void printASTree(ASTree *t, int depth) {
  if(t==NULL) return;

  printf("%d:",depth);
  int i=0;
    
  for(; i<depth; i++)
      printf("  ");
    
  printNodeTypeAndAttribute(t);
  printf("\n");
    
  /*recursively print all children*/
  ASTList *childListIterator = t->children;
    
  while(childListIterator!=NULL) {
    printASTree(childListIterator->data, depth+1);
    childListIterator = childListIterator->next;
  }
    
}

/* Print the AST to stdout with indentations marking tree depth. */
void printAST(ASTree *t) {
    printf("========= AST =========\n");
    printASTree(t, 0);
    printf("=======================\n");
}
