#pragma once
#include <string>
#include <functional>
#include "threadpool.h"
#include "zkclient.h"
#include "TcpClient.h"
#include "connectionPool.h"
#include <unordered_map>
#include <functional>
#include <atomic>
#include "public.h"
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
namespace miniRpc
{
    class RpcChannel : public google::protobuf::RpcChannel
    {
    public:
        struct PendingCall{
            TcpConnectionPtr conn;
            int64_t requestId;
            google::protobuf::Closure* done;
        };
    private:
        std::shared_ptr<ConnectionPool> m_connPool;
        std::unordered_map<int, PendingCall> m_requestMap;

    public:
        RpcChannel();
        ~RpcChannel();
        virtual void CallMethod(const google::protobuf::MethodDescriptor *method,
                                google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                                google::protobuf::Message *response, google::protobuf::Closure *done);
        // void callMethodAsync(const std::string &serviceName,
        //                      const std::string &methodName, const std::string &request,
        //                      std::function<void(std::string)>);
        void getResponse(Buffer *buff);
    };
}
