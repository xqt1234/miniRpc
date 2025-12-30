#pragma once
#include <string>
#include <atomic>
#include <Buffer.h>
#include <functional>
namespace miniRpc
{
    class BuildProto
    {
    private:
        /* data */
    public:
        BuildProto(/* args */) = default;
        ~BuildProto()= default;
        static void enCodeRequest(const std::string& request,int64_t requestId,std::function<void(const std::string&)> callback);
        static void deCodeResponse(mymuduo::Buffer* buffer,std::function<void(const std::string&,int64_t requestId)> callback);
    };
}