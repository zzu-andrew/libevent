#include <iostream>
#include <event2/event.h>
#include <signal.h>
#include "event_interface.h"

using namespace std;

//sock 文件描述符，which事件类型 arg 传递的参数
static void Ctrl_C(int sock, short which, void *arg)
{
    cout << "Ctrl + C" << " 添加 " << endl;
}

// Kill函数
static void Kill(int sock, short which, void *arg)
{
    cout << "Kill"<< endl;
    struct event *ev = (struct event *)arg;
    // 如果处于非待决状态
    if(!evsignal_pending(ev, NULL))
    {
        event_del(ev);
        event_add(ev, NULL);
    }

}

int main(int argc, const char** argv) {

    event_base *base = event_base_new();

    //添加CTRL+c 信号事件 处于np pending
    //隐藏的状态 EV_SIGNAL|EV_PERSIST
    struct event *csig = evsignal_new(base, SIGINT, Ctrl_C, base);
    if(!csig)
    {
        cerr << "SIGINT evsignal_new failed!" << endl;
        return ERROR;
    }

    //添加事件到pending状态
    if(event_add(csig, 0) != 0)
    {
        cerr << "SIGINT event_add failed!"<< endl;
        return ERROR;
    }

    //添加Kill信号  SIGTERM
    // 非持久只进入一次
    //  event_self_cbarg 传递当前的event
    event *ksig = event_new(base, SIGTERM, EV_SIGNAL, Kill, event_self_cbarg());
    if(!ksig)
    {
        cerr << "SIGTERM evsignal_new failed." << endl;
        return ERROR;
    }

    // 添加事件到pending
    if(event_add(ksig, NULL))
    {
        cerr << "SIGTERM event_add failed." << endl;
        return ERROR;
    }

    //进入事件主循环
    event_base_dispatch(base);

    event_free(csig);
    event_free(ksig);

    event_base_free(base);

    return 0;
}

