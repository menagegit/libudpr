#include "udpr_core_fsm.h"
#include "udpr_header_decoder.h"

#include <stdio.h>

/* TODO factoriser le code, en creant un package avec tout le code
 * des fsm (ring buffre, events, fsm_impl ect...); */

const int EVENT_DATA_ARRIVED = 0;
const int EVENT_TIMESTAMP_MISMATCH = 1;

typedef fsm_state* (*fsm_transition)(void*, void*, void*);

fsm_state* synced_data_available(void*, void*, void*);
fsm_state *synced_missing(void*, void*, void*);
fsm_state* missing_data_available(void*, void*, void*);

static int burn_all_saved_data();

/* STATE MACHINE IMPL */

typedef struct fsm_state
{
    fsm_transition* transitions;
}
fsm_state;

fsm_transition synced_transitions[] =
    { synced_data_available, synced_missing }; /* TODO add on_missing and on_timeout */

fsm_transition missing_transitions[] =
    { missing_data_available, NULL }; /* TODO add on_missing and on_timeout */

fsm_state synced = { synced_transitions };
fsm_state missing = { missing_transitions };

fsm_state *synced_data_available(void *context, void *arg1, void *arg2)
{
    udpr_header_decoder decoder;
    udpr_packet *packet = (udpr_packet*) arg1;
    udpr_endpoint *from = (udpr_endpoint*) arg2;
    udpr_core_fsm_context *ctx = (udpr_core_fsm_context*) context;

    /* TODO the decoder do not need to have ctx->expected_seq */
    udpr_decoder_init(&decoder, packet, from, ctx->sender, ctx->expected_seq);
    udpr_decoder_run(&decoder);

    if(decoder.decoder_result == ctx->expected_seq)
    {
        printf("synced\n");
        ctx->expected_seq += 1;
        ctx->packet_cb(ctx->parent, packet, from);
        return &synced;
    }
    else if(decoder.decoder_result < ctx->expected_seq)
    {
        printf("already received, dropping\n");
        return &synced;
    }
    else /* result > ctx->last_seq_received */
    {
        printf("missing\n");
        udpr_heap_push(&(ctx->old_packets), packet, from, ctx->expected_seq);
        sender_ask_for(ctx->sender, from, ctx->expected_seq); /* TODO check errors */
        return &missing;
    }

    /* TODO :
     * result == 0 => expected packet received
     * result >  0 => missed packet
     * result <  0 => control or already received (to ignore)
    * for now the timestamp is interpretated as control || !control... so the first packet is dropped
    *if(decoder.decoder_result == 0) was control
    *    { printf("was control\n"); /* do nothing return &synced; }
    else /* was packet */
}

fsm_state *synced_missing(void *context, void *arg1, void *arg2)
{
    printf("missing in fsm\n");
    return &synced;
}

fsm_state *missing_data_available(void *context, void *arg1, void *arg2)
{
    udpr_header_decoder decoder;
    udpr_packet *packet = (udpr_packet*) arg1;
    udpr_endpoint *from = (udpr_endpoint*) arg2;
    udpr_core_fsm_context *ctx = (udpr_core_fsm_context*) context;
    uint64_t key = 0;

    udpr_decoder_init(&decoder, packet, from, ctx->sender, ctx->expected_seq);
    udpr_decoder_run(&decoder);

    if(decoder.decoder_result == ctx->expected_seq)
    {
        ctx->packet_cb(ctx->parent, packet, from);
    }


    return &synced;

    /* HEAD HERE if expected packet => 
        ctx->last_seq_received += 1;
        return &synced; */
    /* else if still missed stay here */
    /* else if already received => drop & return sync */
}

static int burn_all_saved_data()
{
    /* SHOULD BE REPLACED BY A FSM (synchronous)
     res = heap.pop(expectedkey);
    if(res == ko)
        if(!heap.empty())
            sender.ask_for(expected_seq);
            stay in missing
        else
            go to stuff. fait chier
    */
}

/* STATE MACHINE API */

udpr_status udpr_core_fsm_init(
        udpr_core_fsm *fsm,
        struct udpr_handler *parent,
        udpr_sender *sender)
{
    udpr_ring_buffer_init(&(fsm->events), 2, 2);
    fsm->current_state = &synced;
    fsm->context.expected_seq = 0;
    fsm->context.parent = parent;
    fsm->context.sender = sender;
    udpr_heap_init(&(fsm->context.recvd_packets));

    return ok;
}

udpr_status udpr_core_fsm_run(udpr_core_fsm *fsm)
{
    udpr_fsm_event event;

    while(1)
    {
        if(udpr_ring_buffer_pop(&(fsm->events), &event) == ko)
            { return ko; }
        fsm->current_state =
            fsm->current_state->transitions[event.id]((void*) &(fsm->context), event.arg1, event.arg2);
    }
    return ok;
}

udpr_status udpr_core_fsm_data_arrived(udpr_core_fsm *fsm, udpr_packet *p, udpr_endpoint *from)
{
    udpr_fsm_event e;

    e.arg1 = p;
    e.arg2 = from;
    e.id = EVENT_DATA_ARRIVED;

    udpr_ring_buffer_push(&(fsm->events), &e);

    return ok;
}

udpr_status udpr_core_fsm_timestamp_mismatch(udpr_core_fsm *fsm, udpr_packet *p, udpr_endpoint *from)
{
    udpr_fsm_event e;

    e.arg1 = p;
    e.arg2 = from;
    e.id = EVENT_TIMESTAMP_MISMATCH;

    udpr_ring_buffer_push(&(fsm->events), &e);

    return ok;
}

void udpr_core_fsm_set_cb(udpr_core_fsm *fsm, packet_arrived_cb p_cb, out_of_sync_cb oos_cb)
{
    fsm->context.packet_cb = p_cb;
    fsm->context.oos_cb = oos_cb;
}

