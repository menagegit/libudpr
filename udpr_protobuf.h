#ifndef UDPR_PROTOBUF_H
#define UDPR_PROTOBUF_H

#include <stdint.h>

#include "udpr_packet.h"

typedef int (*udpr_packet_reader)(udpr_packet*, uint8_t*, int);
typedef int (*udpr_packet_writer)(udpr_packet*, uint8_t*, int);

int pb_decode_varint(udpr_packet_reader, udpr_packet*, uint64_t *dest);
int pb_encode_varint(udpr_packet_writer, udpr_packet*, uint64_t);

#endif /* UDPR_PROTOBUF_H */
