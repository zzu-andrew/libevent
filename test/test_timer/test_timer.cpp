#include <iostream>
#include <event2/event.h>
#include <signal.h>
#include "event_interface.h"
#include <thread>

using namespace std;

// | s | us |
static timeval t1 = {1,0};
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

void timer2(int sock, short which, void *arg)
{
    cout << "[timer2]"<<flush;
    this_thread::sleep_for(3000ms);
}

void timer3(int sock, short which, void *arg)
{
    cout << "[timer3]"<<flush;
}

int main(int argc, const char** argv) {

    event_base *base = event_base_new();
   

    // 定时器
    cout << "test timer" << endl;

    //event new
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
    event *ev2 = event_new(base,-1,EV_PERSIST,timer2,0);
    event_add(ev2, &t2);

    event *ev3 = event_new(base, -1, EV_PERSIST, timer3, 0);
    

    



 event_base_dispatch(base);
    event_base_free(base);

    return 0;
}

