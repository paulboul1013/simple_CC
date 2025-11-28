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

static Ast *ast_uop(int type,Ctype *ctype,Ast *operand){
    Ast *r=malloc(sizeof(Ast));
    r->type=type;
    r->ctype=ctype;
    r->operand=operand;
    return r;
}


static Ast *ast_binop(int type,Ast *left,Ast *right){
    Ast *r=malloc(sizeof(Ast));
    r->type=type;
    r->ctype=result_type(type,left->ctype,right->ctype);
    if (type!='=' && convert_array(left->ctype)->type!=CTYPE_PTR && convert_array(right->ctype)->type==CTYPE_PTR){
        r->left=right;
        r->right=left;
    } else {
        r->left=left;
        r->right=right;
    }
    return r;
}


static Ast *ast_inttype(Ctype *ctype,long val){
    Ast *r=malloc(sizeof(Ast));
    r->type=AST_LITERAL;
    r->ctype=ctype;
    r->ival=val;
    return r;
}

static Ast *ast_double(double val){
    Ast *r=malloc(sizeof(Ast));
    r->type=AST_LITERAL;
    r->ctype=ctype_double;
    r->fval=val;
    list_push(flonums,r);
    return r;
}


char *make_label(void){
    String s=make_string();
    string_appendf(&s, ".L%d", labelseq++);
    return get_cstring(s);


}

static Ast *ast_lvar(Ctype *ctype,char *name){
    Ast *r=malloc(sizeof(Ast));
    r->type=AST_LVAR;
    r->ctype=ctype;
    r->varname=name;
    dict_put(localenv,name,r);
    if (localvars) list_push(localvars,r);
    return r;
}

static Ast *ast_gvar(Ctype *ctype,char *name,bool filelocal){
    Ast *r=malloc(sizeof(Ast));
    r->type=AST_GVAR;
    r->ctype=ctype;
    r->varname=name;
    r->glabel=filelocal ? make_label() : name;
    dict_put(globalenv,name,r);
    return r;
}

static Ast *ast_string(char *str){
    Ast *r=malloc(sizeof(Ast));
    r->type=AST_STRING;
    r->ctype=make_array_type(ctype_char,strlen(str)+1);
    r->sval=str;
    r->slabel=make_label();
    return r;
}

static Ast *ast_funcall(Ctype *ctype,char *fname,List *args){
    Ast *r=malloc(sizeof(Ast));
    r->type=AST_FUNCALL;
    r->ctype=ctype;
    r->fname=fname;
    r->args=args;
    return r;
}


static Ast *ast_func(Ctype *rettype,
                        char *fname,
                        List *params,
                        Ast *body,
                        List *localvars)
{
    Ast *r=malloc(sizeof(Ast));
    r->type=AST_FUNC;
    r->ctype=rettype;
    r->fname=fname;
    r->params=params;
    r->localvars=localvars;
    r->body=body;
    return r;
}

static Ast *ast_decl(Ast *var,Ast *init){
    Ast *r=malloc(sizeof(Ast));
    r->type=AST_DECL;
    r->ctype=NULL;
    r->declvar=var;
    r->declinit=init;
    return r;
}

static Ast *ast_array_init(List *arrayinit)
{
    Ast *r = malloc(sizeof(Ast));
    r->type = AST_ARRAY_INIT;
    r->ctype = NULL;
    r->arrayinit = arrayinit;
    return r;
}

static Ast *ast_if(Ast *cond,Ast *then,Ast *els){
    Ast *r=malloc(sizeof(Ast));
    r->type=AST_IF;
    r->ctype=NULL;
    r->cond=cond;
    r->then=then;
    r->els=els;
    return r;
}

static Ast *ast_ternary(Ctype *ctype,Ast *cond,Ast *then,Ast *els){
    Ast *r=malloc(sizeof(Ast));
    r->type=AST_TERNARY;
    r->ctype=ctype;
    r->cond=cond;
    r->then=then;
    r->els=els;
    return r;
}

static Ast *ast_for(Ast *init,Ast *cond,Ast *step,Ast *body){
    Ast *r=malloc(sizeof(Ast));
    r->type=AST_FOR;
    r->ctype=NULL;
    r->forinit=init;
    r->forcond=cond;
    r->forstep=step;
    r->forbody=body;
    return r;
}

static Ast *ast_return(Ast *retval){
    Ast *r=malloc(sizeof(Ast));
    r->type=AST_RETURN;
    r->ctype=NULL;
    r->retval=retval;
    return r;
}

static Ast *ast_compound_stmt(List *stmts){
    Ast *r=malloc(sizeof(Ast));
    r->type=AST_COMPOUND_STMT;
    r->ctype=NULL;
    r->stmts=stmts;
    return r;
}

static Ast *ast_struct_ref(Ctype *ctype, Ast *struc, char *name)
{
    Ast *r = malloc(sizeof(Ast));
    r->type = AST_STRUCT_REF;
    r->ctype = ctype;
    r->struc = struc;
    r->field = name;
    return r;
}

static Ctype *make_ptr_type(Ctype *ctype){
    Ctype *r=malloc(sizeof(Ctype));
    r->type=CTYPE_PTR;
    r->ptr=ctype;
    r->size=8;
    list_push(ctypes,r);
    return r;
}

static Ctype *make_array_type(Ctype *ctype,int size){
    Ctype *r=malloc(sizeof(Ctype));
    r->type=CTYPE_ARRAY;
    r->ptr=ctype;
    r->size=(len<0) ? -1:ctype->size*len;
    r->len=len;
    list_push(ctypes,r);
    return r;
}


static Ctype *make_struct_type(Dict *fields,int size){
    Ctype *r=malloc(sizeof(Ctype));
    r->type=CTYPE_STRUCT;
    r->fields=fields;
    r->size=size;
    list_push(ctypes,r);
    return r;
}


