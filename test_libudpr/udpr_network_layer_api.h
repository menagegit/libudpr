#ifndef UDPR_NETWORK_LAYER_API_H
#define UDPR_NETWORK_LAYER_API_H

#include "udpr_endpoint.h"
#include "udpr_packet.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void (*on_network_data_arrived_cb)(void*, udpr_packet*, udpr_endpoint*);

void *create_network_layer();
void send_data(void *net, udpr_packet *data, udpr_endpoint *to);
void register_data_arrived(void*, on_network_data_arrived_cb, void*);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UDPR_NETWORK_LAYER_API_H */
