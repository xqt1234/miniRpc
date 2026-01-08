#include "rpcChannel.h"
#include "connectionPool.h"
#include "TcpClient.h"
#include "rpcApplication.h"
#include <vector>
#include <string.h>
#include "buildproto.h"
#include "rpcheader.pb.h"
using namespace miniRpc;
RpcChannel::RpcChannel()
{
    m_requestMap.clear();
    m_connPool = std::make_shared<ConnectionPool>();
    m_connPool->setMessageCallBack(std::bind(&RpcChannel::getResponse, this, std::placeholders::_1));
}

RpcChannel::~RpcChannel()
{
    m_requestMap.clear();
}

void miniRpc::RpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                                     google::protobuf::RpcController *controller,
                                     const google::protobuf::Message *request,
                                     google::protobuf::Message *response,
                                     google::protobuf::Closure *done)
{

    const google::protobuf::ServiceDescriptor *serviced = method->service();
    auto client = m_connPool->getConnection(serviced->name());
    if (client == nullptr)
    {
        return;
    }
    TcpConnectionPtr conn = client->connection();
    static std::atomic<int64_t> m_requestId = 0;
    int requestId = ++m_requestId;
    PendingCall pending{conn,requestId,done,response};
    m_requestMap[requestId] = pending;
    std::string servicename = method->service()->name();
    std::string methodname = method->name();
    std::string reqstring;
    miniRpc::RpcHeader header;
    header.set_methodname(methodname);
    header.set_servicename(servicename);
    std::string requestData;
    request->SerializeToString(&requestData);
    header.set_reqdata(requestData);
    std::string sendData;
    header.SerializeToString(&sendData);
    std::cout << "发送数据长度:" << sendData.length() << std::endl;
    BuildProto::enCodeRequest(sendData,requestId ,[&](const std::string& req)
    {
        if(conn && conn->isConnected())
        {
            conn->sendWithoutProto(req);
        }
    });
}


void RpcChannel::getResponse(Buffer *buffer)
{
    BuildProto::deCodeResponse(buffer, [&](const std::string &response, int64_t requestId)
                               {
        auto it = m_requestMap.find(requestId);
        if(it != m_requestMap.end())
        {
            it->second.msg->ParseFromString(response);
            it->second.done->Run();
            m_requestMap.erase(it);
        } 
    });
}
