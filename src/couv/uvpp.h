/*
 * uvpp.h
 *
 *  Created on: 2015年7月7日
 *      Author: zhusheng
 */

#ifndef SRC_COUV_UVPP_H_
#define SRC_COUV_UVPP_H_

#include <stdlib.h>
#include <memory>
#include <functional>
#include <tuple>
#include <uv.h>
#include <sys/un.h>
#include "couv/sem.h"
#include "couv/channel.h"
#include "couv/scheduler.h"
#include "log/Logger.h"

namespace couv
{

namespace uvpp
{

extern const char* get_handle_name(uv_handle_t* h);

template<typename T1, typename T2>
struct mixed: public T1
{
    T2 m_data;

    mixed(){}
    mixed(T2&& d):m_data(std::move(d)){}
};

struct sock_addr
{
    union
    {
        struct sockaddr m_addr;
        struct sockaddr_in m_addr4;
        struct sockaddr_in6 m_addr6;
        struct sockaddr_un m_addr_unix;
    } m_d;

    sock_addr(){}
    sock_addr(const struct sockaddr* addr){ assign(addr);}
    struct sockaddr*     addr()   { return &m_d.m_addr;      }
    struct sockaddr_in*  addr_v4(){ return &m_d.m_addr4;     }
    struct sockaddr_in6* addr_v6(){ return &m_d.m_addr6;     }
    struct sockaddr_un*  addr_un(){ return &m_d.m_addr_unix; }

    void assign(const struct sockaddr* addr)
    {
        if(addr->sa_family==AF_INET) {
            m_d.m_addr4 = *reinterpret_cast<const struct sockaddr_in*>(addr);
        } else if(addr->sa_family==AF_INET6) {
            m_d.m_addr6 = *reinterpret_cast<const struct sockaddr_in6*>(addr);
        } else if(addr->sa_family==AF_UNIX) {
            m_d.m_addr_unix = *reinterpret_cast<const struct sockaddr_un*>(addr);
        }
    }
};

class buf_t: public uv_buf_t
{
public:
    buf_t():uv_buf_t(){base=nullptr; len=0;}
    buf_t(const uv_buf_t& t):uv_buf_t(t){}
    buf_t(const uv_buf_t& t, size_t size){base=t.base; len=size;}
    buf_t(char* t, size_t size){base=t; len=size;}
    virtual ~buf_t(){}
};

class mem_buf_t: public buf_t
{
    enum{Size = 65536};
public:
    mem_buf_t(size_t size = Size):buf_t((char*)malloc(size), size){}
    mem_buf_t(const mem_buf_t& other) = delete;
    mem_buf_t operator=(const mem_buf_t& other) = delete;
    
    mem_buf_t(mem_buf_t&& other):buf_t(other){
        other.base=nullptr;
        other.len=0;
    }
    mem_buf_t(const char* str, size_t size):buf_t(memdup(str, size), size){}

    static char* memdup(const char* str, size_t len)
    {
        if(str==nullptr || len == 0)
            return nullptr;
        
        char* p = (char*)malloc(len);
        ::memcpy(p, str, len);
        return p;
    }
    
    virtual ~mem_buf_t()
    {
        if(base) {
            ::free(base);
            base=nullptr;
        }
    }

    void resize(size_t size)
    {
        if(size > len){
            base= (char*)realloc(base, size);
            len=size;
        }
    }

    void assign(const buf_t& t)
    {
        resize(len);
        ::memmove(base, t.base, t.len);
    }

    void assign(const buf_t& t, size_t len)
    {
        resize(len);
        ::memmove(base, t.base, len);
    }

    void assign(const char* t, size_t len)
    {
        resize(len);
        ::memmove(base, t, len);
    }

    void assign(mem_buf_t&& other)
    {
        base = other.base;
        len = other.len;
    }
};


class loop_t
{
    std::shared_ptr<uv_loop_t> m_loop;
public:
    loop_t()
            : m_loop(uv_loop_new(), uv_loop_delete)
    {
    }
    explicit loop_t(uv_loop_t* loop)
            : m_loop(loop, uv_loop_delete)
    {
    }

    ~loop_t()
    {
        stop();
    }

    uv_loop_t* get()
    {
        return m_loop.get();
    }

    bool is_alive()
    {
        return ::uv_loop_alive(get());
    }

    void stop()
    {
        ::uv_stop(get());
    }

    int run(uv_run_mode mode = UV_RUN_DEFAULT)
    {
        return uv_run(get(), mode);
    }
};

class uvscheduler_t: public scheduler_t
{
    loop_t s_loop;
public:
    uvscheduler_t(bool install=true)
            :scheduler_t(install)
    {
    }

