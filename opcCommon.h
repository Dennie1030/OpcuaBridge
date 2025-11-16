#ifndef OPCCOMMON_H
#define OPCCOMMON_H


#include "open62541.h"

typedef struct {
	const char* name;
    UA_Int32 data;
} MyNodeInt32Context;

typedef struct {
	const char* name;
	UA_String data;
} MyNodeStringContext;

void DcInit(UA_Server* server);

void DcUpdateCurrentStateInt32(UA_Server* server, MyNodeInt32Context* ctx);
void DcAddCurrentStateVariableInt32(UA_Server* server, MyNodeInt32Context* ctx, UA_ValueCallback* callback);

void DcUpdateCurrentStateString(UA_Server* server, MyNodeStringContext* ctx);
void DcAddCurrentStateVariableString(UA_Server* server, MyNodeStringContext* ctx, UA_ValueCallback* callback);

#endif // !OPCCOMMON_H




