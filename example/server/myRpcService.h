#pragma once
#include "loginservice.pb.h"

class MyRpcService : public miniRpc::MyRpcService
{
private:
    /* data */
public:
    MyRpcService(/* args */);
    ~MyRpcService() = default;
    virtual void SayHello(google::protobuf::RpcController *controller,
                          const ::miniRpc::HelloRequest *request,
                          ::miniRpc::HelloResponse *response,
                          ::google::protobuf::Closure *done);
    virtual void GetData(google::protobuf::RpcController *controller,
                         const ::miniRpc::DataRequest *request,
                         ::miniRpc::DataResponse *response,
                         ::google::protobuf::Closure *done);
};
