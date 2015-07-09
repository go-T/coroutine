/*
 * scheduler.h
 *
 *  Created on: 2015年7月2日
 *      Author: zhusheng
 */

#ifndef SRC_COUV_SCHEDULER_H_
#define SRC_COUV_SCHEDULER_H_

#include <deque>

#include "couv/delegate.h"
#include "log/Logger.h"

#define yield(...) do{ \
    logDebug("yield from %d", couv::coroutine_base::self()->id()); \
    couv::scheduler_t::self()->yield_coroutine( __VA_ARGS__ ); \
    } while(0)

#define resume couv::scheduler_t::self()->resume

#define go (*couv::scheduler_t::self())<<[&]
#define goo (*couv::scheduler_t::self())<<

namespace couv
{

/**
 * 不管如何切换
 */
class scheduler_t: public delegate_t
{
public:
    typedef void sign_type();
    typedef std::function<sign_type> func_type;
public:
    scheduler_t(bool install=true);
    virtual ~scheduler_t();
    virtual bool add(coroutine_ptr r);
    virtual void remove(coroutine_ptr r);
    virtual void run();
    
    void yield_coroutine(coroutine_ptr r);
    void yield_coroutine();
    
    coroutine_ptr add(const func_type& f);
    coroutine_ptr add(func_type&& f);
    scheduler_t& operator<<(const func_type& f);
    scheduler_t& operator<<(func_type&& f);

    void set_current(coroutine_ptr r) { m_current = r; }
    coroutine_ptr current(){ return m_current; }

    void stop()         { m_stop = true; }
    bool is_stop()const { return m_stop; }

    void install()           { s_instance = this; }
    bool is_installed()const { return s_instance == this; }

    static scheduler_t* self()
    {
        logAssert(s_instance != nullptr);
        return s_instance;
    }

protected:
    virtual void resume_coroutine(coroutine_ptr r);
    virtual bool has_more();

    virtual void on_start(coroutine_base* r);
    virtual void on_stop(coroutine_base* r);

protected:
    static scheduler_t* s_instance;
    bool m_stop;
    coroutine_ptr m_root;
    coroutine_ptr m_current;
    std::deque<coroutine_ptr> m_queue;
};

} /* namespace coroutine */

#endif /* SRC_COUV_SCHEDULER_H_ */
