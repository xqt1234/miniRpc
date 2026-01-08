#include "provider.h"
#include "EventLoop.h"
#include "Logger.h"
#include "rpcApplication.h"
#include <string.h>
#include "public.h"
#include "buildproto.h"
#include "rpcheader.pb.h"
#include <functional>
using namespace mymuduo;
using namespace miniRpc;
miniRpc::ProVider::ProVider()
{
    RpcConfig &conf = RpcApplication::getRpcConfig();
    m_rpcIp = conf.getValue("rpcserverip");
    m_rpcPort = atoi(conf.getValue("rpcserverport").c_str());
    // std::cout << "rpcip:" << m_rpcIp << " port:" << m_rpcPort << std::endl;
    start();
}

ProVider::~ProVider()
{
    m_serviceMap.clear();
    if (m_loop)
    {
        m_loop->quit();
    }
}

void ProVider::start()
{
    m_rootLoc = "/services";
    bool res = RpcApplication::getZkClient().createNode(m_rootLoc, "", ZOO_PERSISTENT);
    if (!res)
    {
        LOG_FATAL("zk根路径创建失败");
    }
    m_loop = std::make_unique<EventLoop>();
    m_server = std::make_unique<TcpServer>(m_loop.get(), m_rpcPort, m_rpcIp);
    m_server->setThreadNum(3);
    m_server->setMessageCallBack(std::bind(&ProVider::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    m_server->setConnectionCallBack(std::bind(&ProVider::onConnection, this, std::placeholders::_1));
    m_server->start();
    RpcApplication::getThreadPool().addTask([&]
                                            { m_loop->loop(); });
}

void ProVider::AddService(google::protobuf::Service* service)
{
    std::string servicename = service->GetDescriptor()->name();
    m_serviceMap.insert({servicename,service});
    std::string loc = m_rootLoc + "/" + servicename;
    ZkClient &zk = RpcApplication::getZkClient();
    bool res = zk.createNode(loc, "", ZOO_PERSISTENT);
    std::string ip = m_rpcIp + ":" + std::to_string(m_rpcPort);
    std::string host = loc + "/" + ip;
    if (res)
    {
        std::vector<std::string> nodes = zk.getNodeChildren(m_rootLoc);
        res = zk.createNode(host, "", ZOO_EPHEMERAL);
        if (!res)
        {
            std::cout << "创建节点失败" << std::endl;
        }
        else
        {
            std::cout << "创建节点成功" << host << std::endl;
        }
    }
}

void ProVider::onMessage(const TcpConnectionPtr &conn, Buffer *buffer)
{
    BuildProto::deCodeResponse(buffer,[&](const std::string &request,int64_t requestId){
        this->processReq(conn, request, requestId);
    });
}
void miniRpc::ProVider::processReq(const TcpConnectionPtr &conn, const std::string &req,int64_t requestId)
{
    std::cout << "服务端收到消息，长度是:" << req.length() << std::endl;
    RpcHeader header;
    if(!header.ParseFromString(req))
    {
        std::cout << "解析 RPC 头失败" << std::endl;
        return;
    }
    std::string servicename = header.servicename();
    std::string methodname = header.methodname();
    std::string reqData = header.reqdata();
    
    auto it = m_serviceMap.find(servicename);
    if(it == m_serviceMap.end())
    {
        std::cout << "找不到服务" << servicename << std::endl;
        return;
    }
    std::cout << "服务名称:" << servicename << " 函数名称:" << methodname << std::endl;
    google::protobuf::Service* tservice = it->second;
    const google::protobuf::MethodDescriptor* method = tservice->GetDescriptor()->FindMethodByName(methodname);
    google::protobuf::Message* request = tservice->GetRequestPrototype(method).New();
    request->ParseFromString(reqData);
    google::protobuf::Message* response = tservice->GetResponsePrototype(method).New();
    // google::protobuf::Closure* callback = google::protobuf::NewCallback(this, &ProVider::handSend,{conn,requestId,response,request});
    
    tservice->CallMethod(method,nullptr,request,response,nullptr);
    std::string result;
    response->SerializeToString(&result);
    BuildProto::enCodeRequest(result,requestId,[&](std::string str){
                conn->sendWithoutProto(str);
                delete request;
                delete response;
            });
}

void miniRpc::ProVider::handSend(const ConnectionInfo& info)
{
    std::string result;
    info.response->SerializeToString(&result);
    delete info.response;
    delete info.request;
    std::cout << "调用完毕:" << result << std::endl;
    BuildProto::enCodeRequest(result,info.requestId,[&](std::string str){
                info.conn->sendWithoutProto(str);
            });
}

void ProVider::onConnection(const TcpConnectionPtr &conn)
{
    if (!conn->isConnected())
    {
        std::cout << "连接断开" << std::endl;
        // conn->shutdown();
    }
    else
    {
        std::cout << "有客户端连接" << std::endl;
    }
}
