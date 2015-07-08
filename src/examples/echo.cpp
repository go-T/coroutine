/*
 * echo.cpp
 *
 *  Created on: 2015年7月8日
 *      Author: zhusheng
 */

#include "couv/scheduler.h"
#include "couv/uvpp.h"

using namespace couv;
using namespace couv::uvpp;

int main()
{
    //Logger::logger().setDevice(NULL);

    scheduler_t scheduler;

    loop_t loop;
    tcp_t server(&loop);
    server.accept([](tcp_t* client, int status){
        client->read([](tcp_t* client, char* buf, ssize_t len){
            client->write(buf, len);
        });
    });

    scheduler.add([&loop]{
        loop.run();
    });

    scheduler.run();
    return 0;
}


