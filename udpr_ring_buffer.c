#include "udpr_ring_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

inline uint32_t atomic_uinc(uint32_t* ptr)
{
    return __sync_fetch_and_add(ptr, 1);
}

inline int32_t atomic_inc(int32_t* ptr)
{
    return __sync_fetch_and_add(ptr, 1);
}

inline int32_t atomic_dec(int32_t* ptr)
{
    return __sync_fetch_and_sub(ptr, 1);
}

void udpr_ring_buffer_init(udpr_ring_buffer* b, uint32_t max_r, uint32_t max_w)
{
    b->r = 0;
    b->w = 0;

    b->maxr = max_r;
    b->maxw = max_w;

    b->nr = max_r;
    b->nw = max_w;

    b->sr = 0;
    b->sw = RING_BUFFER_SIZE;
}

udpr_status udpr_ring_buffer_push(udpr_ring_buffer *b, udpr_fsm_event* in)
{
    uint32_t local_index = 0;

    /* add a new writer */
    if(atomic_dec(&(b->nw)) <= 0)
        { atomic_inc(&(b->nw)); return ko; }

    /* remove one writable slot; if there is less than max_reader
     * slot, we can't write because a reader might be reading
     * one of the remaining slot */
    if(atomic_dec(&(b->sw)) <= b->maxr - b->nr)
        { atomic_inc(&(b->nw)); atomic_inc(&(b->sw)); return ko; }

    local_index = atomic_uinc(&(b->w));
    b->data[local_index % RING_BUFFER_SIZE] = *in;

    /* one more slot can be read and release one writer */
    atomic_inc(&(b->sr));
    atomic_inc(&(b->nw));

    return ok;
}

udpr_status udpr_ring_buffer_pop(udpr_ring_buffer* b, udpr_fsm_event* out)
{
    uint32_t local_index = 0;

    if(atomic_dec(&(b->nr)) < 0)
        { atomic_inc(&(b->nr)); return ko; }

    if(atomic_dec(&(b->sr)) <= b->maxw - b->nw)
        { atomic_inc(&(b->nr)); atomic_inc(&(b->sr)); return ko; }

    local_index = atomic_uinc(&(b->r));
    *out = b->data[local_index % RING_BUFFER_SIZE];

    atomic_inc(&(b->sw));
    atomic_inc(&(b->nr));

    return ok;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
