
#include <stdio.h>
#include "civetweb.h"

#include <windows.h>
#include <time.h>
extern "C" {
    #include "open62541.h"
    #include "opcCommon.h"
}
#include "OpcuaCtx/OpcuaDataBase.h"
#include "OpcuaCtx/OpcuaDataDeviceStatus.h"


UA_Server* gOpcuaServer;


//bool gDeviceIsOnline = true;

/* 监控 handle_ask 访问时间的全局变量和同步 */
static time_t gLastAskTime = 0;
static CRITICAL_SECTION gLastAskCs;
static HANDLE gMonitorThread = NULL;
static volatile BOOL gMonitorRunning = TRUE;



OpcuaDataBaseString* nodeVarProductName;
OpcuaDataDeviceStatus* nodeVarRunningStatus;
OpcuaDataBaseInt32* nodeVarCmd;
OpcuaDataBaseString* nodeVar_C_RequestProductChange;
OpcuaDataBaseString* nodeVar_S_ResponseProductChange;
OpcuaDataBaseDateTime* nodeVar_S_ProductionStartTime;
OpcuaDataBaseFloat* nodeVar_S_ProductionProgress;
OpcuaDataBaseString* nodeVar_C_RequestProductionCommand;



// 监控线程：检查 handle_ask 最后访问时间，若超过 15 秒则置 gDeviceIsOnline = false
static DWORD WINAPI MonitorAskThread(LPVOID lpParam) {
    (void)lpParam;
    while (gMonitorRunning) {
        EnterCriticalSection(&gLastAskCs);
        time_t last = gLastAskTime;
        LeaveCriticalSection(&gLastAskCs);

        time_t now = time(NULL);
        double diff = difftime(now, last);
        if (last != 0 && diff > 600.0) {
            /* 超过 15 秒未访问 */
			nodeVarRunningStatus->SetDeviceOnline(false);
        }
        Sleep(1000); /* 每秒检查一次 */
    }
    return 0;
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

/* 刷新在线监控的时间及状态 */
static void refresh_ask_time() {
    EnterCriticalSection(&gLastAskCs);
    gLastAskTime = time(NULL);
    nodeVarRunningStatus->SetDeviceOnline(true);
    LeaveCriticalSection(&gLastAskCs);
}


// ===================================== http ====================================
// 
// 

static int handle_ask(struct mg_connection* conn, void* cbdata) {
    const struct mg_request_info* req_info = mg_get_request_info(conn);
    if (strcmp(req_info->request_method, "GET") == 0) {
        /* 每次被访问都更新最后访问时间，并保证 device 标记为在线 */
        refresh_ask_time();

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
		// 读取 所有query 参数
        refresh_ask_time();

        if (req_info->query_string) {
            char value_buf[32] = { 0 };
            if (mg_get_var(req_info->query_string, strlen(req_info->query_string), "devicestatus", value_buf, sizeof(value_buf)) > 0) {
                //nodeVarRunningStatus->SetStatus(value_buf);
                nodeVarRunningStatus->SetValue(value_buf);
            }
            if (mg_get_var(req_info->query_string, strlen(req_info->query_string), "cmd", value_buf, sizeof(value_buf)) > 0) {
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

int main(void) {
    gOpcuaServer = UA_Server_new();

    DcInit(gOpcuaServer);

	nodeVarProductName = new OpcuaDataBaseString(gOpcuaServer, OpcuaDataBase::VAR_READ,(char*)"S-ProductName");
	nodeVarRunningStatus = new OpcuaDataDeviceStatus(gOpcuaServer, OpcuaDataBase::VAR_READ,(char*)"S-RunningStatus");
	nodeVar_C_RequestProductChange = new OpcuaDataBaseString(gOpcuaServer, OpcuaDataBase::VAR_READWRITE, (char*)"C-RequestProductChange");
	nodeVar_S_ResponseProductChange = new OpcuaDataBaseString(gOpcuaServer, OpcuaDataBase::VAR_READ, (char*)"S-ResponseProductChange");
	nodeVar_S_ProductionStartTime = new OpcuaDataBaseDateTime(gOpcuaServer, OpcuaDataBase::VAR_READ, (char*)"S-ProductionStartTime");
	nodeVar_S_ProductionProgress = new OpcuaDataBaseFloat(gOpcuaServer, OpcuaDataBase::VAR_READ, (char*)"S-ProductionProgress");
	nodeVar_C_RequestProductionCommand = new OpcuaDataBaseString(gOpcuaServer, OpcuaDataBase::VAR_READWRITE, (char*)"C-RequestProductionCommand");


	nodeVarCmd = new OpcuaDataBaseInt32(gOpcuaServer, OpcuaDataBase::VAR_READWRITE,(char*)"Cmd");


    nodeVarProductName->SetValue("group2");
	nodeVarRunningStatus->SetStatus("idle");
	nodeVar_C_RequestProductChange->SetValue("none");	
	nodeVar_S_ResponseProductChange->SetValue("none");
	nodeVar_S_ProductionStartTime->SetValue(UA_DateTime_now());
	nodeVar_S_ProductionProgress->SetValue(0.07f);
	nodeVar_C_RequestProductionCommand->SetValue("none");
	nodeVar_C_RequestProductionCommand->SetFilterFromCsv("none,start,stop,pause,resume");

    start_http_server();

    /* 启动检查 handle_ask 访问时间的监控线程 */
    start_ask_monitor();

    UA_Server_runUntilInterrupt(gOpcuaServer);

    /* 退出前清理监控线程 */
    stop_ask_monitor();

    UA_Server_delete(gOpcuaServer);
    return 0;
}
