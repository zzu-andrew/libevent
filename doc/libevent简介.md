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

```c++
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


































































