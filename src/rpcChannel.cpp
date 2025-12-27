#include "rpcChannel.h"
#include "connectionPool.h"
#include "TcpClient.h"
using namespace miniRpc;
RpcChannel::RpcChannel()
{
    m_requestMap.clear();
    m_threadPool = std::make_shared<ThreadPool>();
    m_threadPool->start();
    m_client = std::make_shared<ZkClient>(m_threadPool);
    m_client->reConnect();
    m_connPool = std::make_shared<ConnectionPool>(m_threadPool,m_client);
    m_connPool->setMessageCallBack(std::bind(&RpcChannel::getResponse,this,std::placeholders::_1));
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
    std::string sendstr = std::to_string(m_requestId) + ":" + request;
    TcpConnectionPtr conn = client->connection();
    if(conn && conn->isConnected())
    {
        conn->send(sendstr);
        std::cout << "发送成功" << std::endl;
        m_requestMap.emplace(m_requestId,func);
        m_requestId++;
    }
}

void RpcChannel::getResponse(const std::string& response)
{
    int index = response.find(":");
    int64_t requestId = static_cast<int64_t>(atol(response.substr(0,index).c_str()));
    std::string body = response.substr(index+1);
    auto it = m_requestMap.find(requestId);
    if(it != m_requestMap.end())
    {
        it->second(body);
        m_requestMap.erase(it);
    }
}
