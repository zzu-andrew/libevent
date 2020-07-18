#include <iostream>
#include <event2/event.h>
#include <signal.h>
#include "event_interface.h"
#include <thread>

using namespace std;

// | s | us |
static timeval t1 = {5,0};
void timer1(int sock, short which, void *arg)
{
    cout << "{timer1}" << endl;
    event *ev = (event*)arg;

    // no pending
    if(!evtimer_pending(ev, &t1))
    {
        evtimer_del(ev);
        evtimer_add(ev, &t1);
    }
}


/* 调用的函数内部不能进行延时，需要加延时可以在外部进行添加 */
// 否则可能造成计时不准确
void timer2(int sock, short which, void *arg)
{
    cout << "[timer2]"<<flush;
    this_thread::sleep_for(1ms);
}

void timer3(int sock, short which, void *arg)
{
    cout << "[timer3]"<<flush;
}

int main(int argc, const char** argv) {

    event_base *base = event_base_new();
   

    // 定时器 
    cout << "test timer" << endl;

    //
    /*
    #define evtimer_assign(ev, b, cb, arg) \
	    event_assign((ev), (b), -1, 0, (cb), (arg))
    #define evtimer_new(b, cb, arg)		event_new((b), -1, 0, (cb), (arg))
    #define evtimer_add(ev, tv)		event_add((ev), (tv))
    #define evtimer_del(ev)			event_del(ev)
    #define evtimer_pending(ev, tv)		event_pending((ev), EV_TIMEOUT, (tv))
    #define evtimer_initialized(ev)		event_initialized(ev)
    */

    // 定时器非持久事件
    // 需要在回调函数中再次进行添加
    struct event *ev1 = evtimer_new(base,timer1,event_self_cbarg());
    if(!ev1)
    {
        cout << "evtimer_new timer1 failed." << endl;
        return ERROR;
    }

    evtimer_add(ev1, &t1);
    static timeval t2;
    t2.tv_sec = 1;
    t2.tv_usec = 200000;//微妙

    //EV_PERSIST  添加之后 回调函数就会一直存在 不在删除
    event *ev2 = event_new(base,-1,EV_PERSIST,timer2,0);
    event_add(ev2, &t2);

    event *ev3 = event_new(base, -1, EV_PERSIST, timer3, 0);
    //超时优化性能优化，默认event用二叉堆存储（完全二叉树），插入删除 0(logn)
    //优化到双向队列  插入删除0 (1)
    static timeval tv_in = {1,0};
    const timeval *t3;
    t3 = event_base_init_common_timeout(base, &tv_in);
    event_add(ev3, t3);

    event_base_dispatch(base);

    event_free(ev1);
    event_free(ev2);
    event_free(ev3);
    event_base_free(base);

    
    return 0;
}