#pragma once
#include "XFtpTask.h"
class XFtpUSER : public XFtpTask
{
public:
	//����Э��
	virtual void Parse(std::string type, std::string msg);
	XFtpUSER();
	~XFtpUSER();
};

