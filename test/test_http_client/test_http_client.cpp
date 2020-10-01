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

void http_client_cb(struct evhttp_request *req, void *ctx)
{
    EVENT_DEBUG << "http client cb" << endl;

    event_base * base = (event_base*) ctx;

    // 服务端响应错误
    if(req == NULL)
    {
        int errcode = EVUTIL_SOCKET_ERROR();
        EVENT_DEBUG << "socket error" << evutil_socket_error_to_string(errcode) << endl;
        return ;
    }

    // 获取path
    const char * path = evhttp_request_get_uri(req);
    EVENT_DEBUG << "request path is " << path <<endl;
    string filepath = ".";
    filepath += path;
    EVENT_DEBUG << "filepath is  "<< filepath << endl;

    //通过http协议实现实现文件下载到本地
    // 如果路径中有目录 需要分析出目录 并创建
    FILE *fp = fopen(filepath.c_str(), "wb");
    if(!fp)
    {
        EVENT_DEBUG << "opend file" << filepath << "failed!" << endl;
    }

    // 200 404 ....
    EVENT_DEBUG << "Response :" << evhttp_request_get_response_code_line(req) << endl;; //200

    EVENT_DEBUG << evhttp_request_get_response_code_line(req) << endl; //OK
    char buff[1024] =  {0};
    evbuffer *input = evhttp_request_get_input_buffer(req);
    for(;;)
    {
        int len = evbuffer_remove(input, buff, sizeof(buff) - 1);
        if(len <= 0)break;;;
        buff[len] = 0;
        if(!fp)
            continue;
        // 将buff中的内容写到 fp中
        fwrite(buff,1, len, fp);
       // EVENT_DEBUG << buff << endl;
    }

    if(fp)
        fclose(fp);
    //能够退出时因为有退出
    event_base_loopbreak(base);
}



int TestGetHttp(void)
{
    // 创建libevent的上下文
    event_base * base = event_base_new();
    if(base)
    {
        EVENT_DEBUG << "event base new success!" << endl;
    }

    // 生成请求信息 GET
    //  ?id=1 ? 之后给的id其实就是一个变量 这个变量会传递给对方的服务器
    string http_url = "http://ffmpeg.club/index.html?id=1";
   http_url = "http://ffmpeg.club/lesson_img/101.jpg";
    // 分析url地址
    evhttp_uri *uri = evhttp_uri_parse(http_url.c_str());
    // http https
    const char *scheme = evhttp_uri_get_scheme(uri);
    if(!scheme)
    {
        EVENT_DEBUG << "scheme is null" << endl;
        return ERROR; 
    }

    EVENT_DEBUG << "scheme is " << scheme << endl;
    int port = evhttp_uri_get_port(uri);
    if(port < 0)
    {
        if(strcmp(scheme, "http") == 0)
        {
            port = 80;
        }
    }
    EVENT_DEBUG << "port is " << port << endl;
    // host ffmpeg
    const char *host = evhttp_uri_get_host(uri);
    if(!host)
    {
        EVENT_DEBUG << "host is null" << endl;
        return ERROR;
    }

    EVENT_DEBUG << "host is " << host << endl;
    const char *path = evhttp_uri_get_path(uri);
    if(!path || strlen(path) == 0)
    {
        path = "/";
    }
    if(path)
    {
        EVENT_DEBUG << "path is " << path << endl;
    }

    // id =1 ？后面的内容id =1
    const char *query = evhttp_uri_get_query(uri);
    if(query)
    {
        EVENT_DEBUG << "query is " << query << endl;
    }
    else
    {
        EVENT_DEBUG << "query is NULL" << endl;
    }
    // bufferevent 连接http服务器
    // fd设置成-1需要后期调用 
    bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    evhttp_connection *evcon = evhttp_connection_base_bufferevent_new(base, NULL,bev, host,port);

    // http client 请求 回调函数设置
    evhttp_request *req = evhttp_request_new(http_client_cb, base);
    // 设置请求的head 消息报头信息
    evkeyvalq *output_headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(output_headers, "HOST", host);

    // 发起请求
    evhttp_make_request(evcon, req, EVHTTP_REQ_GET, path);

    // 事件分发处理
    if(base)
        event_base_dispatch(base);
    if(uri)
        evhttp_uri_free(uri);
    if(evcon)
        evhttp_connection_free(evcon);
    if(base)
        event_base_free(base);
    
    return OK;
}

/**
 * @brief TestPostHttptest http post
 * 
 * @return int 
 */
int TestPostHttp(void)
{
    // 创建libevent的上下文
    event_base * base = event_base_new();
    if(base)
    {
        EVENT_DEBUG<< "event base new success!" << endl;
    }
    // 生成请求信息 POST
    string http_url = "http://127.0.0.1:8080/index.html";
    // 
    // 分析utl地址
    //  uri
    evhttp_uri *uri = evhttp_uri_parse(http_url.c_str());
    // http https
    const char *scheme = evhttp_uri_get_scheme(uri);
    if(!scheme)
    {
        EVENT_DEBUG << "shceme is null" << endl;
        return ERROR;
    }

    EVENT_DEBUG << "scheme is " << scheme << endl;
    int port = evhttp_uri_get_port(uri);
    if(port < 0)
    {
        if(strcmp(scheme, "http") == 0)
        {   
            port = 80;
        }
    } 

    EVENT_DEBUG << "port is " << port << endl;
    //  host ffmpeg.club

    const char *host = evhttp_uri_get_host(uri);
    if(!host)
    {
        EVENT_DEBUG << "host is null "<< endl;
        return ERROR;
    }
    EVENT_DEBUG <<"host is "<< host << endl;
    const char *path = evhttp_uri_get_path(uri);
    if(!path || strlen(path) == 0)
    {
        path = "/";
    }
    if(path)
    {
        EVENT_DEBUG << "path is " << path << endl;
    }
    // ?id 后面的内容 id为1
    const char *query = evhttp_uri_get_query(uri);
    if(query)
    {
        EVENT_DEBUG << "query is " << query << endl;
    }
    else
    {
        EVENT_DEBUG << "query is NULL" << endl;
    }

    // bufferevent 连接http服务器
    bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    evhttp_connection *evcon = evhttp_connection_base_bufferevent_new(base,
        NULL,bev, host, port);
    //  http client 请求回调函数设置
    evhttp_request *req = evhttp_request_new(http_client_cb, base);    
    // 设置请求的head 消息报头
    evkeyvalq*output_headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(output_headers, "HOST", host);

    // 发送POST数据
    evbuffer *output = evhttp_request_get_output_buffer(req);
    evbuffer_add_printf(output, "xcj=%d&b=%d\n", 1,2);

    // 发起请求
    evhttp_make_request(evcon, req, EVHTTP_REQ_POST, path);
    
	//事件分发处理
	
    if(base)
        event_base_dispatch(base);        
	if (uri) evhttp_uri_free(uri);
	if (evcon) evhttp_connection_free(evcon);
	if (base)
		event_base_free(base);


    return OK;

}


int main(int argc, char *argv[])
{
    //1. 忽略管道信号,发送数据给已关闭的socket
	//一些socket程序莫名宕掉的原因
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	{
		cout << "ignal pipe signal" << endl;
	}

    EVENT_DEBUG << "test http client!" << endl;
	TestGetHttp();
    TestPostHttp();
}
