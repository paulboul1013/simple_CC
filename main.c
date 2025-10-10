#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cc.h"


static char *outfile=NULL,*infile=NULL;
static FILE *outfp=NULL;
static bool ast_output=false;

void test_string(){
    String s=make_string();
    string_appendf(&s,"Hello, World!");
    string_appendf(&s,"aelle");
    string_append(&s,'#');
    printf("s: %s\n",get_cstring(s));

    assert(strcmp(get_cstring(s),"Hello, World!aelle#")==0);

    String s2=make_string();
    string_appendf(&s2,"hello \"world\"");
    
    printf("quote_cstring: %s\n",quote_cstring(get_cstring(s2)));
    assert(strcmp(quote_cstring(get_cstring(s2)),"hello \\\"world\\\"")==0);
    free(s.body);
    free(s2.body);
}

void test_list(){
    List *clist=make_list();
    ListNode *node,*tmp;
    list_push(clist,"Hello, World!");
    list_push(clist,"恨阿魷");
    list_push(clist,"aelle");
    list_push(clist,"gne");
    list_push(clist,"kk");

    list_for_each_safe(node,tmp,clist){
        printf("clist->elem: %s\n",(char *)node->elem);
    }

    list_pop(clist);

    clist=list_reverse(clist);
    printf("reverse-list\n");
    list_for_each_safe(node,tmp,clist){
        printf("clist->elem: %s\n",(char *)node->elem);
    }

    printf("clist->len: %d\n",list_len(clist));
    // list_free(clist);
}


static void usage_help(){
    fprintf(stdout,
          "cc [options] filename\n"
          "OPTIONS\n"
           "-o filename     write output to the specified file.\n"
           "--ast           ouput AST tree\n");
}

static void print_usage_and_exit(){
    usage_help();
    exit(1);
}

static void parse_args(int argc,char **argv){
    if (argc < 2){
        print_usage_and_exit();
    }

    while (true){
        //skip the first origin environment setting argument
        argc--;
        argv++; 
        if (!argc){ 
            break;
        }

        //if argument start with '-' ,it means it's a option
        if ((*argv)[0] =='-'){
            switch((*argv)[1]){
            
            case '\0':
                infile="/dev/stdin";
                // printf("file: %s\n",infile);
                break;
            
            case 'o':
                argc--;
                argv++;
                outfile=*argv;
                // printf("outfile: %s\n",outfile);
                break;
            
            case '-':
                if (!strcmp(*argv,"--ast")){
                    ast_output=true;
                    // printf("ast_output: %d\n",ast_output);
                    break;
                }

            default:
                print_usage_and_exit();
            
            }
        } else{
            if (infile){ //when the second non-option argument is not what we expect
                print_usage_and_exit();
            }
            infile=argv[0];
        }
    }
}

static void open_output_file(){
    if (outfile){
        if (!(outfp=fopen(outfile,"w"))){ //can't open output file
            printf("Can not open file %s\n",outfile);
            exit(1);
        }
    } else{
        outfp=stdout;
    }
}

static void open_input_file() {
    if (!infile){
        printf("Input file  is not given\n\n");
        print_usage_and_exit();
    }

    if (!freopen(infile,"r",stdin)){
        printf("Can't open file %s\n",infile);
        exit(1);
    }
}

int main(int argc ,char **argv){
    
    parse_args(argc,argv);
    open_input_file();
    open_output_file();

    test_list();
    test_string();


    return 0;
}