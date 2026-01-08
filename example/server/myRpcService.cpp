#include "myRpcService.h"

MyRpcService::MyRpcService()
{
    
}

void MyRpcService::SayHello(google::protobuf::RpcController *controller, const ::miniRpc::HelloRequest *request, ::miniRpc::HelloResponse *response, ::google::protobuf::Closure *done)
{
    std::cout << "sayhello 被调用" << std::endl;
    std::cout << "收到请求消息" << request->name() << std::endl;
    std::string name = request->name();
    response->set_message(name + ",你好");
}

void MyRpcService::GetData(google::protobuf::RpcController *controller, const ::miniRpc::DataRequest *request, ::miniRpc::DataResponse *response, ::google::protobuf::Closure *done)
{
}
