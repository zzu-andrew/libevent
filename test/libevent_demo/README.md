# `MINI`libevent

该库是抽离至开源组件`pppoe`

```c
/* Handler structure */
typedef struct EventHandler_t {
    struct EventHandler_t *next; /* Link in list                           */
    int fd;			/* File descriptor for select              */
    unsigned int flags;		/* Select on read or write; enable timeout */
    struct timeval tmout;	/* Absolute time for timeout               */
    EventCallbackFunc fn;	/* Callback function                       */
    void *data;			/* Extra data to pass to callback          */
} EventHandler;

/* Selector structure */
typedef struct EventSelector_t {
    EventHandler *handlers;	/* Linked list of EventHandlers            */
    int nestLevel;		/* Event-handling nesting level            */
    int opsPending;		/* True if operations are pending          */
    int destroyPending;		/* If true, a destroy is pending           */
} EventSelector;
```

