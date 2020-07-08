# <font size=32 color=red>`Libevent` </font>简介

`Libevent`是一款事件驱动的网络开发包，由于采用`C`语言开发体积小巧，跨平台，速度极快。大量开源项目使用了`Libevent`比如谷歌的浏览器和分布式的高速缓存系统`memcached`。`libevent`支持`kqueue`,`select`,`poll`,`epoll`,`iocp`。内部事件机制完全独立于公开事件`API`，`libevent`支持跨平台可以在`Linux`，`*BSD`，`MacOSX`,`Solaris`,`Windows`等平台上编译。

**学习条件**：具有一定的`C/C++`基础，熟悉`Linux`



## 环境搭建

- [x] 配置`zlib`库


```bash
# 1. 解压zlib 1.2.11
tar xvf zlib-1.2.11.tar.gz
# 2. 编译
cd zlib-1.2.11/
./configure
make
make install
```

- [x] 配置`openssl`库

```bash
# 1. 解压openssl-1.1.1.tar.gz
tar xvf openssl-1.1.1.tar.gz
# 2. 编译
cd openssl-1.1.1/
./configure
make
make install
```

- [x] 配置`libevent`环境

```bash
# 1.加压liebevent 2.1.8
unzip libevent-master.zip
# 2. 编译
cd libevent-master/
./autogen.sh
./configure
make
make install
# 3.将动态路来连接到 /usr/lib 下或者执行以下  ldconfig
sudo ln -s /usr/local/lib/libevent-2.2.so.1 /usr/lib/libevent-2.2.so.1
```



## 实战实例



### 创建`event_base`

仅仅实现创建上下文

```c
/***
 *  创建event base
 * */

#include <event2/event.h>
#include <iostream>
using namespace std;
int main()
{

    std::cout << "test libevent!\n"; 
	//创建libevent的上下文
	event_base * base = event_base_new();
	if (base)
	{
		cout << "event_base_new success!" << endl;
	}
	return 0;
}

```



### 创建`test_server`

`test_server`中说明了如何使用`libevent`创建一个`socket`监听

`evconnlistener_new_bind`一个接口完成了`socket`的创建，绑定和监听。



```c
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
```


































































