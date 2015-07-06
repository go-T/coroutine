/*
 * sem.cpp
 *
 *  Created on: 2015年7月7日
 *      Author: zhusheng
 */

#include "coroutine/scheduler.h"
#include "coroutine/coroutine.h"
#include "coroutine/sem.h"
#include "log/Logger.h"
#include "iostream"

using namespace coroutine;

int main()
{
    scheduler scheduler1;
    current_scheduler = &scheduler1;
    std::cout << "hello world0\n";

    sem_t sem;
    scheduler1.add([&sem]{
            std::cout << "hello world1.1\n";
            sem.wait();
            std::cout << "hello world1.2\n";
    });
    scheduler1.add([&sem]{
            std::cout << "hello world2.1\n";
            sem.wait();
            std::cout << "hello world2.2\n";
    });
    scheduler1.add([&sem]{
            std::cout << "hello world3.1\n";
            sem.wait();
            std::cout << "hello world3.2\n";
    });
    scheduler1.add([&sem]{
            std::cout << "hello world4.1\n";
            sem.signal();
            std::cout << "hello world4.2\n";
            sem.signal();
            std::cout << "hello world4.3\n";
            sem.signal();
            std::cout << "hello world4.4\n";
    });
    scheduler1.run();
    return 0;
}






