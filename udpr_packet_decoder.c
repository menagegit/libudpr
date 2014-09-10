#include "udpr_header_decoder.h"

#include "udpr_protobuf.h"

#include <stdio.h>

/* TODO put it in a file called protocol constant */

const uint32_t CONTROL_PACKET_TYPE = 0;
const uint32_t DATA_PACKET_TYPE = 1;

static uint8_t is_finished_state(dec_fsm_transition *t);

/* STATE MACHINE IMPL */

struct dec_fsm_transition
{
    uint32_t (*condition)(udpr_header_decoder*);
    void (*action)(udpr_header_decoder*);
    dec_fsm_transition *next;
};

extern dec_fsm_transition initial[];
extern dec_fsm_transition type_parsed[];
extern dec_fsm_transition data[];

/* Conditions */

static uint32_t always_true(udpr_header_decoder *fsm)
{
    return 1;

    /* unused */
    (void) fsm;
}

static uint32_t is_control(udpr_header_decoder *decoder)
{
    return (decoder->decoder_result == CONTROL_PACKET_TYPE);
}

static uint32_t is_data(udpr_header_decoder *decoder)
{
    return (decoder->decoder_result == DATA_PACKET_TYPE);
}

static uint32_t ts_ok(udpr_header_decoder *fsm)
{
    /*data_handler_context_t *ctx = (data_handler_context_t*) fsm->context;
    receiver_context_t *parent_ctx = (receiver_context_t*) fsm->parent_context;
    return (uint8_t) (ctx->parsed_int == parent_ctx->current_ts + 1);*/
    return 1;
}

static uint32_t miss_packet(udpr_header_decoder *fsm)
{
    /*data_handler_context_t *ctx = (data_handler_context_t*) fsm->context;
    receiver_context_t *parent_ctx = (receiver_context_t*) fsm->parent_context;
    return (uint8_t) (ctx->parsed_int > parent_ctx->current_ts + 1);*/
    return 1;
}

static uint32_t already_received(udpr_header_decoder *fsm)
{
    /*data_handler_context_t *ctx = (data_handler_context_t*) fsm->context;
    receiver_context_t *parent_ctx = (receiver_context_t*) fsm->parent_context;
    return (uint8_t) (ctx->parsed_int <= parent_ctx->current_ts);*/
    return 1;
}

/* Actions */

static void do_parse(udpr_header_decoder *decoder)
{
    uint64_t seq = 0;
    int read = 0;

    read = pb_decode_varint(
                &udpr_packet_read,
                decoder->packet,
                &seq);

    if(read == -1) { printf("error decoding\n"); }

    decoder->decoder_result = seq;
}

static void execute_control(udpr_header_decoder *decoder)
{
    uint64_t control_id = 0;

    pb_decode_varint(
                &udpr_packet_read,
                decoder->packet,
                &control_id);

    udpr_sender_execute_remote_action(
                decoder->sender,
                control_id,
                decoder->packet,
                decoder->from);

    decoder->decoder_result = 0; /* control packet */
}

static void handle_miss(udpr_header_decoder *fsm)
{
    /*receiver_context_t *parent_ctx = (receiver_context_t*) fsm->parent_context;
    data_handler_context_t *ctx = (data_handler_context_t*) fsm->context;

    sender_ask_for(parent_ctx->sender, parent_ctx->current_ts + 1);
    insert_sorted_list(&(parent_ctx->postponed_packet), ctx->curr_packet, ctx->parsed_int);
    ctx->result = 1; /* missed */
}

static void drop_packet(udpr_header_decoder *fsm)
{
    /*data_handler_context_t *ctx = (data_handler_context_t*) fsm->context;
    /* do nothing - TODO do we need to release packet ? */
    /*ctx->result = -1; /* already received */
}

static void process_packet(udpr_header_decoder *decoder)
{
    /*receiver_context_t *parent_ctx = (receiver_context_t*) fsm->parent_context;
    data_handler_context_t *ctx = (data_handler_context_t*) fsm->context;
    parent_ctx->app_layer->data_arrived(parent_ctx->app_layer->this, ctx->curr_packet);
    parent_ctx->current_ts += 1;
    ctx->result = 0; /* expected received */

    decoder->decoder_result = 1; /* data */
}

static void handle_bad_type(udpr_header_decoder *fsm)
{
    /*data_handler_context_t *ctx = (data_handler_context_t*) fsm->context;
    /* just log some stuff ? */
    //ctx->result = 0;
}

dec_fsm_transition initial[] =
{
    { always_true, do_parse, type_parsed },
    { NULL, NULL, NULL }
};

dec_fsm_transition type_parsed[] =
{
    { is_data, do_parse, /*data*/ NULL },
    { is_control, execute_control, NULL },
    { always_true, handle_bad_type, NULL },
    { NULL, NULL, NULL }
};

dec_fsm_transition data[] =
{
    { ts_ok, process_packet, NULL },
    { miss_packet, handle_miss, NULL },
    { already_received, drop_packet, NULL },
    { NULL, NULL, NULL }
};

/* PUBLIC API IMPL */

void udpr_decoder_init(
        udpr_header_decoder *decoder,
        udpr_packet *p,
        udpr_endpoint *e,
        udpr_sender *s,
        uint64_t seq)
{
    decoder->current = initial;
    decoder->sender = s;
    decoder->packet = p;
    decoder->from = e;
    decoder->expected_seq = seq;
    decoder->decoder_result = 0;
}

/* TODO void */
int udpr_decoder_run(udpr_header_decoder *decoder)
{
    dec_fsm_transition *t = NULL;

    while(!is_finished_state(decoder->current))
    {
        t = decoder->current;
        while(t->condition != NULL)
        {
            if(t->condition(decoder))
            {
                t->action(decoder);
                decoder->current = t->next;
                break;
            }
            t += 1; /* move to the next transition */
        }
        if(t->condition == NULL)
            { printf("no conditions found, error !\n"); return 0; }
    }
    return 1;
}

/* HELPERS */

static uint8_t is_finished_state(dec_fsm_transition *t)
{
    return t == NULL;
}

