#include <iostream>
#include "rpcChannel.h"
#include "rpcApplication.h"
#include "loginservice.pb.h"
#include <google/protobuf/stubs/callback.h>
void handRequest(miniRpc::HelloResponse* response)
{
    std::string msg = response->message();
    std::cout <<"收到消息:" << msg << std::endl;
}

int main()
{
    miniRpc::RpcApplication application;
    application.init();
    miniRpc::RpcChannel channl;
    // miniRpc::MyRpcService_Stub stub(&channl);
    miniRpc::MyRpcService::Stub* service = new miniRpc::MyRpcService::Stub(&channl);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    miniRpc::HelloRequest request;
    request.set_name("xiong");
    miniRpc::HelloResponse response;
    google::protobuf::Closure* callback = google::protobuf::NewCallback(handRequest,&response);
    service->SayHello(nullptr,&request,&response,callback);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    delete service;
    return 0;
}
