#ifndef _STACK_H_
#define _STACK_H_


typedef enum
{
    false,
    true
} pop_mode;

typedef struct _node
{
    char *data;
    size_t level;
    size_t inode;
    struct _node *next;
} node_t;

typedef struct _stack
{
    struct _node *top;
} stack_t;

stack_t     *init_stack(void);
node_t      *init_node(char *txt, size_t level, size_t inode);
void        push(stack_t *stk, node_t *new_node);
void        free_node(node_t *node);
node_t      *pop(stack_t *stk, pop_mode mode);
void        free_stack(stack_t *stk);
int         is_free(const stack_t *stk);

#endif //_STACK_H_