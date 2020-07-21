#include "event_interface.h"
#include "zlib_client.h"

using namespace std;

#define FILEPATH "001.txt"
// #define FILEPATH "001.bmp"

// 通过void *类型的参数传递整个函数运行的状态 有利于在全局变量满天飞的函数中实现函数可重入

struct  ClientStatus
{
    FILE *fp = 0;
    bool end = false;
    // 是否可以开始发送文件
    bool startSend = false;
    int readNum = 0;
    int sendNum = 0;
    z_stream *z_output = 0;
    ~ClientStatus()
    {
        if(fp)
            fclose(fp);
        fp = NULL;
        if(z_output)
            deflateEnd(z_output);
        delete z_output;
        z_output = NULL;
    }
};

bufferevent_filter_result filter_out(evbuffer *s, evbuffer *d,ev_ssize_t limit, bufferevent_flush_mode mode, void * arg)
{
    ClientStatus *sta = (ClientStatus *)arg;

    // 压缩文件
    // 发送文件消息 001 去掉
    if(!sta->startSend)
    { 
        char data[1024] = {0};
        // 从sock中取数据 并将数据直接放到对应的eventbuffer中
        // 在没有发送之前一直是按照明文进行发送
        int len = evbuffer_remove(s, data, sizeof(data));
        // 将数据放到bufferevent中
        evbuffer_add(d, data,len);
        return BEV_OK;
    }
    // 打开压缩文件
    // 取出buffer中数据的引用
    evbuffer_iovec v_in[1];
    // 将数据的指针拿出来
    // 这里使用peek是有讲究的，因为下面的压缩函数一次要锁多少数据是不确定的
    // 这里使用peek将数据不取出，最后使用多少在drain多少 evbuffer_drain(s, nread);
    // 从而可以达到压缩多少就去除多少数据，没有压缩的数据下次还是会触发filter_out函数
    int n = evbuffer_peek(s, -1, 0, v_in, 1);
    if(n <= 0)
    {
        // 调用write回调清理空间
        if(sta->end)
        {
            return BEV_OK;
        }
        // 没有数据 BEV_NEED_MORE 不回进入写入回调
        return BEV_NEED_MORE;
    }
    // zlib上下文
    z_stream *p= sta->z_output;
    if(!p)
    {
        EVENT_DEBUG << "BEV_ERROR" << endl;
        return BEV_ERROR;
    }
    // zlib输入数据大小
    p->avail_in = v_in[0].iov_len;
    // zlib输入数据地址
    p->next_in = (Bytef*)v_in[0].iov_base;

    // 申请输出空间大小
    // 没有使用new因为 new出来的还要传递给buffervent 还牵涉到释放等一系列操作
    // 直接在bufferevent里面在获取一块使用
    evbuffer_iovec v_out[1];
    evbuffer_reserve_space(d, 4096, v_out, 1);
    // zlib输出空间大小
    p->avail_out = v_out[0].iov_len;
    // zlib输出空间地址
    p->next_out = (Bytef*)v_out[0].iov_base;
    // zlib压缩
    int ret = deflate(p,Z_SYNC_FLUSH);
    if(ret != Z_OK)
    {
        cerr << "deflate failed!" << endl;
    }
    // 压缩用了多少数据，从soutce evbuffer中移除
    // p->avail_in 未处理的数据大小
    int nread = v_in[0].iov_len - p->avail_in;
    // 压缩后数据大小 传入des evbuffer
    // p->avail_out 剩余空间大小
    int nwrite = v_out[0].iov_len - p->avail_out;
    // 移除 source evbuffer中数据
    // 没有移除的数据下次进来还会在这里
    evbuffer_drain(s, nread);
   
   
    EVENT_DEBUG << "filter out "<<endl;
    // 传入des evbuffer
    v_out[0].iov_len =nwrite;
    // 压缩之后的数据大小，真正的需要放到发送缓存区中发送给对方的数据
    evbuffer_commit_space(d, v_out, 1);
    EVENT_DEBUG << "clietn nread = "<<nread << "," <<"nwrite = "<<nwrite<< endl;
    // 堆读取的数据量 和压缩的之后的总的的数据大小进行统计
    sta->readNum += nread;
    sta->sendNum += nwrite;
    return BEV_OK;

}



