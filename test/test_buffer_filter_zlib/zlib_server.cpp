#include "event_interface.h"
#include "zlib_server.h"


using namespace std;

struct Status
{
	bool start = false;
	FILE *fp = 0;
	z_stream *p = 0;
	int recvNum = 0;
	int writeNum = 0;
	~Status()
	{
		if(fp)
			fclose(fp);
		if (p)
			inflateEnd(p);
		p = 0;
		delete p;
	}
	//string filename;
};


bufferevent_filter_result filter_in(evbuffer *s, evbuffer *d,ev_ssize_t limit,
                bufferevent_flush_mode mode,void *arg)
{
    Status *status = (Status *)arg;
    // 1.接收客户端发送的文件名
    if(!status->start)
    {
        char data[1024] = {0};
        int len = evbuffer_remove(s,data,sizeof(data) - 1);
        EVENT_DEBUG << "server recv " << data << endl;
        // 所有放进去的数据都会被  read_cb中重新读取 bufferevent_read
        evbuffer_add(d,data, len);
        return BEV_OK;
    }

    // 解压
    evbuffer_iovec v_in[1];
    // 读取数据 不清理缓缓冲区
    int n = evbuffer_peek(s, -1, NULL, v_in, 1);
    if(n <= 0)
    {
        EVENT_DEBUG << "evbuffer peek failed" << strerror(errno) << endl;
        return BEV_NEED_MORE;
    }
    // 解压上下文
    z_stream *p = status->p;
    // zlib 输入数据大小
    p->avail_in = v_in[0].iov_len;
    // zlib输入数据地址
    p->next_in = (Byte*)v_in[0].iov_base;

    // 申请输出空间大小
    evbuffer_iovec v_out[1];
    evbuffer_reserve_space(d, 4096, v_out,1);

    // zlib输出空间大小
    p->avail_out =v_out[0].iov_len;
    // zlib输出空间地址
    p->next_out  = (Byte*)v_out[0].iov_base;

    // 解压数据
    int ret = inflate(p, Z_SYNC_FLUSH);
    if(ret != Z_OK)
    {
        cerr << "inflate failed"<< endl;
    }

    // 解压用了多少数据从 source evbuffer中移除
    // p->avail_in 未处理的数据大小
    int nread = v_in[0].iov_len - p->avail_in;

    // 解压数据大小 传入des evbuffer
    // p->avail_out 剩余空间的大小
    int nwrite =  v_out[0].iov_len - p->avail_out;

    // 移除source evbuffer中数据
    evbuffer_drain(s, nread);
    EVENT_DEBUG << "filter out "<< endl;
    // 传入des_evbuffer
    v_out[0].iov_len = nwrite;
    evbuffer_commit_space(d, v_out, 1);
    EVENT_DEBUG <<"server nread = "<< nread << "nwrite = "<<nwrite<<endl;
    status->recvNum += nread;
    status->writeNum += nwrite;
    return BEV_OK;
}


void read_cb(bufferevent *bev, void *arg)
{
    Status *status = (Status *)arg;
    if(!status->start)
    {
        // 01接收文件名
        char data[1024] = {0};
        bufferevent_read(bev, data, sizeof(data) - 1);
        // status->filename = data;
        string out = "out/";
        out += data;
        // 打开写入文件
        status->fp =fopen(out.c_str(), "wb");
        if(!status->fp)
        {
            EVENT_DEBUG << "server open " << out <<  "failed" <<endl;
            return;
        }
        
        // 002 回复OK
        bufferevent_write(bev, "OK", 2);
        status->start = true;

        return;
    }

    do
    {
        // 写入文件
        char data[1024] = {0};
        int len = bufferevent_read(bev, data, sizeof(data) - 1);
        // 当buffervent中有数据的时候奖数据写到文件中
        if(len >= 0)
        {
            fwrite(data, 1, len, status->fp);
            fflush(status->fp);
        }
    } while (evbuffer_get_length(bufferevent_get_input(bev)) > 0);

}

void event_cb(bufferevent *bev, short events, void *arg)
{
    EVENT_DEBUG << "server event_cb" << events << endl;
    Status * status = (Status *)arg;
    if(events&BEV_EVENT_EOF)
    {
        EVENT_DEBUG << "server event BEV_EVENT_EOF"<< endl;
        EVENT_DEBUG << "server recv"<< status->recvNum;
        EVENT_DEBUG << "write："<<status->writeNum<< endl;
        delete status;
        bufferevent_free(bev);
    }
}


static void listen_cb(struct evconnlistener * e, evutil_socket_t s, struct sockaddr *a, int socklen, void *arg)
{
	EVENT_DEBUG << "listen_cb" << endl;
	event_base *base = (event_base *)arg;
    // 全部都设置了的效果就是后期只需要关闭bufferevent filter就会将socket 还有buffer event等都同时关闭掉
	//1 创建一个bufferevent 用来通信
    // 这里的s 是经过accept之后的sock fd
    // 为已经连接的sock创建一个bufferevent
    //  BEV_OPT_CLOSE_ON_FREE关闭bev的同时会关闭  sock
	bufferevent *bev = bufferevent_socket_new(base, s, BEV_OPT_CLOSE_ON_FREE);
	Status *status = new Status();
	status->p = new z_stream();
	inflateInit(status->p);

	//2 添加过滤 并设置输入回调
    // 关闭 bufferevent filter的同时也会关闭 bufferevent
	bufferevent *bev_filter = bufferevent_filter_new(bev,
		filter_in,//输入过滤函数
		0,//输出过滤
		BEV_OPT_CLOSE_ON_FREE,//关闭filter同时管理bufferevent
		0, //清理回调
		status	//传递参数
		);

	//3 设置回调 读取 事件（处理连接断开） 
	bufferevent_setcb(bev_filter, read_cb, 0, event_cb, status);
	bufferevent_enable(bev_filter, EV_READ | EV_WRITE);
}

void Server(event_base*base)
{
	EVENT_DEBUG << "begin Server" << endl;
	//监听端口
    //socket ，bind，listen 绑定事件
	sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(SERVER_PORT);

	evconnlistener *ev = evconnlistener_new_bind(base,	// libevent的上下文
		listen_cb,					//接收到连接的回调函数
		base,						//回调函数获取的参数 arg
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,//地址重用，evconnlistener关闭同时关闭socket
		10,							//连接队列大小，对应listen函数
		(sockaddr*)&sin,							//绑定的地址和端口
		sizeof(sin)
	);

}
