#include <stdio.h>
#include "event.h"
#include "eventpriv.h"
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>


// 事件选择器
EventSelector *event_selector;



void printf_test_handler(struct EventSelector_t *es,
				 int fd, unsigned int flags,
				 void *data)
{
    char szReadStr[1024];
    memset(szReadStr, 0, sizeof(szReadStr));
    ssize_t readLen = read(fd, szReadStr, sizeof(szReadStr));
    printf("data = %s", szReadStr);
    int eventId = (int)(*(int *)data);
    printf("eventId = %d\n", eventId);
}


// 事件循环

// 定时事件
void timer_test_handler(struct EventSelector_t *es,
				 int fd, unsigned int flags,
				 void *data)
{
    struct timeval t;
    t.tv_sec = 10;
    t.tv_usec = 0;
    Event_AddTimerHandler(event_selector, t, timer_test_handler, NULL);
    printf("timer test.\n");
}

int main(int argc, char const *argv[])
{
    
    event_selector = Event_CreateSelector();
    int ret;

    EventHandler *eh;
    int eventId = 1;
    eh = Event_AddHandler(event_selector, STDIN_FILENO, EVENT_FLAG_READABLE, printf_test_handler, &eventId);




    struct timeval t;
    t.tv_sec = 10;
    t.tv_usec = 0;
    EventHandler *evTimer =  Event_AddTimerHandler(event_selector, t, timer_test_handler, NULL);


    for(;;)
    {
        ret = Event_HandleEvent(event_selector);
        if(ret < 0)
        {
            printf("errno = [%d]\n", errno);
        }
    }
    

    printf("hello world.\n");
    return 0;
}
