#ifndef __SQLAGENT_PROTOCOL_H__
#define __SQLAGENT_PROTOCOL_H__


#include "buffer.h"

#include "cJSON.h"


namespace e7 {
namespace common {
struct proto_head {
    uint32_t magic_no; // 协议包标识
    uint32_t version; // 协议版本
    uint32_t ent_offset; // 内容起始偏移
    uint32_t ent_sz; // 内容长度
    uint32_t checksum; // 内容校验码
} __attribute__ ((packed));


// 基本的裸缓冲协议
namespace proto_naked_ {
    class proto_naked;
}
typedef proto_naked_::proto_naked proto_naked;


namespace proto_sjsonb_ {
    class proto_sjsonb ;
}
typedef proto_sjsonb_::proto_sjsonb proto_sjsonb;


class proto_naked_::proto_naked : public e7::common::object
{
    friend class e7::common::smart_pointer<proto_naked>;

public:
    // 调用者释放内存
    static buffer *encode_mem(void *d, ssize_t len);
    static buffer *encode(buffer *content);
    static int decode(buffer *stream, buffer **content);
};


class proto_sjsonb_::proto_sjsonb : public e7::common::object
{
    friend class e7::common::smart_pointer<proto_sjsonb>;

public:
    static buffer *encode(cJSON *content);
    static int decode(buffer *stream, cJSON **root);

private:
    buffer_::buffer *bf;
};
} // end of namespace common
} // end of namespace e7
#endif // __SQLAGENT_PROTOCOL_H__
