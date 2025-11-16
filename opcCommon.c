#include "open62541.h"
#include "opcCommon.h"


UA_NodeId dc_namespaceId;
void DcInit(UA_Server* server) {
	// 初始化代码（如果有需要）
    dc_namespaceId = UA_NODEID_STRING(1, "Forcon");
    UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
    oAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Forcon");

    UA_Server_addObjectNode(server, dc_namespaceId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),    // 挂载到 Objects 下
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),        // 组织关系
        UA_QUALIFIEDNAME(1, "mynamespace"),              // 名称
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),   // 类型
        oAttr, NULL, NULL);
}

void DcUpdateCurrentStateInt32(UA_Server* server, MyNodeInt32Context* ctx) {
    UA_Variant value;
    UA_Variant_setScalar(&value, &(ctx->data), &UA_TYPES[UA_TYPES_INT32]);
    char string_buf[128];
    sprintf_s(string_buf, sizeof(string_buf), "%s-callback", ctx->name);
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, string_buf);
    UA_Server_writeValue(server, currentNodeId, value);
}

void DcAddCurrentStateVariableInt32(UA_Server* server, MyNodeInt32Context* ctx,UA_ValueCallback* callback) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", ctx->name);
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    attr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_INT32);
    UA_Variant_setScalar(&attr.value, &(ctx->data), &UA_TYPES[UA_TYPES_INT32]);

	char string_buf[128];
    sprintf_s(string_buf, sizeof(string_buf),"%s-callback", ctx->name);
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, string_buf);


    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, string_buf);
    //UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    //UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);

    UA_NodeId parentNodeId = UA_NODEID_STRING(1, "Forcon"); // 挂到自定义对象
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);


    UA_Server_addVariableNode(server, currentNodeId, parentNodeId,
        parentReferenceNodeId, currentName,
        variableTypeNodeId, attr, NULL, NULL);

    //UA_ValueCallback callback;
    //callback.onRead = beforeReadState;
    //callback.onWrite = afterWriteState;
	if (callback != NULL) UA_Server_setVariableNode_valueCallback(server, currentNodeId, *callback);

    DcUpdateCurrentStateInt32(server,ctx);
}






void DcUpdateCurrentStateString(UA_Server* server, MyNodeStringContext* ctx) {
    UA_Variant value;
    /* 使用 setScalarCopy 以深度复制 UA_String，避免使用了无效或临时指针 */
    UA_Variant_setScalarCopy(&value, &(ctx->data), &UA_TYPES[UA_TYPES_STRING]);

    char string_buf[128];
    sprintf_s(string_buf, sizeof(string_buf), "%s-callback", ctx->name);
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, string_buf);
    UA_Server_writeValue(server, currentNodeId, value);
    UA_Variant_clear(&value); /* 释放 setScalarCopy 分配的内存，避*/
}

void DcAddCurrentStateVariableString(UA_Server* server, MyNodeStringContext* ctx, UA_ValueCallback* callback) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", ctx->name);
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    attr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_STRING);//UA_NODEID_NUMERIC(0, UA_NS0ID_INT32);U
    /* 使用 setScalarCopy 以深度复制 UA_String 到 attr.value */
    UA_Variant_setScalarCopy(&attr.value, &(ctx->data), &UA_TYPES[UA_TYPES_STRING]);


    char string_buf[128];
    sprintf_s(string_buf, sizeof(string_buf), "%s-callback", ctx->name);
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, string_buf);


    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, string_buf);
    //UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    //UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);

    UA_NodeId parentNodeId = UA_NODEID_STRING(1, "Forcon"); // 挂到自定义对象
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);


    UA_Server_addVariableNode(server, currentNodeId, parentNodeId,
        parentReferenceNodeId, currentName,
        variableTypeNodeId, attr, NULL, NULL);

    //UA_ValueCallback callback;
    //callback.onRead = beforeReadState;
    //callback.onWrite = afterWriteState;

    /* attr.value 已被深拷贝到服务器内部，释放临时副本以避免内存泄漏 */
    UA_Variant_clear(&attr.value);

    if (callback != NULL) UA_Server_setVariableNode_valueCallback(server, currentNodeId, *callback);

    DcUpdateCurrentStateString(server, ctx);
}
