#include "event_interface.h"

using namespace std;

#define WEBROOT "." 
#define DEFAULTINDEX "index.html"

void http_cb(evhttp_request *request, void *arg)
{
	EVENT_DEBUG << "http cb" << endl;

	// 获取浏览器请求消息
	// uri
	const char *uri = evhttp_request_get_uri(request);
	EVENT_DEBUG << "URI:" << uri << endl;

	// 请求类型 GET POST
	string cmdType;
	switch(evhttp_request_get_command(request))
	{
		case EVHTTP_REQ_CONNECT:
			cmdType = "GET";
			break;
		case EVHTTP_REQ_POST:
			cmdType = "POST";
			break;
	}
	EVENT_DEBUG << "cmdType:" << cmdType << endl;

	// 消息报头
	evkeyvalq *headers = evhttp_request_get_input_headers(request);
	EVENT_DEBUG << "======= headers =======" << endl;

	for(evkeyval *p = headers->tqh_first; p != NULL; p = p->next.tqe_next)
	{
		EVENT_DEBUG << p->key << ":" << p->value << endl;
	}

	// 请求正文 (GET 为空， POST有表单信息)
	evbuffer *inbuf = evhttp_request_get_input_buffer(request);

	char buff[1024] = {0};
	EVENT_DEBUG << "======= Input data (POST) =======" << endl;

	while(evbuffer_get_length(inbuf))
	{
		int n = evbuffer_remove(inbuf, buff, sizeof(buff) - 1);
		if(n >0)
		{
			buff[n] = '\0';
			EVENT_DEBUG << buff << endl;
		}
	} 

	// 回复浏览器
	// 状态行  消息报头 响应正文 HTTP_NOTFOUND HTTP_INTERNAL

	// 分析出请求的文件uri
	// 设置根目录 WEBROOT
	string filepath = WEBROOT;
	filepath += uri;
	if(strcmp(uri, "/") == 0)
	{
		// 默认加入首页文件
		filepath += DEFAULTINDEX;
	}

	// 消息报头  需要回复浏览器的消息报头
	evkeyvalq *outhead = evhttp_request_get_output_headers(request);
	// 要支持图片  js  css 下载zip文件
	// 获取文件后缀
	// ./root/index.html
	int pos = filepath.rfind('.');
	string postfix = filepath.substr(pos+1, filepath.size() - (pos +1));
	if(postfix == "jpg" || postfix == "gif" || postfix == "png")
	{
		string tmp = "image/" + postfix;
		evhttp_add_header(outhead, "Content-Type", tmp.c_str());
	}
	else if(postfix == "zip")
	{
		evhttp_add_header(outhead, "Content-Type", "application/zip");
	}
	else if(postfix == "html")
	{
		evhttp_add_header(outhead, "Content-Type", "text/html;charset=UTF8");
	}
	else if(postfix == "css")
	{
		evhttp_add_header(outhead, "Content-Type", "text/css");
	}

	// 读取html文件返回正文
	FILE *fp = fopen(filepath.c_str(), "rb");
	if(!fp)
	{
		evhttp_send_reply(request, HTTP_NOTFOUND, "", 0);
		return;
	}

	// 需要回复浏览器的buff也就是消息内容
	evbuffer  *outbuff = evhttp_request_get_output_buffer(request);
	for(;;)
	{
		int len = fread(buff, 1, sizeof(buff), fp);
		if(len <= 0)break;
		evbuffer_add(outbuff, buff, len);
	}
	fclose(fp);
	evhttp_send_reply(request, HTTP_OK, "", outbuff);
}


int main(int argc, char const *argv[])
{
	// 或略管道信号，发送给已关闭的socket
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	{
		EVENT_DEBUG << "set signal failed." << endl;
		return ERROR;
	}
	EVENT_DEBUG << "test eserver" << endl;
	// 创建libevent 的上下文
	event_base * base = event_base_new();
	if(base)
	{
		EVENT_DEBUG << "event base new success" << endl;
	}
	else
	{
		EVENT_DEBUG << "event base new failed." << endl;
		return ERROR;
	}
	// http服务器
	// 创建http上下文
	evhttp *evh = evhttp_new(base);
	// 绑定端口和IP
	if(evhttp_bind_socket(evh, "0.0.0.0", 8080) != 0)
	{
		EVENT_DEBUG<<"evhttp bind socket failed" << endl;
		return ERROR;
	}

	// 设置回调函数
	evhttp_set_gencb(evh, http_cb, 0);
	// 事件分发处理
	if(base)
		event_base_dispatch(base);
	if(base)
		event_base_free(base);
	if(evh)
		evhttp_free(evh);
	
















	return 0;
}









