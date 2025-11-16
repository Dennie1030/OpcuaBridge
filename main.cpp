
#include <stdio.h>
#include "civetweb.h"

#include <windows.h>
#include <time.h>
extern "C" {
    #include "open62541.h"
    #include "opcCommon.h"
}
#include "OpcuaCtx/OpcuaDataBase.h"


UA_Server* gOpcuaServer;


bool gDeviceIsOnline = true;

/* 监控 handle_ask 访问时间的全局变量和同步 */
static time_t gLastAskTime = 0;
static CRITICAL_SECTION gLastAskCs;
static HANDLE gMonitorThread = NULL;
static volatile BOOL gMonitorRunning = TRUE;



OpcuaDataBaseString* nodeVarProductName;
OpcuaDataBaseString* nodeVarProductState;
OpcuaDataBaseInt32* nodeVarCmd;

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
            }else if (mg_get_var(req_info->query_string, strlen(req_info->query_string), "cmd", value_buf, sizeof(value_buf)) > 0) {
                int new_value = atoi(value_buf);
				nodeVarCmd->SetValue(new_value);
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
            "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
		
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

	nodeVarProductName = new OpcuaDataBaseString(gOpcuaServer, (char*)"ProductName");
	nodeVarProductState = new OpcuaDataBaseString(gOpcuaServer, (char*)"ProductState");
	nodeVarCmd = new OpcuaDataBaseInt32(gOpcuaServer, (char*)"Cmd");

    start_http_server();

    /* 启动检查 handle_ask 访问时间的监控线程 */
    start_ask_monitor();

    UA_Server_runUntilInterrupt(gOpcuaServer);

    /* 退出前清理监控线程 */
    stop_ask_monitor();

    UA_Server_delete(gOpcuaServer);
    return 0;
}
