/*
 * coroutine_base.cpp
 *
 *  Created on: 2015年7月2日
 *      Author: zhusheng
 */
#include "couv/coroutine_base.h"
#include "couv/scheduler.h"
#include "couv/scheduler.h"
#include "log/Logger.h"

namespace couv
{

static int coroutine_id = 0;

coroutine_base::coroutine_base()
        : m_active(false)
        , m_blocked(false)
        , m_done(false)
        , m_id(++coroutine_id)
        , m_delegate(nullptr)
{
}

coroutine_base::~coroutine_base()
{
    logDebug("delete %d", m_id);
}


void coroutine_base::resume_coroutine(coroutine_ptr other)
{
    logDebug("yield %d --> %d", m_id, other->m_id);

    logAssert(this != other.get());
    logAssert(!other->m_active);
    logAssert(!other->m_blocked);

    m_active = false;
    other->m_active = true;

    current_scheduler->set_current(other);
    swapcontext(&m_ctx, &other->m_ctx);
}

} /* namespace coroutine */
