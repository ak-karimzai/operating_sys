// #include "stack_t.h"

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stddef.h>

#include "stack_t.h"

static void err_msg(const char *msg);

stack_t *init_stack(void)
{
    return malloc(sizeof(stack_t));
}

node_t *init_node(char *txt, size_t level, size_t inode)
{
    node_t *temp = malloc(sizeof(node_t));

    if (temp)
    {
        temp->data = strdup(txt);
        temp->level = level;
        temp->inode = inode;
        temp->next = NULL;
    }
    return temp;
}

void push(stack_t *stk, node_t *new_node)
{
    if (new_node == NULL)
    {
        err_msg("stackoverflow!");
        return;
    }

    if (stk->top == NULL)
        stk->top = new_node;
    else
    {
        new_node->next = stk->top;
        stk->top = new_node;
    }
}

void free_node(node_t *node)
{
    if (node)
    {
        free(node->data);
        free(node);
    }
}

node_t *pop(stack_t *stk, pop_mode mode)
{
    node_t *temp = stk->top;
    stk->top = stk->top->next;
    
    if (mode == false) free_node(temp);
    else return(temp);
    
    return NULL;
}

int is_free(const stack_t *stk)
{
    return stk->top == NULL;
}

void free_stack(stack_t *stk)
{
    while (!is_free(stk)) pop(stk, false);
    free(stk);
}

static void err_msg(const char *msg)
{
    fputs(msg, stderr);
    exit(EXIT_FAILURE);
}