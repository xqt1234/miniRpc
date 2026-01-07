#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include "zkclient.h"
#include "threadpool.h"
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
using RpcMethod = std::function<void(const std::string request, const std::string &response)>;
using RpcAsyncMethod = std::function<void(const std::string &request, std::function<void(std::string &response)> callback)>;
namespace miniRpc
{
    class RpcService : public google::protobuf::Service
    {
    private:
        std::string m_name;
        std::unordered_map<std::string, RpcMethod> m_methodMap;
        std::unordered_map<std::string, RpcAsyncMethod> m_ansyncMethodMap;
        friend class ProVider;
        google::protobuf::ServiceDescriptor* m_descriptor;
    public:
        RpcService();
        ~RpcService();

        virtual const google::protobuf::ServiceDescriptor *GetDescriptor();
        virtual void CallMethod(const google::protobuf::MethodDescriptor *method,
                                google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                                google::protobuf::Message *response, google::protobuf::Closure *done) = 0;
        virtual const google::protobuf::Message &GetRequestPrototype(
            const google::protobuf::MethodDescriptor *method) const = 0;
        virtual const google::protobuf::Message &GetResponsePrototype(
            const google::protobuf::MethodDescriptor *method) const = 0;
        const std::string &getname() const;
        void setServiceName(const std::string &name);
        void addMethod(const std::string &name, RpcMethod method);
        void addAsyncMethod(const std::string &name, RpcAsyncMethod method);
        bool CallMethod(const std::string &methodname, const std::string &request, std::string &response);
        bool CallAsyncMethod(const std::string &methodname, const std::string &request, std::function<void(std::string)> done);
    };
}
