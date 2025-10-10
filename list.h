#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include <stdlib.h>

//basic list node
typedef struct __ListNode {
    void *elem;
    struct __ListNode *next, *prev;
}ListNode;

//doubly linked list
typedef struct {
    int len;
    ListNode *head,*tail;
} List;

typedef struct {
    ListNode *ptr;
} Iter;

#define EMPTY_LIST ((List){.len=0,.head=NULL,.tail=NULL})

static inline List *make_list(void){
    List *r=malloc(sizeof(List));
    r->len=0;
    r->head=r->tail=NULL;
    return r;
}

static inline void *make_node(void *elem){
    ListNode *r=malloc(sizeof(ListNode));
    r->elem=elem;
    r->next=r->prev=NULL;
    return r;
}

//push an element to the end of the list
static inline void list_push(List *list,void *elem){
    ListNode *node=make_node(elem);
    if (!list->head){
        list->head=node;
    }else{
        list->tail->next=node;
        node->prev=list->tail;
    }
    list->tail=node;
    list->len++;
}

//pop an element from the end of the list
static inline void *list_pop(List *list){
    if (!list->head){
        return NULL;
    }
    ListNode *tail_node=list->tail;
    void *r=tail_node->elem;
    list->tail=tail_node->prev; //move the tail to the previous node
    list->len--; 
    if (list->tail){
        list->tail->next=NULL; //to divide the list into two parts
    }else{
        list->head=NULL;
    }
    
    free(tail_node);
    return r; //return the popped element
}

//push an element to the beginning of the list
static void list_unshift(List *list,void *elem){
    ListNode *node=make_node(elem);
    node->next=list->head;
    if (list->head){
        list->head->prev=node;
    }
    list->head=node;
    if (!list->tail){
        list->tail=node;
    }
    list->len++;
}

//return an iterator for the list
static inline Iter list_iter(void *ptr){
    return (Iter){
        .ptr=((List *)ptr)->head,
    };
}

//check if the iterator is at the end of the list
static inline bool iter_end(const Iter iter){
    return !iter.ptr;
}

//get the next element from the iterator
static inline void *iter_next(Iter *iter){
    if (!iter->ptr){
        return NULL;
    }
    void *r=iter->ptr->elem;
    iter->ptr=iter->ptr->next;
    return r;
}

//reverse the list
static inline List *list_reverse(List *list){
    List *r=make_list();
    for (Iter i=list_iter(list);!iter_end(i);){
        list_unshift(r,iter_next(&i));
    }
    return r;
}

//get the length of the list
static inline int list_len(List *list){
    return list->len;
}

#define list_safe_next(node) ((node)? (node)->next:NULL)
#define list_for_each_safe(node,tmp,list)  \
    for ((node)=(list)->head,(tmp)=list_safe_next(node);(node); \
         (node)=(tmp),(tmp)=list_safe_next(node))


static inline void list_free(List *list){
    ListNode *node,*tmp;
    list_for_each_safe(node,tmp,list){
        free(node->elem);
        free(node);
    }
}

#endif