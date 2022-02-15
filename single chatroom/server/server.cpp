#include"chatroom_server.hpp"


int main()
{
	chatroom_server server(AF_INET , SOCK_STREAM , IPPROTO_TCP , 8888);
	server.run();

	system("pause");
	return 0;
}