    void run()
    {
        install();

        while(!m_stop)
        {
            bool found = false;
            for(std::deque<coroutine_ptr>::iterator it = m_queue.begin(); it != m_queue.end(); ++it)
            {
                coroutine_ptr& r = *it;
                if(!r->is_blocked() && !r->is_done() && r != m_current)
                {
                    found = true;
                    resume_coroutine(r);
                    break;
                }
            }

            if(!found)
            {
                if(s_loop.run(UV_RUN_NOWAIT) == 0)
                {
                    break;
                }
            }
        }
        
        // close all pending handles.
        ::uv_walk(s_loop.get(), [](uv_handle_t* handle, void* arg){
            if(handle && !::uv_is_closing(handle)) {
                // TODO: release object in coroutine
                ::uv_close(handle, nullptr);
            }
        }, nullptr);
        
        s_loop.run();
        s_instance = nullptr;
    }

    loop_t* loop()
    {
        return &s_loop;
    }

    void stop()
    {
        scheduler_t::stop();
        s_loop.stop();
    }

    static uvscheduler_t* self()
    {
        return static_cast<uvscheduler_t*>(s_instance);
    }
};


class dns_t
{
    loop_t* m_loop;
    channel< std::tuple<int,addrinfo*> > m_getaddr_channel;
public:
    dns_t(loop_t* lp=nullptr):m_loop(lp)
    {
    }

