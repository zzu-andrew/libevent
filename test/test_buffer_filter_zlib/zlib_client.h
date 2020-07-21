#ifndef __ZLIB__CLIENT_H__
#define __ZLIB__CLIENT_H__

#include "event_interface.h"
#include <zlib.h>

bufferevent_filter_result filter_out(evbuffer *s, evbuffer *d,ev_ssize_t limit, bufferevent_flush_mode mode, void * arg);

void client_read_cb(bufferevent *bev, void *arg);

void client_write_cb(bufferevent *bev, void *arg);

void client_event_cb(bufferevent *bev, short events, void *arg);

void Client(event_base *base);

#endif  // !__ZLIB__CLIENT_H__