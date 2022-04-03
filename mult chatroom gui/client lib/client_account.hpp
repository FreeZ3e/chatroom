#pragma once
#include<WinSock2.h>
#include<string>

using std::string;


class client_account
{
	public:
		int _login(const SOCKET& client_sock, const string& name, const string& passwd)
		{
			if (send_msg(client_sock, name.c_str()))
			{
				if (send_msg(client_sock, passwd.c_str()))
				{
					int res = 0;
					if(recv_wrapper(client_sock, res))
						return res;
				}
			}

			return -1;
		}

	private:
		bool recv_wrapper(const SOCKET& client_sock, int& state)
		{
			char recvBuf[256];
			int len = recv_msg(client_sock, recvBuf, 256);
			if (len > 0)
				state = recvBuf[0] - 48;
			else
				return false;

			return true;
		}

		int send_msg(const SOCKET& client_sock, const char* msg, int flag = 0)
		{
			return send(client_sock, msg, strlen(msg) + 1, flag);
		}

		int recv_msg(const SOCKET& client_sock, char* recvBuf, int len, int flag = 0)
		{
			return recv(client_sock, recvBuf, len, flag);
		}
};