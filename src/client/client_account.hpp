#pragma once
#include<WinSock2.h>
#include<iostream>
#include<string>

using std::cout;
using std::endl;
using std::string;


class client_account
{
	public:
		bool login(int& verify , const SOCKET& client_sock , string& name)
		{
			cout << "user name:" << endl;
			if (send_wrapper(client_sock , name))
			{
				cout << "password:" << endl;
				if (!send_wrapper(client_sock , name))
					return false;

				int state = 0;
				if (!recv_wrapper(client_sock , state))
					return false;

				verify = state;
				return true;
			}

			return false;
		}

	private:
		bool recv_wrapper(const SOCKET& client_sock , int& state)
		{
			char recvBuf[256];
			int len = recv_msg(client_sock , recvBuf , 256);
			if (len > 0)
				state = recvBuf[0] - 48;
			else
				return false;

			return true;
		}

		bool send_wrapper(const SOCKET& client_sock , string& name)
		{
			char msg[256];
			gets_s(msg , 255);

			int msg_len = send_msg(client_sock , msg);
			if (name.empty())
				name.assign(msg);//get user name

			return msg_len >= 0;
		}

		int send_msg(const SOCKET& client_sock , const char* msg , int flag = 0)
		{
			return send(client_sock , msg , strlen(msg) + 1 , flag);
		}

		int recv_msg(const SOCKET& client_sock , char* recvBuf , int len , int flag = 0)
		{
			return recv(client_sock , recvBuf , len , flag);
		}
};