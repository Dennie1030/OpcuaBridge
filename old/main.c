#include "open62541.h"
#include <stdio.h>
#include "civetweb.h"
#include "opcCommon.h"
#include <windows.h>
#include <time.h>


UA_Int32 gRunningState = 1;
UA_Int32 gCommandState = 0;
UA_Server* gOpcuaServer;


MyNodeInt32Context ctxState= { "device-state", 0 };
MyNodeInt32Context ctxCmd = { "cmd", 0 };

MyNodeStringContext ctx_product_name;
MyNodeStringContext ctx_device_work_mode;

bool gDeviceIsOnline = true;

/* 监控 handle_ask 访问时间的全局变量和同步 */
static time_t gLastAskTime = 0;
static CRITICAL_SECTION gLastAskCs;
static HANDLE gMonitorThread = NULL;
static volatile BOOL gMonitorRunning = TRUE;


/* 更新 OPC UA 变量的辅助函数 */
static void updateCurrentState(UA_Server* server) {
    UA_Variant value;
    UA_Variant_setScalar(&value, &gRunningState, &UA_TYPES[UA_TYPES_INT32]);
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "current-device-state-callback");
    UA_Server_writeValue(server, currentNodeId, value);
}


static void beforeReadState(UA_Server* server,
    const UA_NodeId* sessionId, void* sessionContext,
    const UA_NodeId* nodeid, void* nodeContext,
    const UA_NumericRange* range, const UA_DataValue* data) {
    updateCurrentState(server);
}

static void afterWriteState(UA_Server* server,
    const UA_NodeId* sessionId, void* sessionContext,
    const UA_NodeId* nodeId, void* nodeContext,
    const UA_NumericRange* range, const UA_DataValue* data) {
    // 检查客户端是否提供了值
    if (!data->hasValue) {
        UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Client attempted to write without a value.");
        return;
    }

    // 检查写入的值是否为 Int32 类型
    if (data->value.type != &UA_TYPES[UA_TYPES_INT32]) {
        UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Client attempted to write a non-Int32 value. Type: %s", data->value.type->typeName);
        return;
    }

    // 获取客户端尝试写入的值
    UA_Int32 clientValue = *(UA_Int32*)data->value.data;
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Client wrote value: %d", clientValue);
    //UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "SessionName: %s", sessionContext ? (char*)sessionContext : "N/A");

    // 更新全局状态变量
    gRunningState = clientValue;
}

static void beforeReadCmd(UA_Server* server,
    const UA_NodeId* sessionId, void* sessionContext,
    const UA_NodeId* nodeid, void* nodeContext,
    const UA_NumericRange* range, const UA_DataValue* data) {

    if (nodeContext == NULL) {
		MyNodeInt32Context* ctx = (MyNodeInt32Context*)nodeContext;
        DcUpdateCurrentStateInt32(server, &ctx);
        return;
	}
	
}

static void afterWriteCmd(UA_Server* server,
    const UA_NodeId* sessionId, void* sessionContext,
    const UA_NodeId* nodeId, void* nodeContext,
    const UA_NumericRange* range, const UA_DataValue* data) {
    // 检查客户端是否提供了值
    if (!data->hasValue) {
        UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Client attempted to write without a value.");
        return;
    }

    // 检查写入的值是否为 Int32 类型
    if (data->value.type != &UA_TYPES[UA_TYPES_INT32]) {
        UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Client attempted to write a non-Int32 value. Type: %s", data->value.type->typeName);
        return;
    }

    // 获取客户端尝试写入的值
    UA_Int32 clientValue = *(UA_Int32*)data->value.data;
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Client wrote value: %d", clientValue);
    //UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "SessionName: %s", sessionContext ? (char*)sessionContext : "N/A");

    // 更新全局状态变量
    gCommandState = clientValue;
}


static void addCurrentStateVariable(UA_Server* server) {    
	UA_VariableAttributes attr = UA_VariableAttributes_default;
	attr.displayName = UA_LOCALIZEDTEXT("en-US", "current-device-state");
	attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    attr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_INT32);
	UA_Variant_setScalar(&attr.value, &gRunningState, &UA_TYPES[UA_TYPES_INT32]);

	UA_NodeId currentNodeId = UA_NODEID_STRING(1, "current-device-state-callback");
    

	UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, "current-device-state-callback");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);


    UA_Server_addVariableNode(server, currentNodeId, parentNodeId,
        parentReferenceNodeId, currentName,
        variableTypeNodeId, attr, NULL, NULL);

    UA_ValueCallback callback;
    callback.onRead = beforeReadState;
    callback.onWrite = afterWriteState;
    UA_Server_setVariableNode_valueCallback(server, currentNodeId, callback);

    updateCurrentState(server);
}




// ===================================== http ====================================
// 
// 