    int get_addr_info(const char*host, int port, std::function<void(int, struct addrinfo*)>&& cb)
    {
        typedef mixed<uv_getaddrinfo_t, addrinfo> getaddrinfo_t;

        getaddrinfo_t* req = new getaddrinfo_t;
        req->data = this;

        struct addrinfo& hints = req->m_data;
        memset(&hints, 0, sizeof(addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = 0;
        hints.ai_protocol = 0;

        char buf[20];
        snprintf(buf, 20, "%d", port);

        int ret = ::uv_getaddrinfo(m_loop->get(), req,
                                   [](uv_getaddrinfo_t* req,int status, struct addrinfo* res){
                                       void*data = req->data;
                                       delete req;
                                       static_cast<dns_t*>(data)->m_getaddr_channel.send(std::make_tuple(status, res));
                                   }, host, buf, &req->m_data);
        if(ret){
            cb(ret, nullptr);
            return ret;
        } else {
            std::tuple<int,addrinfo*> node;
            if(m_getaddr_channel.receive(node)){
                int status = std::get<0>(node);
                struct addrinfo* addr_info = std::get<1>(node);
                cb(status, addr_info);
                ::uv_freeaddrinfo(addr_info);
                return status;
            }
        }
        return -1;
    }
};

namespace create_helper
{

    inline void init(loop_t* lp, uv_tcp_t* h)
    {
        ::uv_tcp_init(lp->get(), h);
    }

    inline void init(loop_t* lp, uv_idle_t* h)
    {
        ::uv_idle_init(lp->get(), h);
    }

    template<typename T> inline T* create(loop_t* lp)
    {
        T* t = new T;
        init(lp, t);
        return t;
    }
};

template<typename T>
class handle_t
{
    typedef handle_t<T> self_type;
    
    loop_t* m_loop;
    T* m_handle;

    static void on_close(uv_handle_t* h)
    {
        logDebug("delete handle_t %s %p", get_handle_name(h), h);
        delete h;
    }

    static loop_t* get_loop(loop_t* loop)
    {
        return loop?loop:uvscheduler_t::self()->loop();
    }
public:
    handle_t(loop_t* lp=nullptr, T*t=nullptr)
        : m_loop(get_loop(lp)), m_handle(t?t:create_helper::create<T>(m_loop))
    {
        m_handle->data = this;
        logDebug("new handle_t %s %p", get_handle_name(handle()), handle());
    }
    
    ~handle_t()
    {
        close();
    }

    void close()
    {
        if(m_handle)
        {
            logDebug("close handle_t %s %p", get_handle_name(handle()), m_handle);

            uv_handle_t* t = handle();
            m_handle = nullptr;
            if(!::uv_is_closing(t))
            {
                ::uv_close(reinterpret_cast<uv_handle_t*>(t), on_close);
            }
        }
    }
    
    handle_t(const self_type& other) = delete;
    self_type& operator=(const self_type& other) = delete;
    
    T*get()
    {
        return m_handle;
    }

    loop_t* loop()
    {
        return m_loop;
    }

    uv_handle_t* handle()
    {
        return reinterpret_cast<uv_handle_t*>(this->get());
    }

    int fd()
    {
        uv_os_fd_t fno = -1;
        ::uv_fileno(handle(), &fno);
        return fno;
    }

    bool is_valid()
    {
        return get()!=nullptr;
    }

    bool is_active()
    {
        return ::uv_is_active(handle());
    }

    bool is_closing()
    {
        return ::uv_is_closing(handle());
    }

    void unref()
    {
        ::uv_unref(handle());
    }

    void ref()
    {
        ::uv_ref(handle());
    }
};
    
class idle_t: public handle_t<uv_idle_t>
{
    std::function<void()> m_cb;
public:
    idle_t(std::function<void()>&& cb, loop_t* lp=nullptr)
            :handle_t<uv_idle_t>(lp)
    {
        std::swap(m_cb, cb);
        start();
    }

    int start()
    {
        return ::uv_idle_start(get(), [](uv_idle_t* t){
            idle_t* self = static_cast<idle_t*>(t->data);
            self->m_cb();
            delete self;
        });
    }

    int stop()
    {
        return ::uv_idle_stop(get());
    }
};

template<typename W, typename T>
class stream_t: public handle_t<T>
{
protected:
    typedef stream_t<W,T> self_type;
    typedef W wrapper_type;

    channel<int> m_read_channel;
    channel<int> m_write_channel;

    mem_buf_t m_alloc_buf;
public:
    stream_t(loop_t* lp=nullptr, T*t=nullptr)
            : handle_t<T>(lp,t)
    {
    }

    uv_stream_t* stream()
    {
        return reinterpret_cast<uv_stream_t*>(this->get());
    }
    
    bool is_readable()const
    {
        return ::uv_is_readable(stream());
    }
    
    bool is_writable()const
    {
        return ::uv_is_writable(stream());
    }
    
    int set_blocking(bool blocking)
    {
        return ::uv_stream_set_blocking(stream(), blocking);
    }

    int listen(int backlog=128)
    {
        return ::uv_listen(stream(), backlog,
                [](uv_stream_t* t, int status){
                    logDebug("uv_connection_cb: %d", status);
                    static_cast<self_type*>(t->data)->m_read_channel.send(status);
                });
    }

    int accept(self_type& client)
    {
        return ::uv_accept(stream(), client.stream());
    }

    int read_start()
    {
        return ::uv_read_start(stream(),
                [](uv_handle_t* t, size_t suggested_size, uv_buf_t* buf){
                    logDebug("uv_alloc_cb: %lu", suggested_size);
                    *buf = static_cast<self_type*>(t->data)->m_alloc_buf;
                },
                [](uv_stream_t* t, ssize_t nread, const uv_buf_t* buf){
                    logDebug("uv_read_cb: %ld", nread);
                    self_type* self = static_cast<self_type*>(t->data);
                    logAssert(buf->base == self->m_alloc_buf.base);
                    self->m_read_channel.send(nread);
                });
    }

    int read_stop()
    {
        return ::uv_read_stop(stream());
    }

    /**
     * void cb(W* client, int status);
     */
    template<typename Client = W>
    void accept(const std::function<void(const std::shared_ptr<Client>&,int)>& cb)
    {
        int status = 0;
        while(m_read_channel.receive(status) && status == 0){
            logDebug("accept: %d", status);
            if(status) {
                cb(nullptr, status);
            } else {
                std::shared_ptr<Client> client = std::make_shared<Client>(this->loop());
                accept(*client);
                cb(client, status);
            }
        }
        logDebug("accept quit: %d", status);
    }

    /**
     * void cb(W* client, char* buf, ssize_t len);
     */
    int read(const std::function<void(W*, char*, ssize_t)>& cb)
    {
        read_start();

        int nread = 0;
        while(m_read_channel.receive(nread) && nread > 0){
            logDebug("read: %ld", nread);
            cb(static_cast<W*>(this), m_alloc_buf.base, nread);
        }
        logDebug("read quit: %ld", nread);
        return nread;
    }

    /**
     * void cb(W* client, ssize_t len);
     */
    int write(const char* str, int len, const std::function<void(W*, ssize_t)>& cb)
    {
        typedef mixed<uv_write_t, mem_buf_t> write_t;

        write_t* request = new write_t(mem_buf_t(str,len));
        request->data = this;
        
        int ret = uv_write(request, stream(), &request->m_data, 1, [](uv_write_t* req, int status){
            void* data = req->data;
            delete static_cast<write_t*>(req);
            static_cast<self_type*>(data)->m_write_channel.send(status);
        });

        if(ret) {
            cb(static_cast<W*>(this), ret);
        } else if(m_write_channel.receive(ret)) {
            cb(static_cast<W*>(this), ret);
        }
        logDebug("write quit: %ld", ret);
        return ret;
    }

    int write(const char* str, int len)
    {
        typedef mixed<uv_write_t, mem_buf_t> write_t;

        write_t* request = new write_t(mem_buf_t(str,len));
        request->data = this;
        int ret = ::uv_write(request, stream(), &request->m_data, 1, [](uv_write_t* req, int status){
            delete static_cast<write_t*>(req);
        });
        logDebug("write quit: %ld", ret);
        return ret;
    }

    /**
     * void cb(W* client, int len);
     */
    int shutdown()
    {
        m_read_channel.close();
        m_write_channel.close();
        
        uv_shutdown_t* req = new uv_shutdown_t();
        req->data = this;
        
        logDebug("shutdown: %p", this);
        int ret = ::uv_shutdown(req, stream(),
                [](uv_shutdown_t* req, int status){
                    logDebug("shutdown callback:");
                    delete req;
                });
        logDebug("shutdown quit: %ld", ret);
        return ret;
    }
};

class tcp_t: public stream_t<tcp_t, uv_tcp_t>
{
protected:
    typedef stream_t<tcp_t, uv_tcp_t> parent_type;
    typedef mixed<uv_connect_t, sock_addr> connect_t;
public:
    tcp_t(loop_t* lp=nullptr)
            : parent_type(lp)
    {
    }

    int set_nodely(bool enable)
    {
        return ::uv_tcp_nodelay(get(), enable);
    }

    int set_keepalive(bool enable, unsigned int delay)
    {
        return ::uv_tcp_keepalive(get(), enable, delay);
    }

    int set_simultaneous_accepts(bool enable)
    {
        return ::uv_tcp_simultaneous_accepts(get(), enable);
    }

    int get_sockname(struct sockaddr* name, int* namelen)
    {
        return ::uv_tcp_getsockname(get(), name, namelen);
    }

    int get_peername(struct sockaddr* name, int* namelen)
    {
        return ::uv_tcp_getpeername(get(), name, namelen);
    }

    int bind(const struct sockaddr* addr, unsigned int flags = 0)
    {
        return ::uv_tcp_bind(get(), addr, flags);
    }
    
    int listen(int port, const char* host="0.0.0.0", int backlog=128)
    {
        dns_t dns(loop());
        
        int ret = 0;
        dns.get_addr_info(host, port, [this, &ret](int status, struct addrinfo * addr){
            if (status == 0) {
                ret = bind(addr->ai_addr);
            } else {
                ret = status;
            }
        });
        
        if (ret == 0) {
            ret = parent_type::listen(backlog);
        }
        return ret;
    }

    int connect(connect_t* req, const std::function<void(tcp_t*, int)>& cb)
    {
        channel<int> chan;
        req->data = &chan;
        int ret = ::uv_tcp_connect(req, get(), req->m_data.addr(), [](uv_connect_t* req, int status){
            void* data = req->data;
            delete req;
            ((channel<int>*)data)->send(status);
        });
        if(ret) {
            cb(this, ret);
            delete req;
        } else if(chan.receive(ret)) {
            cb(this, ret);
        }
        return ret;
    }

    int connnect_ip(const char*ip, int port, const std::function<void(tcp_t*, int)>& cb)
    {
        connect_t* req = new connect_t;
        int ret = ::uv_ip4_addr(ip, port, req->m_data.addr_v4());
        if(ret) {
            cb(nullptr, ret);
            delete req;
        } else {
            connect(req, cb);
        }
        return ret;
    }

    int connnect_ipv6(const char*ip, int port, const std::function<void(tcp_t*, int)>& cb)
    {
        connect_t* req = new connect_t;
        int ret = ::uv_ip6_addr(ip, port, req->m_data.addr_v6());
        if(ret) {
            cb(nullptr, ret);
            delete req;
        } else {
            connect(req, cb);
        }
        return ret;
    }

    int connect_host(const char*host, int port, const std::function<void(tcp_t*, int)>& cb)
    {
        int ret = 0;
        connect_t* req = new connect_t;
        
        dns_t dns(loop());
        dns.get_addr_info(host, port, [&req, &ret](int status, struct addrinfo * addr){
            if (status == 0) {
                req->m_data.assign(addr->ai_addr);
            } else {
                ret = status;
            }
        });
        
        if (ret == 0) {
            ret = connect(req, cb);
        } else {
            delete req;
        }
        return ret;
    }
};

} /* namespace couv */

} /* namespace couv */

#endif /* SRC_COUV_UVPP_H_ */
