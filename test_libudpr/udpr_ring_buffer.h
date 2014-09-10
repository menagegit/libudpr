#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <limits.h>

#include "udpr_fsm_event.h"
#include "udpr_status.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define RING_BUFFER_SIZE 16

/* RING_BUFFER_SIZE must be a multiple of MAX_INT */
/*#if (RING_BUFFER_SIZE % MAX_INT) != 0
#error "RING_BUFFER_SIZE must be a multiple of MAX_INT"
#endif*/

/* TODO mesure influence of write contention when many writer */

typedef struct udpr_ring_buffer
{
    udpr_fsm_event data[RING_BUFFER_SIZE];
    uint32_t r, w; /* next read / write index (mudulo size) */
    int32_t maxr, maxw; /* max concurent reader / writer allowed */

    /* signed integer because they a decremented concurently */
    int32_t nr, nw; /* number of reader / writer remaining */
    int32_t sr, sw; /* number of free slot for reading / writing */
}
udpr_ring_buffer;

void udpr_ring_buffer_init(udpr_ring_buffer* b, uint32_t max_r, uint32_t max_w);

udpr_status udpr_ring_buffer_push(udpr_ring_buffer* b, udpr_fsm_event* in);
udpr_status udpr_ring_buffer_pop(udpr_ring_buffer* b, udpr_fsm_event* out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RING_BUFFER_H */
