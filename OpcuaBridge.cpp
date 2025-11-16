#include <iostream>
#include <thread>
#include <string>
#include <map>
#include <mutex>
#include "open62541.h"

// CivetWeb（或 Mongoose）头文件
#include "civetweb.h"
#include "CivetServer.h"

std::mutex nodeMutex;
std::map<std::string, UA_NodeId> variableNodes;
UA_Server* server = nullptr;

// 添加一个整型变量节点
UA_NodeId addVariableNode(const std::string& name, int initialValue)
{
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_Int32 value = initialValue;
    UA_Variant_setScalar(&attr.value, &value, &UA_TYPES[UA_TYPES_INT32]);
    attr.description = UA_LOCALIZEDTEXT_ALLOC("zh-CN", name.c_str());
    attr.displayName = UA_LOCALIZEDTEXT_ALLOC("zh-CN", name.c_str());
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;

    UA_NodeId newNodeId = UA_NODEID_STRING_ALLOC(1, name.c_str());
    UA_QualifiedName newNodeName = UA_QUALIFIEDNAME_ALLOC(1, name.c_str());

    UA_Server_addVariableNode(server,
        newNodeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
        newNodeName,
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        attr,
        NULL, NULL);

    UA_QualifiedName_clear(&newNodeName);
    variableNodes[name] = newNodeId;
    return newNodeId;
}

// 设置 OPC UA 变量节点的值
bool setNodeValue(const std::string& name, int value)
{
    std::lock_guard<std::mutex> lock(nodeMutex);
    if (variableNodes.count(name) == 0) {
        return false;
    }
    UA_Variant newValue;
    UA_Variant_setScalar(&newValue, &value, &UA_TYPES[UA_TYPES_INT32]);
    UA_Server_writeValue(server, variableNodes[name], newValue);
    return true;
}

// HTTP回调处理
class UpdateHandler : public CivetHandler {
public:
    bool handlePost(CivetServer* srv, struct mg_connection* conn)
    {
        char buffer[1024];
        int len = mg_read(conn, buffer, sizeof(buffer));
        if (len <= 0) {
            mg_printf(conn,
                "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nBad Request");
            return true;
        }
        buffer[len] = 0;
        std::string reqBody(buffer);
        // 简单解析 body，提取 node/value（生产环境推荐用 JSON 库，这里直接处理）
        size_t nodePos = reqBody.find("\"node\":\"");
        size_t valuePos = reqBody.find("\"value\":");
        if (nodePos == std::string::npos || valuePos == std::string::npos) {
            mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\nMissing Params");
            return true;
        }
        nodePos += 8; // 跳过 "node":"
        size_t nodeEnd = reqBody.find("\"", nodePos);
        std::string nodeName = reqBody.substr(nodePos, nodeEnd - nodePos);

        valuePos += 8;
        size_t valueEnd = reqBody.find_first_of(",}\n\r", valuePos);
        std::string valueStr = reqBody.substr(valuePos, valueEnd - valuePos);
        int value = std::stoi(valueStr);

        bool updated = setNodeValue(nodeName, value);
        if (updated) {
            mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nUpdated %s = %d", nodeName.c_str(), value);
        }
        else {
            mg_printf(conn, "HTTP/1.1 404 Not Found\r\n\r\nNode Not Found");
        }
        return true;
    }
};

// OPC UA 服务器主线程
void opcua_thread()
{
    server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

    // 初始化几个变量节点
    addVariableNode("Var1", 100);
    addVariableNode("Var2", 200);

    // 修正：定义一个变量
    UA_Boolean running = UA_TRUE;
    UA_Server_run(server, &running);

    // 清理
    UA_Server_delete(server);
}

// HTTP 服务器主线程
void http_thread()
{
    const char* options[] = { "document_root", ".", "listening_ports", "8000", 0 };
    CivetServer server(options);
    UpdateHandler updateHandler;
    server.addHandler("/update", updateHandler);
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main()
{
    std::thread th1(opcua_thread);
    std::thread th2(http_thread);

    th1.join();
    th2.join();
    return 0;
}