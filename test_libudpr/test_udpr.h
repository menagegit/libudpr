

typedef struct mock_net_layer
{
    on_network_data_arrived_cb handler;
    void *handler_data;
    int id;
}
mock_net_layer;
