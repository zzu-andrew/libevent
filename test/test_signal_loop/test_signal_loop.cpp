#include <iostream>
#include "event_interface.h"

using namespace std;

//sock 文件描述符，which事件类型 arg 传递的参数
static void Ctrl_C(int sock, short which, void *arg)
{
    cout << "Ctrl + C" << endl;
    event_base *base = (event_base *)arg;
    // 执行完当前处理的事件函数就退出

    // event_base_loopbreak(base); //运行完当前的事件就退出

    // 运行完所有的活动事件再退出
    // 事件循环没有运行时也要等运行一次在退出
    timeval t ={3,0};
    event_base_loopexit(base, 0); // 运行完所有的事件在退出，事件循环没有运行时也要等事件处理完在退出

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

    // // 
    // EVLOOP_ONCE  等待一个事件运行，直到没有活动事件就退出
    // EVLOOP_NONBLOCK   没有活动事件处理，就返回0
    // EVLOOP_NO_EXIT_ON_EMPTY 没有注册事件也不返回 用于事件后期多线程添加
    // 就算前期一个event_add也没有执行 只要设置了EVLOOP_NO_EXIT_ON_EMPTY线程就不回退出，这样
    // 就方便后期后期事件的添加
    event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY);
   
    event_free(csig);
    event_free(ksig);
    event_base_free(base);

    return 0;
}

