#include "OpcuaDataBase.h"
#include <string>
#include <vector>
#include <mutex>
#include <algorithm>
#include <cctype>
#include <sstream>

OpcuaDataBase::OpcuaDataBase(UA_Server* server)
{
	m_server = server;
}
OpcuaDataBase::~OpcuaDataBase()
{
}


OpcuaDataBaseInt32::OpcuaDataBaseInt32(UA_Server* server, int rwflag, char* nodeName)
	:OpcuaDataBase(server)
{
	m_nodeName = nodeName;

	AddVariableInt32(rwflag);
}

void OpcuaDataBaseInt32::UpdateInt32()
{
	UA_Variant value;
	UA_Variant_setScalar(&value, &m_data, &UA_TYPES[UA_TYPES_INT32]);
	char string_buf[128];
	sprintf_s(string_buf, sizeof(string_buf), "%s-callback", m_nodeName);
	UA_NodeId currentNodeId = UA_NODEID_STRING(1, string_buf);
	UA_Server_writeValue(m_server, currentNodeId, value);
}

void OpcuaDataBaseInt32::AddVariableInt32(int rwflag)
{

	UA_VariableAttributes attr = UA_VariableAttributes_default;
	attr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", m_nodeName);
	if (rwflag == VAR_READ)
		attr.accessLevel = UA_ACCESSLEVELMASK_READ;
	else if (rwflag == VAR_WRITE)
		attr.accessLevel = UA_ACCESSLEVELMASK_WRITE;
	else
		attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
	attr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_INT32);
	UA_Variant_setScalar(&attr.value, &m_data, &UA_TYPES[UA_TYPES_INT32]);
	char string_buf[128];
	sprintf_s(string_buf, sizeof(string_buf), "%s-callback", m_nodeName);
	UA_NodeId currentNodeId = UA_NODEID_STRING(1, string_buf);
	UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, string_buf);
	UA_NodeId parentNodeId = UA_NODEID_STRING(1, (char*)"Forcon"); // 挂到自定义对象
	UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
	UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
	UA_Server_addVariableNode(m_server, currentNodeId, parentNodeId,
		parentReferenceNodeId, currentName,
		variableTypeNodeId, attr, this, NULL);

	UA_ValueCallback callback;
	callback.onRead = beforeReadCallback;
	callback.onWrite = afterWriteCallback;
	UA_Server_setVariableNode_valueCallback(m_server, currentNodeId, callback);
}

// =============================================================== OpcuaDataBaseFloat =======================================================================
OpcuaDataBaseFloat::OpcuaDataBaseFloat(UA_Server* server, int rwflag, char* nodeName)
	:OpcuaDataBase(server)
{
	m_nodeName = nodeName;
	AddVariableFloat(rwflag);
}

void OpcuaDataBaseFloat::UpdateFloat()
{
	UA_Variant value;
	UA_Variant_setScalar(&value, &m_data, &UA_TYPES[UA_TYPES_FLOAT]);
	char string_buf[128];
	sprintf_s(string_buf, sizeof(string_buf), "%s-callback", m_nodeName);
	UA_NodeId currentNodeId = UA_NODEID_STRING(1, string_buf);
	UA_Server_writeValue(m_server, currentNodeId, value);
}

void OpcuaDataBaseFloat::AddVariableFloat(int rwflag)
{
	UA_VariableAttributes attr = UA_VariableAttributes_default;
	attr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", m_nodeName);
	if (rwflag == VAR_READ)
		attr.accessLevel = UA_ACCESSLEVELMASK_READ;
	else if (rwflag == VAR_WRITE)
		attr.accessLevel = UA_ACCESSLEVELMASK_WRITE;
	else
		attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
	attr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_FLOAT);
	UA_Variant_setScalar(&attr.value, &m_data, &UA_TYPES[UA_TYPES_FLOAT]);
	char string_buf[128];
	sprintf_s(string_buf, sizeof(string_buf), "%s-callback", m_nodeName);
	UA_NodeId currentNodeId = UA_NODEID_STRING(1, string_buf);
	UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, string_buf);
	UA_NodeId parentNodeId = UA_NODEID_STRING(1, (char*)"Forcon"); // 挂到自定义对象
	UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
	UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
	UA_Server_addVariableNode(m_server, currentNodeId, parentNodeId,
		parentReferenceNodeId, currentName,
		variableTypeNodeId, attr, this, NULL);
	UA_ValueCallback callback;
	callback.onRead = beforeReadCallback;
	callback.onWrite = afterWriteCallback;
	UA_Server_setVariableNode_valueCallback(m_server, currentNodeId, callback);
}

