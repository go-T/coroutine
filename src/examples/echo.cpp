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

    uvscheduler_t scheduler;
    tcp_t server;

    go {
        server.listen(9527);
        server.accept<tcp_t>([&](const std::shared_ptr<tcp_t>& new_conn, int status){
            logDebug("new_conn %p", new_conn.get());
            goo[&, new_conn]{
                std::shared_ptr<tcp_t> client(new_conn);
                client->read([&](tcp_t* client, char* buf, ssize_t len){
                    buf[len] = 0;
                    if (strncasecmp(buf, "quit\r\n", len) == 0) {
                        client->write("bye", 3, [](tcp_t* client, int len){
                            client->shutdown();
                        });
                    } else if(strncasecmp(buf, "exit\r\n", len) == 0) {
                        client->write("byebye", 6, [&](tcp_t* client, int len){
                            client->close();
                            scheduler.stop();
                        });
                    } else {
                        client->write(buf, len);
                    }
                });
            };
        });
    };

    scheduler.run();
    return 0;
}
