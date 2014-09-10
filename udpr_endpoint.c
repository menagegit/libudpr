#include "udpr_endpoint.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/* TODO replace all int returns by proper error handling */
/* TODO add const in args, and when needed ! */
/* ca comment a pas etre joli du tout, cf to_str... */

typedef int (*nb_to_str_func)(const char*, int, char*, const char*);

struct udpr_endpoint
{
    char ip[16];
    uint32_t len;
    uint16_t port;
    nb_to_str_func to_str;
    char sep; /* separator in the string */
    int nb_size;
};

static int ipv4_nb_to_str(const char *in, int ilen, char *dest, const char *guard)
{
    int res = 0;
    /* in case nb == 0 */
    uint8_t nb, div = 1;

    if(ilen < 1)
        { return 0; } /* input overflow ! */

    nb = *in;

    while(nb / div > 10)
        { div *= 10; }
    do
    {
        if(dest >= guard) /* overflow (equals because of '\0') */
            { return 0; }
        *(dest + res) = '0' + nb / div;
        res += 1;
        nb %= div;
        div /= 10;
    } while(nb != 0);
    return res;
}

static int ipv6_nb_to_str(const char* in, int ilen, char *dest, const char *guard)
{
    /* in case nb == 0 */
    uint16_t nb, mask = 0xf000;
    char c = '0';
    uint32_t i = 0;

    if(ilen < 2 || dest + 4 >= guard)
        { return 0; } /* input overflow ! */

    nb = *in & 0xff;
    nb <<= 8;
    nb |= *(in + 1) & 0xff;

    for(i = 0; i < 4; i += 1)
    {
        c = (nb & mask) >> (4 * (3 - i));
        *dest = c > 0x9 ? 'a' + c - 0xa : '0' + c;
        mask >>= 4;
        dest += 1;
    }
    return 4; /* always write 4 char */
}

static int ip_to_str(const char *ip, uint32_t ilen,
                     char *dest, uint32_t olen,
                     nb_to_str_func nb_to_str, char sep, int step)
{
    uint32_t i = 0, written = 0;
    char *guard = dest + olen;

    /* do this here so we don't start with a seperator */
    written = nb_to_str(ip, ilen, dest, guard);
    if(written == 0)
        { return 0; } /* overflow */
    dest += written;

    for (i = step; i < ilen; i += step)
    {
        if (dest >= guard) /* overflow */
           { return 0; }
        *dest = sep;
        dest += 1;
        written = nb_to_str(ip + i, ilen - i, dest, guard);
        if(written == 0)
           { return 0; }  /* overflow */
        dest += written;
    }
    *dest = '\0';
    return 1;
}

/* note: if c++, a great idea would be to implement that using traits */

udpr_endpoint *udpr_endpoint_create_ipv4(uint32_t addr, uint16_t port)
{
    udpr_endpoint *e = NULL;
    uint32_t i = 0, mask = 0xff000000;

    e = malloc(sizeof(udpr_endpoint));
    if(e == NULL)
        { printf("malloc failed, %s:%d", __FILE__, __LINE__); return NULL; }

    e->len = 4;
    e->to_str = ipv4_nb_to_str;
    e->port = port;
    e->sep = '.';
    e->nb_size = 1;

    for(i = 0; i < e->len; i += 1)
    {
        *(e->ip + i) = (addr & mask) >> (8 * (3 - i));
        mask >>= 8;
    }

    return e;
}

udpr_endpoint *udpr_endpoint_create_ipv6(char addr[16], uint16_t port)
{
    udpr_endpoint *e = NULL;
    uint32_t i = 0;

    e = malloc(sizeof(udpr_endpoint));
    if(e == NULL)
        { printf("malloc failed, %s:%d", __FILE__, __LINE__); return NULL; }

    e->len = 16;
    e->to_str = ipv6_nb_to_str;
    e->port = port;
    e->sep = ':';
    e->nb_size = 2;

    for(i = 0; i < e->len; i += 1)
    {
        *(e->ip + i) = addr[i];
    }
    return e;
}

void udpr_endpoint_release(udpr_endpoint **e)
{
    free(*e);
    *e = NULL;
}

int udpr_endpoint_get_ip_str(udpr_endpoint *e, char *dest, int len)
{
    return ip_to_str(e->ip, e->len, dest, len, e->to_str,
                     e->sep, e->nb_size);
}

uint16_t udpr_endpoint_port(udpr_endpoint *e)
{
    return e->port;
}
