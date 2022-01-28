#include"chatroom_client.hpp"


int main()
{
	chatroom_client client(AF_INET , SOCK_STREAM , IPPROTO_TCP , 8888 , "127.0.0.1");
	client.run();

	system("pause");
	return 0;
}