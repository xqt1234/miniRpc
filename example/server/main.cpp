#include <iostream>
#include "provider.h"
#include "rpcApplication.h"
#include "myRpcService.h"
int main()
{
    miniRpc::RpcApplication application;
    application.init();
    miniRpc::ProVider provider;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    // std::shared_ptr<miniRpc::RpcService> service = std::make_shared<miniRpc::RpcService>();
    // service->setServiceName("UserService2");
    MyRpcService service;
    std::string servicename = service.GetDescriptor()->name();
    std::string servicename2 = service.GetDescriptor()->full_name();
    std::cout << servicename << ":" << servicename2 << std::endl;
    provider.AddService(&service);
    std::cout << "你好呀" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;
}

