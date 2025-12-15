#include "rpcChannel.h"

RpcChannel::RpcChannel()
{
}

void RpcChannel::callMethodAsync(const std::string &serviceName, const std::string &methodName, const std::string &request, std::function<void(const std::string &)> callback)
{

}
