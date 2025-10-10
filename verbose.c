#include "cc.h"

char *ctype_to_string(Ctype *ctype){
    if (!ctype){
        return "(nil)";
    }
    switch (ctype->type){
        case CTYPE_VOID:
            return "void";
        case CTYPE_CHAR:
            return "char";
        case CTYPE_INT:
            return "int";
        case CTYPE_LONG:
            return "long";
        case CTYPE_FLOAT:
            return "float";
        case CTYPE_DOUBLE:
            return "double";
        case CTYPE_PTR: {
           String s=make_string();
           string_appendf(&s, "*%s", ctype_to_string(ctype->ptr));
           return get_cstring(s);
        } 
        case CTYPE_ARRAY: {
           String s=make_string();
           string_appendf(&s, "[%d]%s", ctype->len, ctype_to_string(ctype->ptr));
           return get_cstring(s);
        }
        case CTYPE_STRUCT: {
            String s=make_string();
            string_appendf(&s, "(struct");
            for (Iter i=list_iter(dict_values(ctype->fields)); !iter_end(i);){
                string_appendf(&s, " (%s)", ctype_to_string(iter_next(&i)));
            }
            string_appendf(&s, ")");
            return get_cstring(s);
        }
        default:
            error("Unknown ctype: %d", ctype);
            return NULL;
    }
}

static void ast_to_string_int(String *buf, Ast *ast){

}
char *ast_to_string(Ast *ast){
    String s=make_string();
    ast_to_string_int(&s,ast);
    return get_cstring(s);
}

// static void uop_to_string(String *buf, char *op,Ast *ast){
//     string_appendf(buf,"(%s %s)",op,ast_to_string(ast->operand));
// }

// static void binop_to_string(String *buf, char *op, Ast *ast){
//     string_appendf(buf,"(%s %s %s)",op,ast_to_string(ast->left),ast_to_string(ast->right));
// }