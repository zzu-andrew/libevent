/***
 *  创建event base
 * */
#include <event2/event.h>
#include <iostream>
#include <signal.h>
#include <event2/listener.h>
#include <string.h>
#include "event_interface.h"
#include "zlib_server.h"
#include "zlib_client.h"

using namespace std;

int main(int argc, char *argv[])
{
    //1. 忽略管道信号,发送数据给已关闭的socket
	//一些socket程序莫名宕掉的原因
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	{
		cout << "ignal pipe signal" << endl;
	}

    std::cout << "test libevent!\n"; 
	//创建libevent的上下文
	event_base * base = event_base_new();
	if (!base)
	{
		cout << "event_base_new failed." << endl;
		return -1;
	}
	else
	{
		cout << "event_base_new success!" << endl;
	}


	Server(base);

	Client(base);


	//事件分发处理
	if(base)
		event_base_dispatch(base);

	if(base)
		event_base_free(base);
	return 0;
}
