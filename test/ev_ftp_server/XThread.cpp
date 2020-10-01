#include "XThread.h"
#include <thread>
#include <iostream>
#include <event2/event.h>
#include "XTask.h"
#include <unistd.h>

using namespace std;

//�����߳�����Ļص�����?
static void NotifyCB(evutil_socket_t fd, short which, void *arg)
{
	XThread *t = (XThread *)arg;
	t->Notify(fd, which);
}
void XThread::Notify(evutil_socket_t fd, short which)
{
	//ˮƽ���� ֻҪû�н�����ɣ����ٴν���?
	char buf[2] = { 0 };

	//linux���ǹܵ���������recv
	int re = read(fd, buf, 1);

	if (re <= 0)
		return;
	cout << id << " thread " << buf << endl;
	XTask *task = NULL;
	//��ȡ���񣬲���ʼ������
	tasks_mutex.lock();
	if (tasks.empty())
	{
		tasks_mutex.unlock();
		return;
	}
	task = tasks.front(); //�Ƚ��ȳ�
	tasks.pop_front();
	tasks_mutex.unlock();
	task->Init();
}

void XThread::AddTask(XTask *t)
{
	if (!t)return;
	t->base = this->base;
	tasks_mutex.lock();
	tasks.push_back(t);
	tasks_mutex.unlock();
}
//�̼߳���
void XThread::Activate()
{

	int re = write(this->notify_send_fd, "c", 1);
	if (re <= 0)
	{
		cerr << "XThread::Activate() failed!" << endl;
	}
}
//�����߳�
void XThread::Start()
{
	Setup();
	//�����߳�
	thread th(&XThread::Main,this);

	//�Ͽ������߳���ϵ
	th.detach();
}
//��װ�̣߳���ʼ��event_base�͹ܵ������¼����ڼ���
bool XThread::Setup()
{
	//windows�����socket linux�ùܵ�

	//�����Ĺܵ� ������send recv��ȡ read write
	int fds[2];
	if (pipe(fds))
	{
		cerr << "pipe failed!" << endl;
		return false;
	}


	//��ȡ�󶨵�event�¼��У�д��Ҫ����
	notify_send_fd = fds[1];

	//����libevent�����ģ�������
	event_config *ev_conf = event_config_new();
	event_config_set_flag(ev_conf, EVENT_BASE_FLAG_NOLOCK);
	this->base = event_base_new_with_config(ev_conf);
	event_config_free(ev_conf);
	if (!base)
	{
		cerr << "event_base_new_with_config failed in thread!" << endl;
		return false;
	}

	//���ӹܵ������¼������ڼ����߳�ִ������
	event *ev = event_new(base, fds[0], EV_READ | EV_PERSIST, NotifyCB, this);
	event_add(ev, 0);

	return true;
}
//�߳���ں���?
void XThread::Main()
{
	cout << id << " XThread::Main() begin" << endl;
	event_base_dispatch(base);
	event_base_free(base);

	cout << id << " XThread::Main() end" << endl;
}


XThread::XThread()
{
}


XThread::~XThread()
{
}
