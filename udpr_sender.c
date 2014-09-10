#include "udpr_sender.h"

#include "udpr_protobuf.h"
#include "udpr_core_fsm.h"
#include "udpr_network_layer_api.h"
#include "udpr.h"

#include <stdlib.h>
#include <stdio.h>

/* TODO move to util.stuff.lol.c */
static uint64_t atomic_get_inc(uint64_t *i)
{
    return __sync_fetch_and_add(i, 1);
}

typedef udpr_status (*control_function_t)(udpr_sender*, udpr_packet*, udpr_endpoint*);

#define EXPAND_AS_ENUM(a,b) a,
#define EXPAND_AS_PROTOTYPE(a,b) static udpr_status (b)(udpr_sender*, udpr_packet*, udpr_endpoint*);
#define EXPAND_AS_JUMPTABLE(a, b) (b),

#define PROTOCOL_ACTIONS(ENTRY)   \
    ENTRY(RESEND, do_resend)	\
    ENTRY(REMOTE_OUT_OF_SYNC, do_remote_out_of_sync) \
    ENTRY(SEND_CURRENT_TS, do_send_current_ts) \
    ENTRY(HERE_IS_REMOTE_TS, do_check_remote_ts)

/* declare an enumeration for actions asked by remote */
typedef enum remote_orders
{
    PROTOCOL_ACTIONS(EXPAND_AS_ENUM)
    NUM_ACTIONS
} remote_orders;

PROTOCOL_ACTIONS(EXPAND_AS_PROTOTYPE)

control_function_t jumptable[NUM_ACTIONS] = {
    PROTOCOL_ACTIONS(EXPAND_AS_JUMPTABLE)
};

extern const uint32_t CONTROL_PACKET_TYPE;
extern const uint32_t DATA_PACKET_TYPE;

static udpr_status send_out_of_sync(udpr_sender *s, udpr_endpoint *to);

static udpr_status do_resend(udpr_sender *s, udpr_packet *p, udpr_endpoint *from)
{
    uint64_t expected_packet_ts = 0;
    int read = 0;

    read = pb_decode_varint(
                &udpr_packet_read,
                p,
                &expected_packet_ts);
    if(read < 0)
        { return ko; }

    if(s->current_sent_ts < expected_packet_ts)
        { return ok; /* bad arg from remote */ }

    if(s->current_sent_ts - expected_packet_ts > REMEMBER_SIZE)
    {
        send_out_of_sync(s, from);
        s->udpr_fsm->context.oos_cb(s->handler, from);
    }

    /* read ts from packet(varint=>move the code from decoder
     * if out available resend
     * else throw packet + send (out of sync) */

    return ok;
}

static udpr_status do_remote_out_of_sync(udpr_sender *s, udpr_packet *p, udpr_endpoint *from)
{
    s->udpr_fsm->context.oos_cb(s->udpr_fsm->context.parent, from);

    return ok;
    (void) p;
}

static udpr_status do_send_current_ts(udpr_sender *s, udpr_packet *p, udpr_endpoint *from)
{
    int written = 0;

    udpr_packet_reset(p);

    written = pb_encode_varint(udpr_packet_write, p, CONTROL_PACKET_TYPE);
    if(written == -1)
        { return ko; }
    written = pb_encode_varint(udpr_packet_write, p, HERE_IS_REMOTE_TS);
    if(written == -1)
        { return ko; }
    written = pb_encode_varint(udpr_packet_write, p, s->current_sent_ts);
    if(written == -1)
        { return ko; }

    /* just send it, no need to update the current_ts cause it is a control packet */
    send_data(s->handler->net_layer, p, from);

    return ok;
}

static udpr_status do_check_remote_ts(udpr_sender *s, udpr_packet *p, udpr_endpoint *from)
{
    printf("remote asked to verify that seq are ok\n");

    uint64_t remote_ts = 0;
    int read = 0;

    read = pb_decode_varint(udpr_packet_read, p, &remote_ts);
    if(read < 0)
        { return ko; }

    if(s->udpr_fsm->context.expected_seq - 1 != remote_ts)
    {
        udpr_core_fsm_timestamp_mismatch(s->udpr_fsm, p, from);
    }
    /* else the last message received was the last message sent */

    return ok;
}

static udpr_status send_out_of_sync(udpr_sender *s, udpr_endpoint *to)
{
    int written = 0;

    udpr_packet *p = udpr_packet_allocate();
    if(p == NULL)
        { return ko; }

    written = pb_encode_varint(udpr_packet_write, p, CONTROL_PACKET_TYPE);
    if(written == -1)
        { return ko; }
    written = pb_encode_varint(udpr_packet_write, p, REMOTE_OUT_OF_SYNC);
    if(written == -1)
        { return ko; }
    written = pb_encode_varint(udpr_packet_write, p, s->current_sent_ts);
    if(written == -1)
        { return ko; }

    /* just send it, no need to update the current_ts cause it is a control packet */
    send_data(s->handler->net_layer, p, to);

    udpr_packet_release(&p);

    return ok;
}

/* PUBLIC FUNCTIONS */

void udpr_sender_init(udpr_sender *s, udpr_handler *h, udpr_core_fsm *receiver)
{
    int i = 0;

    s->udpr_fsm = receiver;
    s->sent_index = 0;
    s->current_sent_ts = 0;
    s->received_packet = NULL;
    s->handler = h;

    for(i = 0; i < REMEMBER_SIZE; i += 1)
    {
        s->sent[i].packet = NULL;
        s->sent[i].endpoint = NULL;
    }
}

udpr_status udpr_sender_execute_remote_action(udpr_sender *s,
        uint64_t action,
        udpr_packet *p,
        udpr_endpoint *from)
{
    return jumptable[action](s, p, from);
}

udpr_status udpr_sender_send_packet(
        udpr_sender *s,
        udpr_packet *p,
        udpr_endpoint *to)
{
    size_t index = 0;
    uint64_t tmp = atomic_get_inc(&(s->sent_index));
    uint64_t current_ts = atomic_get_inc(&(s->current_sent_ts));
    index = (tmp % ((uint64_t) REMEMBER_SIZE));

    udpr_packet_goto_header(p);

    if(pb_encode_varint(udpr_packet_write, p, DATA_PACKET_TYPE) == -1)
        { return ko; }
    if(pb_encode_varint(udpr_packet_write, p, current_ts) == -1)
        { return ko; }

    struct pair_packet_endpoint *pair = &(s->sent[index]);

    if(pair->packet != NULL)
        { udpr_packet_release(&(pair->packet)); }
    if(pair->endpoint != NULL)
        { udpr_endpoint_release(&(pair->endpoint)); }

    pair->packet = p;
    pair->endpoint = to;

    send_data(s->handler->net_layer, p, to);

    /* TODO do we need error handling ? */
    return ok;
}

udpr_status sender_ask_for(udpr_sender *s, udpr_endpoint *to, uint64_t ts)
{
    int written = 0;

    udpr_packet *p = udpr_packet_allocate();
    if(p == NULL)
        { return ko; }

    written = pb_encode_varint(udpr_packet_write, p, CONTROL_PACKET_TYPE);
    if(written == -1)
        { return ko; }
    written = pb_encode_varint(udpr_packet_write, p, RESEND);
    if(written == -1)
        { return ko; }
    written = pb_encode_varint(udpr_packet_write, p, ts);
    if(written == -1)
        { return ko; }

    /* just send it, no need to update the current_ts cause it is a control packet */
    send_data(s->handler->net_layer, p, to);

    udpr_packet_release(&p);

    return ok;
}

