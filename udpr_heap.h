#ifndef UDPR_HEAP_H
#define UDPR_HEAP_H

#include "udpr_status.h"
#include "udpr_packet.h"
#include "udpr_endpoint.h"

#include <stdlib.h>
#include <stdint.h>

#define HEAP_SIZE 32

typedef struct udpr_heap_elem
{
    uint64_t key;
    udpr_packet *packet;
    udpr_endpoint *endpoint;
}
udpr_heap_elem;

typedef struct udpr_heap
{
    udpr_heap_elem buffer[HEAP_SIZE];
    int len;
}
udpr_heap;

udpr_status udpr_heap_init(udpr_heap*);

udpr_status udpr_heap_pop(
    udpr_heap *h,
    udpr_packet**,
    udpr_endpoint**,
    uint64_t*);

udpr_status udpr_heap_push(
    udpr_heap*,
    udpr_packet*,
    udpr_endpoint*,
    uint64_t);

#endif /* UDPR_HEAP_H */

