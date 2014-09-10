#ifndef UDPR_ENDPOINT_H
#define UDPR_ENDPOINT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct udpr_endpoint udpr_endpoint;

udpr_endpoint *udpr_endpoint_create_ipv4(uint32_t, uint16_t);
udpr_endpoint *udpr_endpoint_create_ipv6(char[16], uint16_t);
void udpr_endpoint_release(udpr_endpoint **e);

int udpr_endpoint_get_ip_str(udpr_endpoint *e, char *dest, int len);
uint16_t udpr_endpoint_port(udpr_endpoint*);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UDPR_ENDPOINT_H */
