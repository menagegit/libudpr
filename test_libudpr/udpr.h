#ifndef UDPR_H
#define UDPR_H

#include "udpr_network_layer_api.h"
#include "udpr_packet.h"
#include "udpr_core_fsm.h"
#include "udpr_sender.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct udpr_handler udpr_handler;

struct udpr_handler
{
    /* TODO cool to have void !? it allows me to be full c here */
    void *net_layer;
    udpr_sender sender;
    udpr_core_fsm udpr_fsm;
};

void udpr_init(udpr_handler*);
void udpr_register_callbacks(udpr_handler*, packet_arrived_cb, out_of_sync_cb);
void udpr_send(udpr_handler*, udpr_packet*, udpr_endpoint*);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UDPR_H */
