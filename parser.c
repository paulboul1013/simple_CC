#include <ctype.h>
#include <limits.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cc.h"

#define MAX_ARGS 6
#define MAX_OP_PRIO 16
#define MAX_ALIGN 16

List *ctypes=&EMPTY_LIST;
List *strings=&EMPTY_LIST;
List *flonums=&EMPTY_LIST;

static Dict *globalenv=&EMPTY_DICT;
static Dict *localenv=NULL;
static Dict *struct_defs=&EMPTY_DICT;
static Dict *union_defs=&EMPTY_DICT;
static List *localvars=NULL;

static Ctype *ctype_void=&(Ctype){CTYPE_VOID,0,NULL};
static Ctype *ctype_int=&(Ctype){CTYPE_INT,4,NULL};
static Ctype *ctype_long=&(Ctype){CTYPE_LONG,8,NULL};
static Ctype *ctype_char=&(Ctype){CTYPE_CHAR,1,NULL};
static Ctype *ctype_float=&(Ctype){CTYPE_FLOAT,4,NULL};

static int labelseq=0;

static Ast *read_expr(void);
static Ctype *make_ptr_type(Ctype *ctype);
static Ctype *make_array_type(Ctype *ctype, int size);
static Ast *read_compound_stmt(void);
static Ast *read_decl_or_stmt(void);
static Ctype *result_type(char op, Ctype *a, Ctype *b);
static Ctype *convert_array(Ctype *ctype);
static Ast *read_stmt(void);
static Ctype *read_decl_int(Token *name);

