/***
 *  创建event base
 * */
#include "event_interface.h"

using namespace std;

bufferevent_filter_result filter_in(evbuffer *s, evbuffer *d,ev_ssize_t limit, bufferevent_flush_mode mode, void *arg)
{
	cout << "filter in"<< endl;
	char data[1024] = {0};

	// 读取并清理原数据
	int len = evbuffer_remove(s, data,sizeof(data) - 1);

	// 将所有的字母转换为大写
	for(int i = 0; i<len;++i)
	{
		data[i] = toupper(data[i]);
	}

	evbuffer_add(d, data,len);
	return BEV_OK;
}


// 输出回调函数
bufferevent_filter_result filter_out(evbuffer *s, evbuffer *d,ev_ssize_t limit, bufferevent_flush_mode mode, void *arg)
{
	cout << "filter out" << endl;
	char data[1024] = {0};

	// 读取并清理数据
	int len = evbuffer_remove(s,data,sizeof(data) - 1);
	string str = "";
	str += "---------------------\n";
	str += data;
	str += "=====================\n";
	evbuffer_add(d,str.c_str(),str.size());


	return BEV_OK;
}

void read_cb(bufferevent *bev, void *arg)
{
	cout << "read_cb" << endl;

	char data[1024] = {0};
	// 这里读取的数据其实都是经过filter处理的数据
	// 读取过滤器中的数据
	int len = bufferevent_read(bev, data,sizeof(data) - 1);
	cout << data << endl;


	cout << "read write begin: " << data<<endl;
	// 回复客户消息，经输出过滤的消息
	// 该函数回等输出过滤函数调用之后才会调用真正的向对方输出
	bufferevent_write(bev,data,len);
	cout << "read write end: " << data<<endl;
}


void write_cb(bufferevent *bev, void *arg)
{
	cout << "write cb"<<endl;
}

// 当对方连接退出的时候回调用该函数
void event_cb(bufferevent *bev, short events, void *arg)
{
	cout << "event cb" << endl;
}


/**
   A callback that we invoke when a listener has a new connection.

   @param listener The evconnlistener
   @param fd The new file descriptor
   @param addr The source address of the connection
   @param socklen The length of addr
   @param user_arg the pointer passed to evconnlistener_new()
 */
void listen_cb(struct evconnlistener * evConnListener, evutil_socket_t evUtilSockFd, struct sockaddr * sockAddr, int socklen, void *arg)
{
	cout << "listen cb is called" << endl;
	event_base *base = (event_base *)arg;
	// 创建buffevent
	bufferevent *bev = bufferevent_socket_new(base, evUtilSockFd, BEV_OPT_CLOSE_ON_FREE);
	// 绑定bufferevent filter
	bufferevent *bev_filter = bufferevent_filter_new(bev,
				filter_in, //输入过滤函数
				filter_out, //输出过滤函数
				BEV_OPT_CLOSE_ON_FREE, //关闭filter的同时关闭Bufferevent
				0, // 清理的回调函数
				0 // 传递给回调函数的参数
				);  
	// 设置bufferevent的回调函数
	bufferevent_setcb(bev_filter, read_cb, write_cb, event_cb,NULL);
	// 开启读写权限
	bufferevent_enable(bev_filter, EV_READ|EV_WRITE);

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
		return ERROR;
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
