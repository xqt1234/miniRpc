#include <iostream>
#include "provider.h"
#include "rpcApplication.h"
int main()
{
    miniRpc::RpcApplication application;
    application.init();
    miniRpc::ProVider provider;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::shared_ptr<miniRpc::RpcService> service = std::make_shared<miniRpc::RpcService>();
    service->setServiceName("UserService2");
    provider.AddService(service);
    std::cout << "你好呀" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(500));
    return 0;
}

