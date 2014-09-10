#ifndef UDPR_DECODER_H
#define UDPR_DECODER_H

#include <stdint.h>

#include "udpr_packet.h"
#include "udpr_sender.h"
#include "udpr_endpoint.h"

typedef struct dec_fsm_transition dec_fsm_transition;

typedef struct udpr_header_decoder
{
    dec_fsm_transition *current;
    udpr_sender *sender;
    udpr_packet *packet;
    udpr_endpoint *from;
    uint64_t expected_seq;
    uint32_t decoder_result;
}
udpr_header_decoder;

void udpr_decoder_init(
        udpr_header_decoder*,
        udpr_packet*,
        udpr_endpoint *e,
        udpr_sender*,
        uint64_t);

int udpr_decoder_run(udpr_header_decoder*);

#endif /* UDPR_DECODER_H */
