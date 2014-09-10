#include "udpr_protobuf.h"

#include <stdlib.h>
#include <stdio.h>

#define MAX_VARINT_SIZE 10

int pb_encode_varint(udpr_packet_writer write, udpr_packet *p, uint64_t src)
{
    size_t i = 0;
    uint8_t buffer[MAX_VARINT_SIZE];

    if (src == 0)
    {
        buffer[i] = 0;
        i = 1;
    }

    while (src != 0)
    {
        if(i >= MAX_VARINT_SIZE)
            { printf("buffer too short in encode varint\n"); return -1; }

        buffer[i] = (uint8_t)((src & 0x7f) | 0x80);
        src >>= 7;
        i += 1;
    }
    buffer[i-1] &= 0x7F; /* Unset top bit on last byte */
    return write(p, buffer, i);
}

int pb_decode_varint(udpr_packet_reader read, udpr_packet *p, uint64_t *dest)
{
    uint8_t byte;
    uint8_t bitpos = 0;
    uint64_t result = 0;
    int r = 0, tmp = 0;

    do
    {
        tmp = read(p, &byte, 1);

        if (bitpos >= 64)
            { printf("varint overflow; %s:%d\n", __FILE__, __LINE__); return 0; }
        if (tmp == 0)
            { printf("no more input; %s:%d\n", __FILE__, __LINE__); return 0; }

        r += tmp;

        result |= ((uint64_t)(byte & 0x7F)) << bitpos;
        bitpos = (uint8_t)(bitpos + 7);
    } while (byte & 0x80);

    *dest = result;
    return r;
}
