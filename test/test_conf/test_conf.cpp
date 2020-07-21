#include <event2/event.h>
#include <event2/thread.h>
#include <event2/listener.h>
#include <signal.h>
#include <iostream>
#include <string.h>
#include "event_interface.h"

using namespace std;

int main()
{
	//忽略管道信号，发送数据给已关闭的socket
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		return 1;

	//创建配置上下文
	//该对象创建出来是为了配置事件的规则信息使用的
	event_config *config = event_config_new();
	//显示支持的网络模式
	const char **methods = 	event_get_supported_methods();
	cout << "support methods " << endl;
	for(int i = 0; methods[i] != NULL; i++)
	{
		cout << methods[i] << endl;
	}
	//设置特征，确认特征时候生效
	//这个features在linux中设置没有效果，因为linux中本来就是支持ET模式的，边缘触发模式
	// 设置了EV_FEATURE_FDS其他特征嗯就无法设置
	//也就是所支持了EV_FEATURE_FDS  其他的特征都是无法支持的
	// event_config_require_features(config, EV_FEATURE_FDS); //不支持epoll
	// event_config_require_features(config, EV_FEATURE_FDS); //设置网络模型使用select
	event_config_avoid_method(config, "epoll");
	event_config_avoid_method(config, "poll");
	/*
	int ret = event_config_require_features(config, EV_FEATURE_ET|EV_FEATURE_EARLY_CLOSE);
	if(OK != ret)
	{
		cerr << "event config require features failed." << endl;
		return ERROR; 
	}
	*/
	//初始化libevent上下文
	//这个上下的茶创建是随着配置信息生成的
	event_base *base =  event_base_new_with_config(config);

	//config一旦配置好就不需要在使用了
	//也就是所所有的配置信息需要在这之前进行销毁
	event_config_free(config);

	if(!base)
	{
		cerr << "event base new with config failed!" << endl;
		//首次失败就创建一个base取默认值，若是再次失败就返回失败
		base = event_base_new();
		if(!base)
		{
			cerr << "event base new failed." << endl;	
			return ERROR;
		}
	}
	else
	{
		//获取当前网路模型
		const char * pBaseName = NULL;
		pBaseName = event_base_get_method(base);
		cout << "suport method:" << pBaseName << endl;


		//确认特征那些生效
		int f = event_base_get_features(base);
		if(f&EV_FEATURE_ET)
		{
			cout << "EV_FEATURE_ET events are supported." << endl;
		}
		else
		{
			cout << "EV_FEATURE_ET events are not supports." << endl;
		}

		if(f&EV_FEATURE_O1)
		{
			cout << "EV_FEATURE_O1 events are supported." << endl;
		}
		else
		{
			cout << "EV_FEATURE_O1 events are not supports." << endl;
		}

		if(f&EV_FEATURE_FDS)
		{
			cout << "EV_FEATURE_FDS events are supported." << endl;
		}
		else
		{
			cout << "EV_FEATURE_FDS events are not supports." << endl;
		}

		if(f&EV_FEATURE_EARLY_CLOSE)
		{
			cout << "EV_FEATURE_EARLY_CLOSE events are supported." << endl;
		}
		else
		{
			cout << "EV_FEATURE_EARLY_CLOSE events are not supports." << endl;
		}
		cout << "event base new with config sucess" << endl;
		event_base_free(base);
	}

	EVENT_DEBUG << "hekkolooolololol"<< endl;

	return 0;
}
