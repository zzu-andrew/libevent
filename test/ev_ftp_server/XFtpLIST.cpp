#include "XFtpLIST.h"
#include <iostream>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <string>
using namespace std;

void XFtpLIST::Write(struct bufferevent *bev)
{
	//4 226 Transfer complete发送完成
	ResCMD("226 Transfer complete\r\n");
	//5 关闭连接
	Close();

}
void XFtpLIST::Event(struct bufferevent *bev, short what)
{
	//如果对方网络断掉，或者机器死机有可能收不到BEV_EVENT_EOF数据
	if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
	{
		cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR |BEV_EVENT_TIMEOUT" << endl;
		Close();
	}
	else if(what&BEV_EVENT_CONNECTED)
	{
		cout << "XFtpLIST BEV_EVENT_CONNECTED" << endl;
	}
}
//解析协议
void XFtpLIST::Parse(std::string type, std::string msg)
{
	string resmsg = "";

	if (type == "PWD")
	{
		//257 "/" is current directory.
		resmsg = "257 \"";
		resmsg += cmdTask->curDir;
		resmsg += "\" is current dir.\r\n";

		ResCMD(resmsg);
	}
	else if (type == "LIST")
	{
		//1连接数据通道 2 150 3 发送目录数据通道 4 发送完成226 5 关闭连接
		//命令通道回复消息 使用数据通道发送目录
		//-rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jpg\r\n
		//1 连接数据通道 
		ConnectPORT();
		//2 1502 150
		ResCMD("150 Here comes the directory listing.\r\n");
		//string listdata = "-rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jpg\r\n";
		string listdata = GetListData(cmdTask->rootDir + cmdTask->curDir);
		//3 数据通道发送
		Send(listdata);
	}
	else if (type == "CWD") //切换目录
	{
		//取出命令中的路径
		//CWD test\r\n
		int pos = msg.rfind(" ") + 1;
		//去掉结尾的\r\n
		string path = msg.substr(pos, msg.size() - pos - 2);
		if (path[0] == '/') //局对路径
		{
			cmdTask->curDir = path;
		}
		else
		{
			if (cmdTask->curDir[cmdTask->curDir.size() - 1] != '/')
				cmdTask->curDir += "/";
			cmdTask->curDir += path + "/";
		}
		//  /test/
		ResCMD("250 Directory succes chanaged.\r\n");

		//cmdTask->curDir += 
	}
	else if (type == "CDUP") //回到上层目录
	{
		//  /Debug/test_ser.A3C61E95.tlog /Debug   /Debug/
		string path = cmdTask->curDir;
		//统一去掉结尾的 /
		////  /Debug/test_ser.A3C61E95.tlog /Debug  
		if (path[path.size() - 1] == '/')
		{
			path = path.substr(0, path.size() - 1);
		}
		int pos = path.rfind("/");
		path = path.substr(0, pos);
		cmdTask->curDir = path;
		ResCMD("250 Directory succes chanaged.\r\n");
	}
}
std::string XFtpLIST::GetListData(std::string path)
{
	//-rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jpg\r\n
	string data = "";


	string cmd = "ls -l ";
	cmd += path;
	cout << "popen:" << cmd << endl;
	FILE *f = popen(cmd.c_str(), "r");
	if (!f)
		return data;
	char buffer[1024] = { 0 };
	for (;;)
	{
		int len = fread(buffer, 1, sizeof(buffer) - 1, f);
		if (len <= 0)break;
		buffer[len] = '\0';
		data += buffer;
	}
	pclose(f);

	return data;
}