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
#include "log/Logger.h"

namespace couv
{

namespace uvpp
{

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

    uv_loop_t* get()
    {
        return m_loop.get();
    }

    int run(uv_run_mode mode = UV_RUN_DEFAULT)
    {
        return uv_run(get(), mode);
    }
};

template<typename T>
class handle_t
{
    loop_t* m_loop;
    std::shared_ptr<T> m_handle;

    static void close(T* t)
    {
        ::uv_close(reinterpret_cast<uv_handle_t*>(t), on_close);
    }
    static void on_close(uv_handle_t* h)
    {
        delete h;
    }
public:
    handle_t(loop_t* loop)
            : m_loop(loop), m_handle(new T, close)
    {
        get()->data = this;
    }
    handle_t(loop_t* loop, T* t)
            : m_loop(loop), m_handle(t, close)
    {
        get()->data = this;
    }

    T*get()
    {
        return m_handle.get();
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

    bool is_active()
    {
        return ::uv_is_active(handle());
    }

    bool is_closing()
    {
        return ::uv_is_closing(handle());
    }
};

template<typename W, typename T>
class stream_t: public handle_t<T>
{
protected:
    typedef stream_t<W,T> self_type;
    typedef W wrapper_type;

    channel<int> m_accept_channel;
    channel<int> m_read_channel;
    channel<int> m_write_channel;

    mem_buf_t m_alloc_buf;
public:
    stream_t(loop_t* loop)
            : handle_t<T>(loop)
    {
    }
    stream_t(loop_t* loop,T* t)
            : handle_t<T>(loop, t)
    {
    }

    uv_stream_t* stream()
    {
        return reinterpret_cast<uv_stream_t*>(this->get());
    }

    int listen(int backlog=128)
    {
        return ::uv_listen(stream(), backlog,
                [](uv_stream_t* t, int status){
                    static_cast<self_type*>(t->data)->m_accept_channel.send(status);
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
                    *buf = static_cast<self_type*>(t->data)->m_alloc_buf;
                },
                [](uv_stream_t* t, ssize_t nread, const uv_buf_t* buf){
                    self_type* self = static_cast<self_type*>(t->data);
                    logAssert(buf->base == self->m_alloc_buf.base);
                    self->m_read_channel.send(nread);
                });
    }

    int read_stop()
    {
        return ::uv_read_stop(stream());
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

    /**
     * void cb(W* client, int status);
     */
    template<typename C = W>
    void accept(const std::function<void(W*,int)>& cb)
    {
        int status = 0;
        while(status == 0 && !m_accept_channel.is_closed()){
            m_accept_channel.receive(status);
            if(status) {
                cb(nullptr, status);
            } else {
                C c(this->loop());
                accept(c);
                cb(&c, status);
            }
        }
    }

    /**
     * void cb(W* client, char* buf, ssize_t len);
     */
    int read(const std::function<void(W*, char*, ssize_t)>& cb)
    {
        read_start();

        int nread = 0;
        while(nread > 0 && !m_accept_channel.is_closed()){
            m_read_channel.receive(nread);
            cb(static_cast<W*>(this), m_alloc_buf.base, nread);
        }
        return nread;
    }

    /**
     * void cb(W* client, ssize_t len);
     */
    int write(const char* str, int len, const std::function<void(W*, ssize_t)>& cb)
    {
        typedef mixed<uv_write_t, mem_buf_t> write_t;

        write_t* request = new write_t(mem_buf_t(0));
        request->data = this;
        request->m_data.assign(str, len);

        int ret = uv_write(request, stream(), &request->m_data, 1, [](uv_write_t* req, int status){
            void* data = req->data;
            delete static_cast<write_t*>(req);
            static_cast<self_type*>(data)->m_read_channel.send(status);
        });

        if(ret) {
            cb(static_cast<W*>(this), ret);
        } else {
            cb(static_cast<W*>(this), m_read_channel.receive());
        }
        return ret;
    }

    int write(const char* str, int len)
    {
        typedef mixed<uv_write_t, mem_buf_t> write_t;

        write_t* request = new write_t(mem_buf_t(str,len));
        request->data = this;
        return ::uv_write(request, stream(), &request->m_data, 1, [](uv_write_t* req, int status){
            delete static_cast<write_t*>(req);
        });
    }

    /**
     * void cb(W* client, int len);
     */
    int shutdown()
    {
        uv_shutdown_t* req = new uv_shutdown_t();
        return ::uv_shutdown(req, stream(),
                [](uv_shutdown_t* req, int status){
                    delete static_cast<uv_shutdown_t*>(req);
                });
    }
};
    
class dns_t
{
    loop_t* m_loop;
    channel< std::tuple<int,addrinfo*> > m_getaddr_channel;
public:
    dns_t(loop_t* loop):m_loop(loop)
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
            std::tuple<int,addrinfo*> node = m_getaddr_channel.receive();
            int status = std::get<0>(node);
            struct addrinfo* addr_info = std::get<1>(node);
            cb(status, addr_info);
            ::uv_freeaddrinfo(addr_info);
            return status;
        }
    }
};

class tcp_t: public stream_t<tcp_t, uv_tcp_t>
{
protected:
    typedef stream_t<tcp_t, uv_tcp_t> parent_type;
    typedef mixed<uv_connect_t, sock_addr> connect_t;

    channel<int> m_connect_channel;
public:
    tcp_t(loop_t* loop)
            : parent_type(loop)
    {
        ::uv_tcp_init(loop->get(), get());
    }
    explicit tcp_t(loop_t* loop, uv_tcp_t*t)
            : parent_type(loop, t)
    {
        ::uv_tcp_init(loop->get(), get());
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
        req->data = this;
        int ret = ::uv_tcp_connect(req, get(), req->m_data.addr(), [](uv_connect_t* req, int status){
            void* data = req->data;
            delete req;
            static_cast<tcp_t*>(data)->m_connect_channel.send(status);
        });
        if(ret) {
            cb(this, ret);
            delete req;
        } else {
            cb(this, m_connect_channel.receive());
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
