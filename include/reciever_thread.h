#ifndef __SQLAGENT_RECIEVER_THREAD_H__
#define __SQLAGENT_RECIEVER_THREAD_H__


#include "socket.h"


using e7::common::smart_pointer;


namespace sqlagent {
namespace iaction_ {
class iaction;
}
typedef iaction_::iaction iaction;

namespace reciever_arg_ {
    class reciever_arg;
}
typedef reciever_arg_::reciever_arg reciever_arg;

namespace dft_action_ {
    class dft_action;
}
typedef dft_action_::dft_action dft_action;
}


namespace sqlagent {
class iaction_::iaction : public e7::common::object
{
    friend smart_pointer<iaction>;
public:
    virtual ~iaction(void) {}

    virtual void on_connected(connection *) = 0;
    virtual void on_closing(connection *) = 0;
    virtual void on_recv(connection *) = 0;
};


// reciever线程参数
class reciever_arg_::reciever_arg : public e7::common::object
{
public:
    explicit reciever_arg(int channel_fd, iaction *action)
        : channel(channel_fd), act(action)
    {}
    int channel;
    smart_pointer<iaction> act;
};


// 默认action回调
class dft_action_::dft_action : public iaction
{
public:
    virtual void on_connected(connection *conn);
    virtual void on_closing(connection *conn);
    virtual void on_recv(connection *conn);
};
}


extern void *reciever_thread(void *arg);
#endif // __SQLAGENT_RECIEVER_THREAD_H__
