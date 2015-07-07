/*
 * main.cpp
 *
 *  Created on: 2015年7月2日
 *      Author: zhusheng
 */


#include "coroutine/scheduler.h"
#include "coroutine/coroutine.h"
#include "log/Logger.h"
#include "iostream"

int main()
{
    Logger::logger().setDevice(NULL);

    coroutine::scheduler scheduler1;
    std::cout << "hello world 0\n";

    coroutine::coroutine_ptr r1, r2, r3;

    r1 = scheduler1.add([&r1,&r2,&r3]{
            std::cout << "hello world 1.1\n";
            yield(r3);
            std::cout << "hello world 1.2\n";
            yield(r2);
    });
    r2 = scheduler1.add([&r1,&r2,&r3]{
            std::cout << "hello world 2.1\n";
            yield(r1);
            std::cout << "hello world 2.2\n";
            yield();
    });
    r3 = scheduler1.add([&r1,&r2,&r3]{
            std::cout << "hello world 3.1\n";
            yield(r2);
            std::cout << "hello world 3.2\n";
            yield();
    });
    std::cout << "hello world 4\n";
    scheduler1.run();
    std::cout << "hello world 5\n";

    return 0;
}

