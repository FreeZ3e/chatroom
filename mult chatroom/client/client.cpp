#include"chatroom_client.hpp"


int main()
{
	int port = 0;
	string buf;

	aux_set_ip(buf, port);

	chatroom_client client(AF_INET , SOCK_STREAM , IPPROTO_TCP , port, buf.c_str());
	client.run();

	system("pause");
	return 0;
}