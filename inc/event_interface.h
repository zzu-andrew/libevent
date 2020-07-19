/**
 *  事件驱动interface接口
 * 
 * */

#ifndef __EVENT_INTERFACE__
#define __EVENT_INTERFACE__

// 公共头文件引入
#include <thread>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <event2/event.h>
#include <iostream>
#include <signal.h>
#include <event2/listener.h>
#include <string.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_struct.h>
#include <event2/buffer.h>
#include <zlib.h>


/* MACRO DEFINE */
#define  ERROR        -1
#define OK             0


enum
{
    OPEN_FAILED = -2,
    CREATE_FILE_FAILED = -3,
    BIND_FAILED =-4,

}ENUM_ERROR;

#define SUCCESS       0
#define FAILED        1

/**
 * @def 定义服务端端口 
 **/
#define SERVER_PORT 5001






#endif // !__EVENT_INTERFACE__
