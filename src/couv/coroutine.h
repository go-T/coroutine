/*
 * coroutine.h
 *
 *  Created on: 2015年7月2日
 *      Author: zhusheng
 */

#ifndef SRC_COUV_COROUTINE_H_
#define SRC_COUV_COROUTINE_H_

#include <ucontext.h>
#include <functional>

#include "couv/coroutine_base.h"
#include "couv/stack.h"

namespace couv
{

class coroutine_base;

// customer
class coroutine: public coroutine_base
{
public:
    coroutine(func_type f);

protected:
    static void on_run(coroutine* thiz);

    virtual void run();

protected:
    func_type m_fun;
    stack m_stack;

};

} /* namespace coroutine */

#endif /* SRC_COUV_COROUTINE_H_ */
