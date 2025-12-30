#include "provider.h"
#include "EventLoop.h"
#include "Logger.h"
#include "rpcApplication.h"
#include <string.h>
#include "json.hpp"
#include "public.h"
#include "buildproto.h"
using namespace mymuduo;
using namespace miniRpc;
using json = nlohmann::json;
miniRpc::ProVider::ProVider()
{
    RpcConfig &conf = RpcApplication::getRpcConfig();
    m_rpcIp = conf.getValue("rpcserverip");
    m_rpcPort = atoi(conf.getValue("rpcserverport").c_str());
    std::cout << "rpcip:" << m_rpcIp << " port:" << m_rpcPort << std::endl;
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

void ProVider::AddService(std::shared_ptr<RpcService> service)
{
    m_serviceMap[service->m_name] = service;
    std::cout << "----节点名称:" << service->m_name << std::endl;
    std::string loc = m_rootLoc + "/" + service->m_name;
    ZkClient &zk = RpcApplication::getZkClient();
    bool res = zk.createNode(loc, "", ZOO_PERSISTENT);
    std::string ip = m_rpcIp + ":" + std::to_string(m_rpcPort);
    std::string host = loc + "/" + ip;
    if (res)
    {
        std::vector<std::string> nodes = zk.getNodeChildren(m_rootLoc);
        std::cout << "节点个数为:" << nodes.size() << std::endl;
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
    std::cout << "onmessage收到消息,准备解析" << std::endl;
    const char *data = buffer->peek();
    int len = buffer->readableBytes();
    std::cout << "收到长度" << len << std::endl;
    BuildProto::deCodeResponse(buffer,[&](const std::string &request,int64_t requestId){
        this->processReq(conn, request, requestId);
    });
}
void miniRpc::ProVider::processReq(const TcpConnectionPtr &conn, const std::string &req,int64_t requestId)
{
    std::cout << "收到消息:" << req << std::endl;
    if(req.size() == 0)
    {
        return;
    }
    json js = json::parse(req);
    if(!js.contains("service") || !js.contains("method") || !js.contains("data"))
    {
        return;
    }
    std::string sevicename = js["service"];
    std::string methodname = js["method"];
    std::string reqData = js["data"];
    auto it = m_serviceMap.find(sevicename);
    if (it == m_serviceMap.end())
    {
        return;
    }
    auto method = it->second->CallAsyncMethod(methodname, reqData, [&](std::string response){ 
            BuildProto::enCodeRequest(response,requestId,[&](std::string str){
                conn->send(str); 
            });
            
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