// =============================================================== OpcuaDataBaseBoolean =======================================================================
OpcuaDataBaseBoolean::OpcuaDataBaseBoolean(UA_Server* server, int rwflag, char* nodeName)
	:OpcuaDataBase(server)
{
	m_nodeName = nodeName;
	AddVariableBoolean(rwflag);
}
void OpcuaDataBaseBoolean::UpdateBoolean()
{
	UA_Variant value;
	UA_Variant_setScalar(&value, &m_data, &UA_TYPES[UA_TYPES_BOOLEAN]);
	char string_buf[128];
	sprintf_s(string_buf, sizeof(string_buf), "%s-callback", m_nodeName);
	UA_NodeId currentNodeId = UA_NODEID_STRING(1, string_buf);
	UA_Server_writeValue(m_server, currentNodeId, value);
}
void OpcuaDataBaseBoolean::AddVariableBoolean(int rwflag)
{
	UA_VariableAttributes attr = UA_VariableAttributes_default;
	attr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", m_nodeName);
	if (rwflag == VAR_READ)
		attr.accessLevel = UA_ACCESSLEVELMASK_READ;
	else if (rwflag == VAR_WRITE)
		attr.accessLevel = UA_ACCESSLEVELMASK_WRITE;
	else
		attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
	attr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_BOOLEAN);
	UA_Variant_setScalar(&attr.value, &m_data, &UA_TYPES[UA_TYPES_BOOLEAN]);
	char string_buf[128];
	sprintf_s(string_buf, sizeof(string_buf), "%s-callback", m_nodeName);
	UA_NodeId currentNodeId = UA_NODEID_STRING(1, string_buf);
	UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, string_buf);
	UA_NodeId parentNodeId = UA_NODEID_STRING(1, (char*)"Forcon"); // 挂到自定义对象
	UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
	UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
	UA_Server_addVariableNode(m_server, currentNodeId, parentNodeId,
		parentReferenceNodeId, currentName,
		variableTypeNodeId, attr, this, NULL);
	UA_ValueCallback callback;
	callback.onRead = beforeReadCallback;
	callback.onWrite = afterWriteCallback;
	UA_Server_setVariableNode_valueCallback(m_server, currentNodeId, callback);
}

// ================================================================ OpcuaDataDateTime =====================================================================
OpcuaDataBaseDateTime::OpcuaDataBaseDateTime(UA_Server* server, int rwflag, char* nodeName)
	:OpcuaDataBase(server)
{
	m_nodeName = nodeName;
	AddVariableDateTime(rwflag);
}
void OpcuaDataBaseDateTime::UpdateDateTime()
{
	UA_Variant value;
	UA_Variant_setScalar(&value, &m_data, &UA_TYPES[UA_TYPES_DATETIME]);
	char string_buf[128];
	sprintf_s(string_buf, sizeof(string_buf), "%s-callback", m_nodeName);
	UA_NodeId currentNodeId = UA_NODEID_STRING(1, string_buf);
	UA_Server_writeValue(m_server, currentNodeId, value);
}
void OpcuaDataBaseDateTime::AddVariableDateTime(int rwflag)
{
	UA_VariableAttributes attr = UA_VariableAttributes_default;
	attr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", m_nodeName);
	if (rwflag == VAR_READ)
		attr.accessLevel = UA_ACCESSLEVELMASK_READ;
	else if (rwflag == VAR_WRITE)
		attr.accessLevel = UA_ACCESSLEVELMASK_WRITE;
	else
		attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
	attr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_DATETIME);
	UA_Variant_setScalar(&attr.value, &m_data, &UA_TYPES[UA_TYPES_DATETIME]);
	char string_buf[128];
	sprintf_s(string_buf, sizeof(string_buf), "%s-callback", m_nodeName);
	UA_NodeId currentNodeId = UA_NODEID_STRING(1, string_buf);
	UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, string_buf);
	UA_NodeId parentNodeId = UA_NODEID_STRING(1, (char*)"Forcon"); // 挂到自定义对象
	UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
	UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
	UA_Server_addVariableNode(m_server, currentNodeId, parentNodeId,
		parentReferenceNodeId, currentName,
		variableTypeNodeId, attr, this, NULL);
	UA_ValueCallback callback;
	callback.onRead = beforeReadCallback;
	callback.onWrite = afterWriteCallback;
	UA_Server_setVariableNode_valueCallback(m_server, currentNodeId, callback);
}




