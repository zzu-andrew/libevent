#include <iostream>
#include <event2/event.h>
#include <signal.h>
#include "event_interface.h"


using namespace std;

void read_file(evutil_socket_t fd, short event, void *arg)
{
    char buf[1024] = {0};
    int len = read(fd, buf, sizeof(buf) - 1);
    if(len>0)
    {
        cout << buf << endl;
    }
    else
    {
        cout << "." << flush;
        this_thread::sleep_for(300ms);
    }
    
}



int main(int argc, const char** argv) {
    // 发送给已经关闭的socket会导致SGIPIPE
    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        return ERROR;
    }

    event_config *config = event_config_new();
    // 设置支持文件描述符
    event_config_require_features(config, EV_FEATURE_FDS);
    event_base *base = event_base_new_with_config(config);
    event_config_free(config);
    if(!base)
    {
        cerr << "event base new with config failed." << endl;
        return ERROR;
    }
    
    // 打开文件以只读方式  非阻塞
    // int sock = open("/var/log/auth.log", O_RDONLY|O_NONBLOCK);
     int sock = open("test.txt", O_RDONLY|O_NONBLOCK);
    if(0 == sock)
    {
        cerr << "open auth.log failed." << endl;
        return OPEN_FAILED;
    }

    // 文件指针移动到结尾处
    lseek(sock, 0, SEEK_END);

    // 监听文件数据
    event *fev = event_new(base, sock, EV_READ|EV_PERSIST, read_file, 0);
    event_add(fev, NULL);

    // 事件分发
    event_base_dispatch(base);


    event_free(fev);
    event_base_free(base);

    return OK;
}