static int handle_ask(struct mg_connection* conn, void* cbdata) {
    const struct mg_request_info* req_info = mg_get_request_info(conn);
    if (strcmp(req_info->request_method, "GET") == 0) {
        /* 每次被访问都更新最后访问时间，并保证 device 标记为在线 */
        EnterCriticalSection(&gLastAskCs);
        gLastAskTime = time(NULL);
        gDeviceIsOnline = true;
        LeaveCriticalSection(&gLastAskCs);

        mg_printf(conn,
            "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n%s",
            "alive");
        return 200;
    }
    else {
        mg_printf(conn, "HTTP/1.1 405 Method Not Allowed\r\n\r\n");
        return 405;
	}
}
// CivetWeb HTTP 请求处理（GET 返回状态，POST 设置状态）
static int handle_write(struct mg_connection* conn, void* cbdata) {
    const struct mg_request_info* req_info = mg_get_request_info(conn);

    if (strcmp(req_info->request_method, "GET") == 0) {
        mg_printf(conn,
            "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
        return 200;
    }
    else if (strcmp(req_info->request_method, "POST") == 0) {

        if (req_info->query_string) {
            char value_buf[32] = { 0 };
            if (mg_get_var(req_info->query_string, strlen(req_info->query_string), "value", value_buf, sizeof(value_buf)) > 0) {
                int new_value = atoi(value_buf);
                ctxState.data = new_value;
				DcUpdateCurrentStateInt32(gOpcuaServer, &ctxState);
            }else if (mg_get_var(req_info->query_string, strlen(req_info->query_string), "cmd", value_buf, sizeof(value_buf)) > 0) {
                int new_value = atoi(value_buf);
                ctxCmd.data = new_value;
				DcUpdateCurrentStateInt32(gOpcuaServer, &ctxCmd);
            }
            // 返回响应...
        }
        /*
        // 读取 body 或 query 参数
        char post_data[128] = { 0 };
        int data_len = mg_read(conn, post_data, sizeof(post_data) - 1);
        char* value_str = strstr(post_data, "value=");
        int new_value = 0;
        if (value_str) {
            new_value = atoi(value_str + strlen("value="));
            gRunningState = new_value;
        }*/

        mg_printf(conn,
            "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nSet gRunningState to %d\n",
            gRunningState);
		
        return 200;
    }
    else {
        mg_printf(conn, "HTTP/1.1 405 Method Not Allowed\r\n\r\n");
        return 405;
    }
}

// 监控线程：检查 handle_ask 最后访问时间，若超过 15 秒则置 gDeviceIsOnline = false
static DWORD WINAPI MonitorAskThread(LPVOID lpParam) {
    (void)lpParam;
    while (gMonitorRunning) {
        EnterCriticalSection(&gLastAskCs);
        time_t last = gLastAskTime;
        LeaveCriticalSection(&gLastAskCs);

        time_t now = time(NULL);
        double diff = difftime(now, last);
        if (last != 0 && diff > 15.0) {
            /* 超过 15 秒未访问 */
            gDeviceIsOnline = false;
        }
        Sleep(1000); /* 每秒检查一次 */
    }
    return 0;
}

// CivetWeb 启动
void start_http_server() {
    // 可配置端口等选项
    const char* options[] = {
        "document_root", ".",
        "listening_ports", "8080",
        0
    };

    // 创建 CivetWeb 服务
    struct mg_callbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    struct mg_context* ctx = mg_start(&callbacks, 0, options);

    // 注册 REST 路径
    mg_set_request_handler(ctx, "/write", handle_write, 0);
	mg_set_request_handler(ctx, "/ask", handle_ask, 0);

    printf("HTTP server running on http://127.0.0.1:8080/state\n");
    // 不要退出，需要保持 ctx 存在
    // 可以改成线程运行，也可以和主程序一起运行（单线程测试没问题）
}

/* 启动监控线程（初始化临界区并创建线程） */
static void start_ask_monitor() {
    InitializeCriticalSection(&gLastAskCs);
    /* 初始为当前时间，避免程序启动立即判为离线；如果你希望启动后等待第一次访问再开始倒计时，可设置为 0 */
    EnterCriticalSection(&gLastAskCs);
    gLastAskTime = time(NULL);
    LeaveCriticalSection(&gLastAskCs);

    gMonitorRunning = TRUE;
    gMonitorThread = CreateThread(NULL, 0, MonitorAskThread, NULL, 0, NULL);
}

/* 停止监控线程并释放资源 */
static void stop_ask_monitor() {
    if (gMonitorThread) {
        gMonitorRunning = FALSE;
        /* 等待线程结束（超时 5 秒） */
        WaitForSingleObject(gMonitorThread, 5000);
        CloseHandle(gMonitorThread);
        gMonitorThread = NULL;
    }
    DeleteCriticalSection(&gLastAskCs);
}


int main(void) {
    gOpcuaServer = UA_Server_new();
    DcInit(gOpcuaServer);

    //addCurrentStateVariable(gOpcuaServer);

	//command state
    UA_ValueCallback cmdCallback;
    cmdCallback.onRead = beforeReadCmd;
    cmdCallback.onWrite = afterWriteCmd;
    DcAddCurrentStateVariableInt32(gOpcuaServer, &ctxState, &cmdCallback);
	DcAddCurrentStateVariableInt32(gOpcuaServer, &ctxCmd, &cmdCallback);

    ctx_product_name.name = "product-name";
    ctx_product_name.data = UA_String_fromChars("DEF_701");
	DcAddCurrentStateVariableString(gOpcuaServer, &ctx_product_name, NULL);

	ctx_device_work_mode.name = "device-work-mode";
	ctx_device_work_mode.data = UA_String_fromChars("Offline");
    DcAddCurrentStateVariableString(gOpcuaServer, &ctx_device_work_mode, NULL);

    start_http_server();

    /* 启动检查 handle_ask 访问时间的监控线程 */
    start_ask_monitor();

    UA_Server_runUntilInterrupt(gOpcuaServer);

    /* 退出前清理监控线程 */
    stop_ask_monitor();

    UA_Server_delete(gOpcuaServer);
    return 0;
}
