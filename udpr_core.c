#include "udpr.h"

#include <stdlib.h>
#include <stdio.h>

// roadmap:
//
// impl udpr_core_fsm
//  += state: missing, timeout
//  += messages: on_itmeout


static void data_on_network(void *user_data, udpr_packet *packet, udpr_endpoint *from)
{
    udpr_handler *handler = (udpr_handler*) user_data;

    udpr_core_fsm_data_arrived(&(handler->udpr_fsm), packet, from);
    udpr_core_fsm_run(&(handler->udpr_fsm));
}

void udpr_init(udpr_handler *handler)
{
    handler->net_layer = create_network_layer();
    udpr_core_fsm_init(&(handler->udpr_fsm), handler, &(handler->sender)); // TODO init that shit .
    udpr_sender_init(&(handler->sender), handler, &(handler->udpr_fsm));

    if (handler->net_layer == NULL)
        { printf("create_network_layer failed, %s:%d", __FILE__, __LINE__); return ; }

    register_data_arrived(handler->net_layer, &data_on_network, handler);
}

void udpr_register_callbacks(
    udpr_handler *handler,
    packet_arrived_cb packet_cb,
    out_of_sync_cb oos_cb)
{
    udpr_core_fsm_set_cb(&(handler->udpr_fsm), packet_cb, oos_cb);
}

void udpr_send(udpr_handler *handler, udpr_packet *packet, udpr_endpoint *to)
{
    udpr_sender_send_packet(&(handler->sender), packet, to);
}

