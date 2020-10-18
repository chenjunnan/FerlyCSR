#pragma  once
#include <amqp.h>
#include <amqp_framing.h>
#include <stdint.h>

#include <windows.h>
#include "xCommon.h"
#define CHECK_MQRET(condition,line)if(!(condition)){success=false;LOGE("%s",(line));break;}

extern void die(const char *fmt, ...);

extern bool die_on_amqp_error(amqp_rpc_reply_t x, char const *context);

extern void amqp_dump(void const *buffer, size_t len);
extern void die_on_error(int x, char const *context);

extern uint64_t now_microseconds(void);
extern void microsleep(int usec);


