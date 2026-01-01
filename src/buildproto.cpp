#include "buildproto.h"
#include "public.h"
#include <vector>
#include <endian.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
using namespace miniRpc;
void BuildProto::enCodeRequest(const std::string &request,int64_t requestId,std::function<void(const std::string&)> callback)
{
    int len = sizeof(int32_t) + sizeof(int32_t) + sizeof(int64_t) + request.length();
    int totallen = sizeof(RpcMsgHeader) + request.length();
    std::vector<char> sendvec(totallen);
    RpcMsgHeader* header = reinterpret_cast<RpcMsgHeader*>(sendvec.data());
    header->datalength = htonl(request.length());
    header->magic = htonl(kMagicNumber);
    header->reqId = htobe64(requestId);
    memcpy(sendvec.data() + sizeof(RpcMsgHeader),request.data(),request.length());
    callback(std::string(sendvec.data(),sendvec.size()));
}

void BuildProto::deCodeResponse(mymuduo::Buffer *buffer, std::function<void(const std::string &,int64_t)> callback)
{
    const char *data = buffer->peek();
    int len = buffer->readableBytes();
    while (len > sizeof(RpcMsgHeader))
    {
        RpcMsgHeader rpchead;
        memcpy(&rpchead,buffer->peek(),sizeof(RpcMsgHeader));
        // const RpcMsgHeader* rpchead = reinterpret_cast<const RpcMsgHeader*>(buffer->peek());
        uint32_t magic = ntohl(rpchead.magic);
        if (kMagicNumber != magic)
        {
            buffer->retrieve(1);
            len = buffer->readableBytes();
            data = buffer->peek();
            continue;
        }
        uint32_t datalen = ntohl(rpchead.datalength);
        uint64_t requId = be64toh(rpchead.reqId);
        if (len < sizeof(RpcMsgHeader) + datalen)
        {
            break;
        }
        buffer->retrieve(sizeof(RpcMsgHeader));
        std::string msg = buffer->readAsString(datalen);
        data = buffer->peek();
        len = buffer->readableBytes();
        callback(msg,requId);
        // processReq(conn, msg);
    }
}
