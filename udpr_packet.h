#ifndef UDPR_PACKET_H
#define UDPR_PACKET_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

typedef struct udpr_packet udpr_packet;

// TODO create new allocate packet
// one for data, to keep room for the header

/* TODO add function here for read and write
 * so it is encapsulated... verify that it is a good idea */

int udpr_packet_buffer_size();

/* TODO move that to an allocator, which is a seperate class (?) */
udpr_packet *udpr_packet_allocate();
void udpr_packet_release(udpr_packet**);

int udpr_packet_write(udpr_packet*, uint8_t*, int);
int udpr_packet_read(udpr_packet*, uint8_t*, int);
void udpr_packet_reset(udpr_packet*);
void udpr_packet_goto_header(udpr_packet*);
uint8_t udpr_packet_first(udpr_packet *p);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UDPR_PACKET_H */