void client_read_cb(bufferevent *bev, void *arg)
{
	ClientStatus *sta = (ClientStatus *)arg;
	//002 接收服务端发送的OK回复
	char data[1024] = { 0 };
	int len = bufferevent_read(bev, data, sizeof(data) - 1);
	if (strcmp(data, "OK") == 0)
	{
		EVENT_DEBUG << "client read cb"<< data << endl;
        // 同时可以开始发送文件了
		sta->startSend = true;
		//开始发送文件,触发写入回调 也就是通知当前是可以吸入文件 可以开始写入
        // Triggers bufferevent data callbacks. 启动触发启动回调函数
		bufferevent_trigger(bev, EV_WRITE, 0);
	}
	else
	{
		bufferevent_free(bev);
	}
	EVENT_DEBUG << "client_read_cb " << len << endl;
}

void client_write_cb(bufferevent *bev, void *arg)
{
    ClientStatus *s =(ClientStatus *)arg;
    FILE *fp = s->fp;
    // 判断什么时候清理资源
    if(s->end)
    {
        // 判断缓冲区是否有数据，要是有数据就刷新
        // 获取顾虑器绑定的buffer
        bufferevent *bevf = bufferevent_get_underlying(bev);
        // 获取输出缓冲区的大小
        evbuffer *evb = bufferevent_get_output(bevf);
        int len = evbuffer_get_length(evb);
        EVENT_DEBUG << "evbuffer get length = " << len << endl;
        // 缓存区没有数据
        if(len <= 0)
        {
            EVENT_DEBUG << "Client read"<<s->readNum << "send"<<s->sendNum << endl;
            // 立即清理如果缓冲区有数据
            bufferevent_free(bev);
            delete s;
            return ;
        }
        // 要是缓存区是有数据的  那就需要刷新缓冲区 否者函数不回在进来
        // 直到缓存区真正的为0的时候才对缓冲区进行清理
        bufferevent_flush(bev, EV_WRITE, BEV_FINISHED);
        return ;
    }

    if(!fp) return;
    EVENT_DEBUG << "client  write cb " << endl;
    // 读取文件
    char data[1024] = {0};
    int len = fread(data,1,sizeof(data), fp);
    if(len <=0)
    {
        s->end = true;
        // 刷新缓冲区  函数会再次进来
        bufferevent_flush(bev, EV_WRITE, BEV_FINISHED);
        return;
    }


    // 发送文件
    bufferevent_write(bev,data, len);
}

void client_event_cb(bufferevent *bev, short events, void *arg)
{
    EVENT_DEBUG << "client event cb"<<events << endl;
    if(events&BEV_EVENT_CONNECTED)
    {
        EVENT_DEBUG << "BEV_EVENT_CONNECTED" << endl;
        // 发送文件名
        bufferevent_write(bev, FILEPATH, strlen(FILEPATH));
        FILE *fp = fopen(FILEPATH, "rb");
        if(!fp)
        {
            EVENT_DEBUG << "open file" << FILEPATH << "failed" << endl;
            return;
        }
        ClientStatus *s  = new ClientStatus();
        s->fp = fp;

        // 初始化zlib上下文 默认压缩
        s->z_output = new z_stream();
        deflateInit(s->z_output, Z_DEFAULT_COMPRESSION);

        bufferevent *bev_filter = bufferevent_filter_new(bev, 0, filter_out,
                    BEV_OPT_CLOSE_ON_FREE|BEV_OPT_DEFER_CALLBACKS, 0, s);


        // 设置读取、写入和事件的回调
        bufferevent_setcb(bev_filter, client_read_cb, client_write_cb, client_event_cb, s);
        bufferevent_enable(bev_filter, EV_READ|EV_WRITE);

    }

}


void Client(event_base *base)
{
    EVENT_DEBUG << "begin Client" << endl;
    // 连接服务器
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(5001);
    evutil_inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr.s_addr);
    bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

    // 只绑定事件回调 用来确认连接成功
    bufferevent_enable(bev, EV_READ|EV_WRITE);
    
    bufferevent_setcb(bev, 0,0,client_event_cb,0);

    // 当连接事件发生时会调用上述回调函数
    bufferevent_socket_connect(bev, (sockaddr*)&sin, sizeof(sin));
}
