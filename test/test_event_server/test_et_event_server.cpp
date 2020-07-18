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

// 断开连接  超时都会进入
void client_cb(evutil_socket_t s, short w, void *arg)
{
    // 边缘触发 只进来一次，就算不处理也只进来一次
    
   // cout << "." << endl;
    char buff[5] = {0};
    int len = 0;
    event *ev = (event *) arg;
    // 判断超时事件
    if(w&EV_TIMEOUT)
    {
        cout << "ev timeout" << endl;
        goto client_clean;
        
    }
    
    len = recv(s,buff,sizeof(buff) -1, 0);
    if(len > 0)
    {
        cout << buff << endl;
        send(s, "OK\n", sizeof("OK\n"), 0);
        return;
    }
    else
    {
        cout << "." << endl;
        goto client_clean;
    }

client_clean:
    event_free(ev);
    evutil_closesocket(s);
    return;
}

/**
   A callback that we invoke when a listener has a new connection.

   @param listener The evconnlistener
   @param fd The new file descriptor
   @param addr The source address of the connection
   @param socklen The length of addr
   @param user_arg the pointer passed to evconnlistener_new()
 */
//默认是水平触发，只要不处理就会一直
void listen_cb(evutil_socket_t evUtilSockFd, short name,void *data)
{
	cout << "listen cb is called" << name<< endl;
    sockaddr_in sin;
    socklen_t size = sizeof(sin);
    
    // 读取连接信息
    evutil_socket_t client = accept(evUtilSockFd,(sockaddr *)&sin, &size);
    char szIpAddr[16] = {0};
    evutil_inet_ntop(AF_INET, &sin.sin_addr, szIpAddr, sizeof(szIpAddr));
    cout << "client ip is "<< szIpAddr<< endl;
    // 客户端数据读取事件
    event_base *base = (event_base *)data;
    event *ev = event_new(base,client,EV_READ|EV_PERSIST|EV_ET,client_cb, event_self_cbarg());
    // 定超时时间是10s若是在10秒连接不发送数据，就会有超时事件触发
    timeval t ={10,0};
    event_add(ev, &t);

}

int main(int argc, char *argv[])
{
    //1. 忽略管道信号,发送数据给已关闭的socket
	//一些socket程序莫名宕掉的原因
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	{
		cout << "ignal pipe signal" << endl;
	}

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

    cout << "test event sserver" << endl;
    // 创建socket
    evutil_socket_t sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockFd <= 0)
    {
        cout << "socket error: "<<strerror(errno) << endl;
        return OPEN_FAILED;
    }
    // 设置地址复用和非阻塞
    evutil_make_socket_nonblocking(sockFd);
    evutil_make_listen_socket_reuseable(sockFd);


    // 绑定端口和地址
    sockaddr_in sin;
    memset(&sin,0,sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(SERVER_PORT);
    // 加 ::是为了调用全局函数使用，不加的话，C++中要是有部分私有库实现了socket就会掉私有库中的函数
    int ret = ::bind(sockFd, (sockaddr *)&sin, sizeof(sin));
    if(ret != 0)
    {
        cerr << "bind error" << strerror(errno) << endl;
        return BIND_FAILED;
    }
    // 开始监听 监听的队列中最多放置10个，并不是说明只能建立10个连接，只是同时来连接
    // 没有待连接的池子中有10个
    listen(sockFd, 10);
    // 开始接收事件，use eadge trigger by set EV_ET
    event *ev = event_new(base, sockFd, EV_READ|EV_PERSIST, listen_cb, base);
    event_add(ev, 0);

	if(base)
		event_base_dispatch(base);
	
	if(base)
	    event_base_free(base);
	return 0;
}
