#include "rpcChannel.h"
#include "connectionPool.h"
#include "TcpClient.h"
#include "rpcApplication.h"
#include "json.hpp"
#include <vector>
#include <string.h>
using namespace miniRpc;
using json = nlohmann::json;
RpcChannel::RpcChannel()
{
    m_requestMap.clear();
    m_connPool = std::make_shared<ConnectionPool>();
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
    json resjs{
        {"service",serviceName},
        {"method",methodName},
        {"data",request}
    };
    std::string sendstr = resjs.dump();
    // 总长度 魔数+ 数据长度 + 消息id
    int len = sizeof(int32_t) + sizeof(int32_t) + sizeof(int64_t) + sendstr.length();
    int totallen = sizeof(RpcMsgHeader) + sendstr.length();
    std::vector<char> sendvec(totallen);
    RpcMsgHeader* header = reinterpret_cast<RpcMsgHeader*>(sendvec.data());
    header->datalength = htonl(sendstr.length());
    header->magic = htonl(kMagicNumber);
    header->reqId = htobe64(++m_requestId);
    memcpy(sendvec.data() + sizeof(RpcMsgHeader),sendstr.data(),sendstr.length());
    TcpConnectionPtr conn = client->connection();
    if(conn && conn->isConnected())
    {
        conn->send(std::string(sendvec.data(),sendvec.size()));
        std::cout << "发送成功" << sendvec.size() << std::endl;
        m_requestMap.emplace(m_requestId,func);
    }
}

void RpcChannel::getResponse(const std::string& response)
{
    std::cout << "收到回复" << std::endl;
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
