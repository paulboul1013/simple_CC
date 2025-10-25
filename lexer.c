#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "cc.h"

bool is_punct(const Token tok,int c){
    return (get_ttype(tok)==TTYPE_PUNCT) && (get_punct(tok)==c);
}