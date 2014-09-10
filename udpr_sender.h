#ifndef UDPR_SENDER_H
#define UDPR_SENDER_H

//#include "udpr_core_fsm.h"
#include "udpr_packet.h"
#include "udpr_endpoint.h"
#include "udpr_status.h"

/* TODO shit like one timestamp and one queue for all dest. cool
 * you r getting better every day ! */

#include <stdint.h>

/* forward decl */
typedef struct udpr_core_fsm udpr_core_fsm;
typedef struct udpr_handler udpr_handler;

typedef struct udpr_sender udpr_sender;

#define REMEMBER_SIZE 32

struct pair_packet_endpoint
{
    udpr_packet *packet;
    udpr_endpoint *endpoint;
};

struct udpr_sender
{
    struct pair_packet_endpoint sent[REMEMBER_SIZE];
    uint64_t sent_index;
    uint64_t current_sent_ts;
    udpr_packet *received_packet;
    udpr_core_fsm *udpr_fsm;
    udpr_handler *handler;
};

void udpr_sender_init(udpr_sender*, udpr_handler*, udpr_core_fsm*);
udpr_status udpr_sender_execute_remote_action(
        udpr_sender*,
        uint64_t,
        udpr_packet*,
        udpr_endpoint*);

udpr_status udpr_sender_send_packet(
        udpr_sender*,
        udpr_packet*,
        udpr_endpoint*);

udpr_status sender_ask_for(udpr_sender*, udpr_endpoint*, uint64_t);

#endif /* UDPR_SENDER_H */
