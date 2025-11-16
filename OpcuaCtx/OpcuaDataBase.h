#ifndef OPCUDATABASE_H
#define OPCUDATABASE_H
#include "../open62541.h"

class OpcuaDataBase
{
public:
	static const int VAR_READ = 1;
	static const int VAR_WRITE = 2;
	static const int VAR_READWRITE = VAR_READ | VAR_WRITE;
public:
	OpcuaDataBase(UA_Server* server);
	~OpcuaDataBase();

protected:
	virtual void BeforeRead(void* ctx) {};
	virtual void AfterWrite(void* ctx) {};
	virtual void UpdateCurrentVariable() {};
	virtual void AddCVariable() {};
	UA_Server* m_server;

	UA_NodeId m_nodeId;
};
class OpcuaDataBaseInt32 : public OpcuaDataBase
{	
public:
	OpcuaDataBaseInt32(UA_Server* server, int rwflag, char* nodeName);
	~OpcuaDataBaseInt32() {}	
	void UpdateInt32();
	void SetValue(int value) { m_data = value; }

	static void beforeReadCallback(UA_Server* server,
		const UA_NodeId* sessionId, void* sessionContext,
		const UA_NodeId* nodeid, void* nodeContext,
		const UA_NumericRange* range, const UA_DataValue* data) {
		OpcuaDataBaseInt32* ctx = (OpcuaDataBaseInt32*)nodeContext;
		ctx->UpdateInt32();
	}

	static void afterWriteCallback(UA_Server* server,
		const UA_NodeId* sessionId, void* sessionContext,
		const UA_NodeId* nodeId, void* nodeContext,
		const UA_NumericRange* range, const UA_DataValue* data) {
		OpcuaDataBaseInt32* ctx = (OpcuaDataBaseInt32*)nodeContext;
		if (!data->hasValue) {
			UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Client attempted to write without a value.");
			return;
		}
		if (data->value.type != &UA_TYPES[UA_TYPES_INT32]) {
			UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Client attempted to write a non-Int32 value. Type: %s", data->value.type->typeName);
			return;
		}
		ctx->m_data = *(UA_Int32*)data->value.data;
	}
private:
	void AddVariableInt32(int rwflag);
	

	UA_Int32 m_data;
	char* m_nodeName;
	
};

class  OpcuaDataBaseString : public OpcuaDataBase
{
public:
	OpcuaDataBaseString(UA_Server* server, int rwflag, char* nodeName);
	~OpcuaDataBaseString() {}
	void UpdateString();
	void SetValue(const char* value) {
		UA_String_clear(&m_data);
		UA_String tmp = UA_String_fromChars(value);
		UA_String_copy(&tmp, &m_data);
	}

	static void beforeReadCallback(UA_Server* server,
		const UA_NodeId* sessionId, void* sessionContext,
		const UA_NodeId* nodeid, void* nodeContext,
		const UA_NumericRange* range, const UA_DataValue* data) {
		OpcuaDataBaseString* ctx = (OpcuaDataBaseString*)nodeContext;
		ctx->UpdateString();
	}

	static void afterWriteCallback(UA_Server* server,
		const UA_NodeId* sessionId, void* sessionContext,
		const UA_NodeId* nodeId, void* nodeContext,
		const UA_NumericRange* range, const UA_DataValue* data) {
		OpcuaDataBaseString* ctx = (OpcuaDataBaseString*)nodeContext;
		if (!data->hasValue) {
			UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Client attempted to write without a value.");
			return;
		}
		if (data->value.type != &UA_TYPES[UA_TYPES_STRING]) {
			UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Client attempted to write a non-String value. Type: %s", data->value.type->typeName);
			return;
		}
		UA_String_clear(&ctx->m_data);
		UA_String_copy((UA_String*)data->value.data, &ctx->m_data);
		//ctx->m_data = *(UA_String*)data->value.data;
	}
private:
	void AddVariableString(int rwflag);
	UA_String m_data;
	char* m_nodeName;
};

#endif