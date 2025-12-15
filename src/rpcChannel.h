#pragma once
#include <string>
#include <functional>
#include "threadpool.h"
#include "zkclient.h"
#include "TcpClient.h"
class RpcChannel
{
private:
public:
    RpcChannel();
    ~RpcChannel() = default;
    void callMethodAsync(const std::string& serviceName,
        const std::string& methodName,const std::string& request,
    std::function<void(const std::string&)> callback);

};
