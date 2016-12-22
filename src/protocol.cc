#include "protocol.h"
#include "smart_pointer.h"

#include <arpa/inet.h>


using e7::common::buffer;
using e7::common::proto_head;
using e7::common::proto_naked;
using e7::common::proto_sjsonb;
using e7::common::cmem_release;
using e7::common::dela_release;
using e7::common::array_raii;
using e7::common::smart_pointer;


static const uint32_t MAGIC_NO = 0xE78F8A9DU;


buffer *proto_naked::encode_mem(void *d, ssize_t len)
{
    buffer *rslt;
    proto_head hd;

    rslt = new buffer(sizeof(proto_head) + len);

    hd.magic_no = ::htonl(MAGIC_NO);
    hd.version = ::htonl(1000U);
    hd.ent_offset = ::htonl(sizeof(proto_head));
    hd.ent_sz = ::htonl(len);
    hd.checksum = ::htonl(0xFFFFFFFFU);

    rslt->append(&hd, sizeof(proto_head));
    rslt->append(d, len);

    return rslt;
}


buffer *proto_naked::encode(buffer *content)
{
    buffer *rslt;

    rslt = proto_naked::encode_mem(content->b, content->length);
    content->seek = content->length;

    return rslt;
}


int proto_naked::decode(buffer *stream, buffer **content)
{
    uint32_t ne_magic_no = ::htonl(MAGIC_NO);
    uint32_t ent_ofst, ent_sz;

    std::string str_magic(
        reinterpret_cast<char *>(&ne_magic_no), sizeof(ne_magic_no)
    );
    std::string str_buf(reinterpret_cast<char *>(stream->b), stream->length);

    ssize_t pack_pos = str_buf.find(str_magic);
    if (std::string::npos == pack_pos) {
        stream->reset(); // 无效缓冲，清除数据
        return -1;
    }
    stream->seek += pack_pos; // 丢弃无效数据

    // 有效包头
    proto_head *head = reinterpret_cast<proto_head *>(&stream->b[pack_pos]);

    if (1000U != ::ntohl(head->version)) {
        ++stream->seek;
        return decode(stream, content); // 递归
    }

    ent_ofst = ::ntohl(head->ent_offset);
    ent_sz = ::ntohl(head->ent_sz);
    if (ent_ofst < sizeof(proto_head)) {
        ++stream->seek;
        return decode(stream, content); // 递归
    }

    if (ent_ofst + ent_sz > stream->length) {
        // 不足以解析
        return -1;
    }

    *content = new buffer(ent_sz);
    (*content)->append(&stream->b[ent_ofst], ent_sz);

    stream->seek += ent_ofst + ent_sz; // 丢弃已解析的数据

    return 0;
}


buffer *proto_sjsonb::encode(cJSON *content)
{
    char *s = ::cJSON_PrintUnformatted(content);
    ASSERT(s);
    single_raii<char *, cmem_release> auto_free_s(s);

    return proto_naked::encode_mem(s, ::strlen(s));
}


int proto_sjsonb::decode(buffer *stream, cJSON **root)
{
    int rslt;
    buffer *content;

    rslt = proto_naked::decode(stream, &content);
    if (-1 == rslt) {
        return -1;
    }

    std::string str_content(
        reinterpret_cast<char *>(content->b), content->length
    );
    *root = ::cJSON_Parse(str_content.c_str());

    return (NULL == *root) ? -1 : 0;
}
