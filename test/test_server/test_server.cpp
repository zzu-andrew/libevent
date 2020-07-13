/***
 *  创建event base
 * */
#include <event2/event.h>
#include <iostream>
#include <signal.h>
#include <event2/listener.h>
#include <string.h>
#include "event_interface.h"

using namespace std;

/**
   A callback that we invoke when a listener has a new connection.

   @param listener The evconnlistener
   @param fd The new file descriptor
   @param addr The source address of the connection
   @param socklen The length of addr
   @param user_arg the pointer passed to evconnlistener_new()
 */
void listen_cb(struct evconnlistener * evConnListener, evutil_socket_t evUtilSockFd, struct sockaddr * sockAddr, int socklen, void *data)
{
	cout << "listen cb is called" << endl;
}

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

	//监听端口
	//socket, bind, listen
	sockaddr_in sockIn;
	memset(&sockIn, 0, sizeof(sockIn));
	sockIn.sin_family = AF_INET;
	sockIn.sin_port = htons(SERVER_PORT);
	/* 地址没有指定因为对sockIn进行了了memset，地址赋值为0代表着可以为任意可以用的地址 */

	struct evconnlistener *pEvListener = evconnlistener_new_bind(base, /* libevent的上下文 */
				listen_cb, /* 接收到连接的回调 */
				base, /* 回调函数参数 */
				LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, /* 地址重用，evconnlistenner关闭同时关闭socket */
				10, /* 连接队列的大小，对应的listen函数 */
				(sockaddr *)&sockIn,   /* 绑定地址和端口 */
				sizeof(sockIn)
				);

	//事件分发处理
	if(base)
		event_base_dispatch(base);
	
	if(pEvListener)
		evconnlistener_free(pEvListener);

	if(base)
	event_base_free(base);
	return 0;
}