// =============================================================== OpcuaDataBaseString =======================================================================
// 静态成员定义


void OpcuaDataBaseString::ClearFilter() {
	std::lock_guard<std::mutex> lk(s_filterMutex);
	s_filterWords.clear();
}

bool OpcuaDataBaseString::IsWordAllowed(const std::string& word) {
	std::lock_guard<std::mutex> lk(s_filterMutex);
	if (s_filterWords.empty()) {
		// 若过滤表为空，默认拒绝（可按需改为允许所有）
		return true;
	}
	for (const auto& w : s_filterWords) {
		if (w == word) return true;
	}
	return false;
}
static inline std::string trim_copy(const std::string& s) {
	size_t b = 0;
	size_t e = s.size();
	while (b < e && std::isspace((unsigned char)s[b])) ++b;
	while (e > b && std::isspace((unsigned char)s[e - 1])) --e;
	return s.substr(b, e - b);
}

void OpcuaDataBaseString::SetFilterFromCsv(const char* csv) {
	std::vector<std::string> tmp;
	if (csv) {
		std::string s(csv);
		std::istringstream ss(s);
		std::string item;
		while (std::getline(ss, item, ',')) {
			std::string t = trim_copy(item);
			if (!t.empty()) tmp.push_back(t);
		}
	}
	std::lock_guard<std::mutex> lk(s_filterMutex);
	s_filterWords.swap(tmp);
}

OpcuaDataBaseString::OpcuaDataBaseString(UA_Server* server, int rwflag, char* nodeName)
	:OpcuaDataBase(server)
{
	m_nodeName = nodeName;
	UA_String_init(&m_data);
	//m_data = UA_String_fromChars("none");
	AddVariableString(rwflag);
}

void OpcuaDataBaseString::AddVariableString(int rwflag)
{
	UA_VariableAttributes attr = UA_VariableAttributes_default;
	attr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", m_nodeName);
	if (rwflag == VAR_READ)
		attr.accessLevel = UA_ACCESSLEVELMASK_READ;
	else if (rwflag == VAR_WRITE)
		attr.accessLevel = UA_ACCESSLEVELMASK_WRITE;
	else
		attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
	attr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_STRING);
	UA_Variant_setScalar(&attr.value, &m_data, &UA_TYPES[UA_TYPES_STRING]);
	char string_buf[128];
	sprintf_s(string_buf, sizeof(string_buf), "%s-callback", m_nodeName);
	UA_NodeId currentNodeId = UA_NODEID_STRING(1, string_buf);
	UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, string_buf);
	UA_NodeId parentNodeId = UA_NODEID_STRING(1, (char*)"Forcon"); // 挂到自定义对象
	UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
	UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
	UA_Server_addVariableNode(m_server, currentNodeId, parentNodeId,
		parentReferenceNodeId, currentName,
		variableTypeNodeId, attr, this, NULL);
	UA_ValueCallback callback;
	callback.onRead = beforeReadCallback;
	callback.onWrite = afterWriteCallback;
	UA_Server_setVariableNode_valueCallback(m_server, currentNodeId, callback);
}

void OpcuaDataBaseString::UpdateString()
{
	UA_Variant value;
	UA_Variant_setScalar(&value, &m_data, &UA_TYPES[UA_TYPES_STRING]);
	char string_buf[128];
	sprintf_s(string_buf, sizeof(string_buf), "%s-callback", m_nodeName);
	UA_NodeId currentNodeId = UA_NODEID_STRING(1, string_buf);
	//UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "UpdateString A:len=%d, str= %s", m_data.length, (char*)m_data.data);

	UA_Server_writeValue(m_server, currentNodeId, value);

	//UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "UpdateString B:len=%d, str= %s", m_data.length,(char*)m_data.data);
}