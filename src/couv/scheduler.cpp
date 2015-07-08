/*
 * scheduler.cpp
 *
 *  Created on: 2015年7月2日
 *      Author: zhusheng
 */
#include "couv/scheduler.h"
#include <iostream>
#include <algorithm>
#include "couv/coroutine.h"

namespace couv
{
//static  scheduler_t local_scheduler;
//scheduler_t* current_scheduler = &local_scheduler;
scheduler_t* current_scheduler = nullptr;

scheduler_t::scheduler_t(bool install)
{
    m_root = std::make_shared<coroutine_base>();
    m_current = m_root;
    if(install) {
        current_scheduler = this;
    }
}

scheduler_t::~scheduler_t()
{
    logAssert(m_current == m_root);
}

coroutine_ptr scheduler_t::add(coroutine_base::func_type&& f)
{
    return add(coroutine_ptr(new coroutine(f)));
}

coroutine_ptr scheduler_t::add(coroutine_ptr r)
{
    logAssert(r != nullptr);

    if(m_root == r || r->is_done()) {
        return r;
    }

    logDebug("add %d", r->id());
    m_queue.push_back(r);
    return r;
}

void scheduler_t::remove(coroutine_ptr r)
{
    logDebug("remove %d", r->id());
    if(r != m_root) {
        auto pos = std::find(m_queue.begin(), m_queue.end(), r);
        if(pos != m_queue.end()) {
            m_queue.erase(pos);
        }
    }
}

/**
 * resume root looper
 */
void scheduler_t::yield_coroutine()
{
    logAssert(m_current != m_root);
    resume_coroutine(m_root);
}

void scheduler_t::yield_coroutine(coroutine_ptr r)
{
    if(m_current == r || r->is_done() || r->is_blocked()) {
        return;
    }
    resume_coroutine(r);
}

void scheduler_t::resume_coroutine(coroutine_ptr r)
{
    r->set_delegate(this);
    m_current->resume_coroutine(r);
}

void scheduler_t::run()
{
    //logAssert(current_scheduler == nullptr);

    current_scheduler = this;

    while(!m_queue.empty())
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
            logWarn("all coroutine blocked \n");
            exit(1);
        }
        else
        {
            logDebug("OK!");
        }
    }

    current_scheduler = nullptr;
}

void scheduler_t::on_start(coroutine_base* r)
{
    logDebug("start %d", r->id());
}

void scheduler_t::on_stop(coroutine_base* r)
{
    logDebug("stop %d", r->id());
    remove(m_current);
    yield_coroutine();
}

} /* namespace coroutine */
