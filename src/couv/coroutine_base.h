/*
 * coroutine_base.h
 *
 *  Created on: 2015年7月2日
 *      Author: zhusheng
 */

#ifndef SRC_COUV_COROUTINE_BASE_H_
#define SRC_COUV_COROUTINE_BASE_H_

#include <ucontext.h>
#include <functional>
#include <memory>

#include "couv/delegate.h"

namespace couv
{

class coroutine_base
{
public:
    coroutine_base();
    virtual ~coroutine_base();
    virtual void resume_coroutine(coroutine_ptr other);

    //void load();

    int id()const { return m_id; }
    ucontext_t& context()  { return m_ctx;     }
    bool is_active()const  { return m_active;  }
    bool is_blocked()const { return m_blocked; }
    bool is_done()const    { return m_done;    }
    void set_blocked(bool block)    { m_blocked = block; }
    void set_delegate(delegate_t *d){ m_delegate = d;    }

    static coroutine_ptr self();
protected:
    bool m_active;
    bool m_blocked;
    bool m_done;
    int  m_id;
    ucontext_t m_ctx;
    delegate_t*  m_delegate;
};

} /* namespace coroutine */

#endif /* SRC_COUV_COROUTINE_BASE_H_ */
