#include <iostream>
#include "rpcChannel.h"

void handRequest(const std::string& response)
{
    std::cout <<"收到消息" << response << std::endl;
}
int main()
{
    RpcChannel channl;
    std::string request = "hello world";
    channl.callMethodAsync("UserService2","aaa",request,handRequest);
    std::cout << "你好呀" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;
}