#include "rpcChannel.h"
#include "connectionPool.h"
#include "TcpClient.h"
#include "rpcApplication.h"
#include "json.hpp"
#include <vector>
#include <string.h>
#include "buildproto.h"
using namespace miniRpc;
using json = nlohmann::json;
RpcChannel::RpcChannel()
{
    m_requestMap.clear();
    m_connPool = std::make_shared<ConnectionPool>();
    m_connPool->setMessageCallBack(std::bind(&RpcChannel::getResponse,this,std::placeholders::_1));
}

RpcChannel::~RpcChannel()
{
    m_requestMap.clear();
}

void RpcChannel::callMethodAsync(const std::string &serviceName, 
    const std::string &methodName, const std::string &request,
     std::function<void(std::string)> func)
{
    auto client = m_connPool->getConnection(serviceName);
    if(client == nullptr)
    {
        return;
    }
    json resjs{
        {"service",serviceName},
        {"method",methodName},
        {"data",request}
    };
    static std::atomic<int64_t> m_requestId = 0;
    int requestId = ++m_requestId;
    BuildProto::enCodeRequest(resjs.dump(),requestId ,[&](const std::string& req)
    {
        TcpConnectionPtr conn = client->connection();
        if(conn && conn->isConnected())
        {
            conn->sendWithoutProto(req);
            // std::cout << "发送成功" << req.size() << std::endl;
            m_requestMap.emplace(requestId,func);
        }
    });
}

void RpcChannel::getResponse(Buffer* buffer)
{
    BuildProto::deCodeResponse(buffer,[&](const std::string& response,int64_t requestId){
        // std::cout << "收到回复---" << response << std::endl;
        auto it = m_requestMap.find(requestId);
        if(it != m_requestMap.end())
        {
            it->second(response);
            m_requestMap.erase(it);
        }
    });
}
