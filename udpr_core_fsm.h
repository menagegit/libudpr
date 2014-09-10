#ifndef UDP_CORE_FSM_H
#define UDP_CORE_FSM_H

#include "udpr_status.h"
#include "udpr_packet.h"
#include "udpr_endpoint.h"
#include "udpr_sender.h"
#include "udpr_ring_buffer.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct udpr_handler;

typedef void (*packet_arrived_cb)(struct udpr_handler*, udpr_packet*, udpr_endpoint*);
typedef void (*out_of_sync_cb)(struct udpr_handler*, udpr_endpoint*);

typedef struct fsm_state fsm_state;

typedef struct udpr_core_fsm_context
{
    uint64_t expected_seq;
    struct udpr_handler *parent;
    udpr_sender *sender;
    packet_arrived_cb packet_cb;
    out_of_sync_cb oos_cb;
}
udpr_core_fsm_context;

typedef struct udpr_core_fsm
{
    fsm_state *current_state;
    udpr_core_fsm_context context;
    udpr_ring_buffer events;
}
udpr_core_fsm;

udpr_status udpr_core_fsm_init(
        udpr_core_fsm*,
        struct udpr_handler*,
        udpr_sender*);
udpr_status udpr_core_fsm_run(udpr_core_fsm*);

void udpr_core_fsm_set_cb(udpr_core_fsm*, packet_arrived_cb, out_of_sync_cb);
udpr_status udpr_core_fsm_data_arrived(udpr_core_fsm*, udpr_packet*, udpr_endpoint*);
udpr_status udpr_core_fsm_timestamp_mismatch(udpr_core_fsm*, udpr_packet*, udpr_endpoint*);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UDP_CORE_FSM_H */
