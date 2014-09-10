#ifndef UDPR_FSM_EVENT_H
#define UDPR_FSM_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct udpr_fsm_event
{
    int id;
    void *arg1, *arg2;
}
udpr_fsm_event;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UDPR_FSM_EVENT_H */

