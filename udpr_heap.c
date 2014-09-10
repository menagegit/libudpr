#include "udpr_heap.h"

static void swap(udpr_heap *h, size_t e1, size_t e2)
{
    udpr_heap_elem tmp = h->buffer[e1];
    h->buffer[e1] = h->buffer[e2];
    h->buffer[e2] = tmp;
}

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

udpr_status udpr_heap_init(udpr_heap *h)
{
    size_t i = 0;

    for(i = 0; i < HEAP_SIZE; i += 1)
    {
        h->buffer[i].packet = NULL;
        h->buffer[i].endpoint = NULL;
    }
    h->len = 0;
}

udpr_status udpr_heap_pop(
    udpr_heap *h,
    udpr_packet **p,
    udpr_endpoint **e,
    uint64_t *k)
{
    size_t root = 0;    

    if(h->len <= 0)
        { return ko; /* empty heap */ }

    *p = h->buffer[0].packet;
    *e = h->buffer[0].endpoint;
    *k = h->buffer[0].key;

    swap(h, root, h->len - 1);
    h->len -= 1;

    /* son1 check is useless, cause if son1 is out, son2 = son1 + 1 is out */
    while(son2(root) < h->len && son1(root) < h->len)
    {
        if(h->buffer[root].key > h->buffer[son1(root)].key
            && h->buffer[son1(root)].key <= h->buffer[son2(root)].key)
        {
            swap(h, root, son1(root));
            root = son1(root);
        }
        else if(h->buffer[root].key > h->buffer[son2(root)].key)
        {
            swap(h, root, son2(root));
            root = son2(root);
        }
        else
        {
            return ok;
        }
    }
    return ok;
}

udpr_status udpr_heap_push(
    udpr_heap *h,
    udpr_packet *p,
    udpr_endpoint *e,
    uint64_t key)
{
    int curr = 0;

    if(h->len >= HEAP_SIZE)
        { return ko; /* full */ }

    curr = h->len;
    h->len += 1;

    h->buffer[curr].packet = p;
    h->buffer[curr].endpoint = e;
    h->buffer[curr].key = key;

    while(curr > 0 && h->buffer[curr].key < h->buffer[parent(curr)].key)
    {
        swap(h, curr, parent(curr));
        curr = parent(curr);
    }
}

