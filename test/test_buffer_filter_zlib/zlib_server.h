#ifndef __ZLIB_SERVER__
#define __ZLIB_SERVER__

#include "event_interface.h"





bufferevent_filter_result filter_in(evbuffer *s, evbuffer *d,ev_ssize_t limit,
                bufferevent_flush_mode mode,void *arg);


void read_cb(bufferevent *bev, void *arg);



void event_cb(bufferevent *bev, short events, void *arg);


static void listen_cb(struct evconnlistener * e, evutil_socket_t s, struct sockaddr *a, int socklen, void *arg);

void Server(event_base*base);





#endif //__zlib_server__