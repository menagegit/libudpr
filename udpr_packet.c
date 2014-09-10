#include "udpr.h"

#include <stdlib.h>
#include <stdint.h>

#include <stdio.h>

#define PACKET_BUFFER_SIZE 482 /* 512 - 30 */
#define PACKET_HEADER_SIZE 30 /* max_varint * 3 */

struct udpr_packet
{
    uint8_t buffer[PACKET_BUFFER_SIZE];
    uint8_t header[PACKET_HEADER_SIZE];
    uint8_t *r, *w, *wstop;
    int rlen, dlen;
};

int udpr_packet_buffer_size()
{
    return PACKET_BUFFER_SIZE + PACKET_HEADER_SIZE; /* == 512 */
}

udpr_packet *udpr_packet_allocate()
{
    udpr_packet *p = NULL;
    p = malloc(sizeof(udpr_packet));

    if(p != NULL)
    {
        udpr_packet_reset(p);
    }

    return p;
}

void udpr_packet_release(udpr_packet **packet)
{
    free(*packet);
    *packet = NULL;
}

int udpr_packet_write(udpr_packet *p, uint8_t *in, int ilen)
{
    int i = 0;

    for(i = 0; i < ilen && p->w < p->wstop; i += 1)
    {
        *(p->w) = in[i];
        p->w += 1;
    }
    p->rlen += i;

    return i;
}

int udpr_packet_read(udpr_packet *p, uint8_t *out, int olen)
{
    int i = 0;

    for(i = 0; i < olen && p->rlen > 0; i += 1)
    {
        *out = *(p->r);
        out += 1;
        p->r += 1;
        p->rlen -= 1;

        if(p->rlen == p->dlen) /* re think this shit ( when to switch to buffer ) */
            { p->r = p->buffer; }
    }
    return i;
}

void udpr_packet_reset(udpr_packet *p)
{
    p->w = p->buffer; /* start by writing the payload */
    p->r = p->header; /* start by reading the header */
    p->wstop = p->buffer + PACKET_BUFFER_SIZE;
    p->rlen = 0;
    p->dlen = 0;
}

void udpr_packet_goto_header(udpr_packet *p)
{
    p->dlen = p->rlen; /* we have written only to the data */
    p->w = p->header;
    p->wstop = p->header + PACKET_HEADER_SIZE;
}

/* TESTING ONLY ! return the next byte to read, do not advance the reader */
uint8_t udpr_packet_first(udpr_packet *p)
{
    return p->buffer[0];
}

// implement packet stuff here
// First = what functionnalities do we need ?
