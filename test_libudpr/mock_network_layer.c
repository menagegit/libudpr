#include "udpr_network_layer_api.h"

#include <stdlib.h>
#include <stdio.h>

#include "test_udpr.h"

static int ID = 0;

void send_to_h1(udpr_packet *data);
void send_to_h2(udpr_packet *data);

mock_net_layer *nets[2];

void *create_network_layer()
{
    mock_net_layer *n = malloc(sizeof(mock_net_layer));
    n->id = ID++;
    n->handler = NULL;
    n->handler_data = NULL;
    nets[n->id] = n;

    return n;
}

void send_data(void *this, udpr_packet *data, udpr_endpoint *to)
{
    mock_net_layer *net = (mock_net_layer*) this;

    if(net->id == 0)
        send_to_h2(data);
    else if(net->id == 1)
        send_to_h1(data);
    else
        printf("wtf net id is not correct !\n");
}

void register_data_arrived(void *this, on_network_data_arrived_cb h, void *data)
{
    mock_net_layer *net = (mock_net_layer*) this;

    net->handler = h;
    net->handler_data = data;
}

