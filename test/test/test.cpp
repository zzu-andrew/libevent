#include <iostream>
#include <string>


using namespace std;
int main(int argc, char const *argv[])
{
    
    //-rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jpg\r\n
	string data = "";


	string cmd = "ls -l ";
	cout << "popen:" << cmd << endl;
	FILE *f = popen(cmd.c_str(), "r");
	if (!f)
		return -1;
	char buffer[1024] = { 0 };
	for (;;)
	{
		int len = fread(buffer, 1, sizeof(buffer) - 1, f);
		if (len <= 0)break;
		buffer[len] = '\0';
		data += buffer;
	}
	pclose(f);




	cout << data << endl;


    return 0;
}

