#include <stdio.h>
#include "event.h"
#include "eventpriv.h"
#include <unistd.h>
#include <string.h>

/* Event Selector */
EventSelector *event_selector;

/**********************************************************************
* %FUNCTION: InterfaceHandler
* %ARGUMENTS:
*  es -- event selector (ignored)
*  fd -- file descriptor which is readable
*  flags -- ignored
*  data -- Pointer to the Interface structure
* %RETURNS:
*  Nothing
* %DESCRIPTION:
*  Handles a packet ready at an interface
***********************************************************************/
void
InterfaceHandler(EventSelector *es,
		 int fd,
		 unsigned int flags,
		 void *data)
{
    char szReadStr[1024];
    memset(szReadStr, 0, sizeof(szReadStr));
    ssize_t readLen = read(fd, szReadStr, sizeof(szReadStr));
    printf("data = %s\n", szReadStr);
}

void
test(EventSelector *es,
		 int fd,
		 unsigned int flags,
		 void *data)
{
    struct timeval t;
    t.tv_sec = 10;
    t.tv_usec = 0;

    Event_AddTimerHandler(event_selector, t, test, NULL);
    printf("test \n\n");
}

int main(int argc, char const *argv[])
{
    EventHandler *eh;		/* Event handler for this interface */
    int i = 0;
    
    event_selector = Event_CreateSelector();
    
    eh = Event_AddHandler(event_selector,
					    STDIN_FILENO,
					    EVENT_FLAG_READABLE,
					    InterfaceHandler,
					    NULL);
    struct timeval t;
    t.tv_sec = 10;
    t.tv_usec = 0;
    EventHandler *evTimer = Event_AddTimerHandler(event_selector, t, test, NULL);
    for(;;) {
	i = Event_HandleEvent(event_selector);
	if (i < 0) {
	    printf("Event_HandleEvent");
	}
    }
    
    printf("hello world!\n");
    return 0;
}
