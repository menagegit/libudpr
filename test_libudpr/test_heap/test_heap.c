#include "udpr_heap.h"

#include <stdio.h>

udpr_heap heap;

static size_t son1(size_t root)
{
     return 2 * root + 1;
}

static size_t son2(size_t root)
{
     return 2 * root + 2;
}

static size_t parent(size_t son)
{
    return (son - 1) / 2;
}

int check()
{
    int i = 0;

    for(i = 0; i < heap.len && heap.len > 1; i += 1)
    {
        if(son1(i) < heap.len && heap.buffer[i].key > heap.buffer[son1(i)].key)
        {
            printf("condition not respected (%d)\n", i);
            return -1;
        }
        if(son2(i) < heap.len && heap.buffer[i].key > heap.buffer[son2(i)].key)
        {
            printf("condition not respected (%d)\n", i);
            return -1;
        }
    }
}

int test_pop()
{
    int i = 0, p, e, j;

    for(i = 19; i >= 0; i -= 1)
    {
        udpr_heap_push(&heap, (void*) i, (void*) i, i);
        for(j = 0; j < heap.len; j += 1)
        {
            printf("%d ", heap.buffer[j].packet);
        }
        printf("\n");
        check();
    }
    for(i = 0; i < 20; i += 1)
    {
        udpr_heap_pop(&heap, (void**) &p, (void**) &e);
        for(j = 0; j < heap.len; j += 1)
        {
            printf("%d ", heap.buffer[j].packet);
        }
        printf("\n");
        printf("p %d, e %d\n", p, e);
        if(p != e || p != i)
            { printf("test failed\n"); }
    }
}

int main()
{
    udpr_heap_init(&heap);
    test_pop();
}

