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

class OpcuaDataBaseFloat : public OpcuaDataBase
{
	public:
	OpcuaDataBaseFloat(UA_Server* server, int rwflag, char* nodeName);
	~OpcuaDataBaseFloat() {}
	void UpdateFloat();
	void SetValue(float value) { m_data = value; }
	static void beforeReadCallback(UA_Server* server,
		const UA_NodeId* sessionId, void* sessionContext,
		const UA_NodeId* nodeid, void* nodeContext,
		const UA_NumericRange* range, const UA_DataValue* data) {
		OpcuaDataBaseFloat* ctx = (OpcuaDataBaseFloat*)nodeContext;
		ctx->UpdateFloat();
	}
	static void afterWriteCallback(UA_Server* server,
		const UA_NodeId* sessionId, void* sessionContext,
		const UA_NodeId* nodeId, void* nodeContext,
		const UA_NumericRange* range, const UA_DataValue* data) {
		OpcuaDataBaseFloat* ctx = (OpcuaDataBaseFloat*)nodeContext;
		if (!data->hasValue) {
			UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Client attempted to write without a value.");
			return;
		}
		if (data->value.type != &UA_TYPES[UA_TYPES_FLOAT]) {
			UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Client attempted to write a non-Float value. Type: %s", data->value.type->typeName);
			return;
		}
		ctx->m_data = *(float*)data->value.data;
	}
private:
	void AddVariableFloat(int rwflag);
	UA_Float m_data;
	char* m_nodeName;
};


class OpcuaDataBaseBoolean : public OpcuaDataBase
{
public:
	OpcuaDataBaseBoolean(UA_Server* server, int rwflag, char* nodeName);
	~OpcuaDataBaseBoolean() {}
	void UpdateBoolean();
	void SetValue(bool value) { m_data = value; }
	static void beforeReadCallback(UA_Server* server,
		const UA_NodeId* sessionId, void* sessionContext,
		const UA_NodeId* nodeid, void* nodeContext,
		const UA_NumericRange* range, const UA_DataValue* data) {
		OpcuaDataBaseBoolean* ctx = (OpcuaDataBaseBoolean*)nodeContext;
		ctx->UpdateBoolean();
	}
	static void afterWriteCallback(UA_Server* server,
		const UA_NodeId* sessionId, void* sessionContext,
		const UA_NodeId* nodeId, void* nodeContext,
		const UA_NumericRange* range, const UA_DataValue* data) {
		OpcuaDataBaseBoolean* ctx = (OpcuaDataBaseBoolean*)nodeContext;
		if (!data->hasValue) {
			UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Client attempted to write without a value.");
			return;
		}
		if (data->value.type != &UA_TYPES[UA_TYPES_BOOLEAN]) {
			UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Client attempted to write a non-Boolean value. Type: %s", data->value.type->typeName);
			return;
		}
		ctx->m_data = *(UA_Boolean*)data->value.data;
	}
private:
	void AddVariableBoolean(int rwflag);
	UA_Boolean m_data;
	char* m_nodeName;
};

class OpcuaDataBaseDateTime : public OpcuaDataBase
{
	public:
		OpcuaDataBaseDateTime(UA_Server* server, int rwflag, char* nodeName);
	~OpcuaDataBaseDateTime() {}
	void UpdateDateTime();
	void SetValue(UA_DateTime value) { m_data = value; }
	static void beforeReadCallback(UA_Server* server,
		const UA_NodeId* sessionId, void* sessionContext,
		const UA_NodeId* nodeid, void* nodeContext,
		const UA_NumericRange* range, const UA_DataValue* data) {
		OpcuaDataBaseDateTime* ctx = (OpcuaDataBaseDateTime*)nodeContext;
		ctx->UpdateDateTime();
	}
	static void afterWriteCallback(UA_Server* server,
		const UA_NodeId* sessionId, void* sessionContext,
		const UA_NodeId* nodeId, void* nodeContext,
		const UA_NumericRange* range, const UA_DataValue* data) {
		OpcuaDataBaseDateTime* ctx = (OpcuaDataBaseDateTime*)nodeContext;
		if (!data->hasValue) {
			UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Client attempted to write without a value.");
			return;
		}
		if (data->value.type != &UA_TYPES[UA_TYPES_DATETIME]) {
			UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Client attempted to write a non-DateTime value. Type: %s", data->value.type->typeName);
			return;
		}
		ctx->m_data = *(UA_DateTime*)data->value.data;
	}
private:
	void AddVariableDateTime(int rwflag);
	UA_DateTime m_data;
	char* m_nodeName;
};


class  OpcuaDataBaseString : public OpcuaDataBase
{
public:
	OpcuaDataBaseString(UA_Server* server, int rwflag, char* nodeName);
	~OpcuaDataBaseString() {}
	void UpdateString();
	void SetValue(const char* value) {
		if (m_data.data != 0 && m_data.length > 0)UA_String_clear(&m_data);
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
		
		UA_Variant* dataValue = (UA_Variant*)&data->value;
		UA_String* stringdata= (UA_String*)dataValue->data;
		if (stringdata->data == 0 || stringdata->length == 0) {
			//UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Client attempted to write a null String value.");
			return;
		}
		else {
			
			if (stringdata->data != ctx->m_data.data) {
				UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Client attempted to write a String value. len=%d,adr1=%d,adr2=%d", stringdata->length, stringdata->data, ctx->m_data.data);

				UA_String_clear(&ctx->m_data);
				UA_String_copy(stringdata, &ctx->m_data);
			}
			//UA_String_clear(&ctx->m_data);
			//UA_String_copy(stringdata, &ctx->m_data);
		}
		
		//ctx->m_data = *(UA_String*)data->value.data;
		//UA_String_copy((UA_String*)data->value.data, &ctx->m_data);
		//ctx->m_data = *(UA_String*)data->value.data;
	}
private:
	void AddVariableString(int rwflag);
	UA_String m_data;
	char* m_nodeName;
};

#endif