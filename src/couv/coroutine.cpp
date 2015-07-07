/*
 * coroutine.cpp
 *
 *  Created on: 2015年7月2日
 *      Author: zhusheng
 */

#include "couv/coroutine.h"
#include <stdlib.h>


namespace couv
{

coroutine::coroutine(func_type f) :
    coroutine_base(), m_fun(f)
{
    ::getcontext(&m_ctx);

    m_ctx.uc_stack.ss_sp = m_stack.data();
    m_ctx.uc_stack.ss_size = m_stack.size();

    ::makecontext(&m_ctx, reinterpret_cast<sign_type*>(&coroutine::on_run), 1, this);
}

void coroutine::on_run(coroutine* thiz)
{
    if(thiz) {
        thiz->run();
    }
}

void coroutine::run()
{
    if(m_delegate) {
        m_delegate->on_start(this);
    }

    m_fun();

    m_done = true;
    if(m_delegate) {
        m_delegate->on_stop(this);
    }
}

} /* namespace coroutine */
