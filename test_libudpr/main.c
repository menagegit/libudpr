#include "udpr.h"

#include "test_udpr.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint8_t h1_to_h2[32];
uint8_t h2_to_h1[32];
int rindex12 = 0, windex12 = 0;
int rindex21 = 0, windex21 = 0;
udpr_handler h1, h2;

extern mock_net_layer *nets[];

void packet_h1(struct udpr_handler*, udpr_packet*, udpr_endpoint*);
void oos_h1(struct udpr_handler*, udpr_endpoint*);

void packet_h2(struct udpr_handler*, udpr_packet*, udpr_endpoint*);
void oos_h2(struct udpr_handler*, udpr_endpoint*);

void packet_h1(struct udpr_handler *h, udpr_packet *p, udpr_endpoint *e)
{
    uint8_t b = 0;

    udpr_packet_read(p, &b, 1);
    //printf("received (%d)\n", b);
    if(h2_to_h1[rindex21++ % 32] != b)
        { printf("2 -> 1 mismatch ! have fun debugging this shit !\n"); }
/*    else
        { printf("cool one match\n"); }*/
}

void oos_h1(struct udpr_handler *h, udpr_endpoint *e)
{
    printf("wtf oos 2 -> 1\n");
}

void packet_h2(struct udpr_handler *h, udpr_packet *p, udpr_endpoint *e)
{
    uint8_t b = 0;

    udpr_packet_read(p, &b, 1);
//    printf("received (%d)\n", b);
    if(h1_to_h2[rindex12++ % 32] != b)
        { printf("1 -> 2 mismatch ! have fun debugging this shit !\n"); }
  //  else
    //    { printf("cool one match\n"); }
}

void oos_h2(struct udpr_handler *h, udpr_endpoint *e)
{
    printf("wtf oos 1 -> 2\n");
}

void send_to_h1(udpr_packet *data)
{
    static int rdom = 1;

    h2_to_h1[windex21++ % 32] = udpr_packet_first(data);
    if(rdom ++ % 10 != 0)
    {
        nets[0]->handler(nets[0]->handler_data, data, NULL);
    }
}

void send_to_h2(udpr_packet *data)
{
    static int rdom = 1;

    h1_to_h2[windex12++ % 32] = udpr_packet_first(data);
    if(rdom ++ % 10 != 0)
    {
        nets[1]->handler(nets[1]->handler_data, data, NULL);
    }
}

/* two handlers. when h1 send packet, it add it to h1_to_h2 packet list
 * when h2 receives a packet, it compares it to the first one in h1_to_h2 packet
 * idem for h2 to h1 */

int main()
{
    uint8_t b = 0;

    udpr_init(&h1);
    udpr_init(&h2);
    udpr_packet *p = NULL;

    udpr_register_callbacks(&h1, packet_h1, oos_h1);
    udpr_register_callbacks(&h2, packet_h2, oos_h2);

    /* ce serais cool de faire une api vraiment propre pce que la ca part un peu en couille
     * entre ce qui est interne et public */
    while(b <= 200)
    {
        p = udpr_packet_allocate();
        udpr_packet_write(p, &b, 1);
        udpr_send(&h1, p, NULL);
        //udpr_packet_release(&p);
        b += 1;
    }
    printf("done\n");

    return 0;
}

