/*
 * scheduler.h
 *
 *  Created on: 2015年7月2日
 *      Author: zhusheng
 */

#ifndef SRC_COROUTINE_SCHEDULER_H_
#define SRC_COROUTINE_SCHEDULER_H_

#include <deque>
#include "coroutine/delegate.h"
#include "coroutine/coroutine_base.h"
#include "log/Logger.h"

#define current_coroutine coroutine::current_scheduler->current()

#define yield(...) do{ \
    logDebug("yield from %d", current_coroutine->id()); \
    coroutine::current_scheduler->yield_coroutine( __VA_ARGS__ ); \
    } while(0)

#define resume coroutine::current_scheduler->resume

namespace coroutine
{

/**
 * 不管如何切换
 */
class scheduler: public delegate
{
public:
    scheduler();
    virtual ~scheduler();
    virtual void run();

    coroutine_ptr add(coroutine_base::func_type&& f);
    coroutine_ptr add(coroutine_ptr r);
    void remove(coroutine_ptr r);

    void yield_coroutine(coroutine_ptr r);
    void yield_coroutine();

    void set_current(coroutine_ptr r) { m_current = r; }
    coroutine_ptr current(){ return m_current; }

protected:
    void resume_coroutine(coroutine_ptr r);

    virtual void on_start(coroutine_base* r);
    virtual void on_stop(coroutine_base* r);

protected:
    coroutine_ptr m_root;
    coroutine_ptr m_current;
    std::deque<coroutine_ptr> m_queue;
};

extern scheduler* current_scheduler;

} /* namespace coroutine */

#endif /* SRC_COROUTINE_SCHEDULER_H_ */
