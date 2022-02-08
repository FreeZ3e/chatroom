#pragma once
#include<winsock2.h>
#include"chatroom_account.hpp"

class server_account
{
	private:
		chatroom_account account;

	public:
		bool login(const SOCKET& client_socket , string& name , int& state)
		{
			//receive user name & passwd
			string user;
			string passwd;

			bool connect = recv_wrapper(client_socket , user);
			if (connect)
				connect = recv_wrapper(client_socket , passwd);

			if (connect)
			{
				int account_exist = account.account_server(user , passwd);
				connect = send_wrapper(client_socket , account_exist);

				name = user;
				state = account_exist;
			}

			return connect;
		}

		bool account_del(const string& user_name)
		{
			return account.delete_account(user_name);
		}

		bool reset_passwd(const string& user_name , const string& passwd)
		{
			return account.reset_passwd(user_name , passwd);
		}

	private:
		bool recv_wrapper(const SOCKET& client_socket , string& str)
		{
			char recvBuf[256];
			int len = recv_msg(client_socket , recvBuf , 256);
			if (len > 0)
				str.assign(recvBuf);
			else
				return false;

			return true;
		}

		bool send_wrapper(const SOCKET& client_socket , int exist)
		{
			char input[2];
			input[0] = exist + 48;
			input[1] = '\0';

			int msg_len = send_msg(client_socket , input);

			return msg_len >= 0;
		}

		int send_msg(const SOCKET& client_socket , const char* msg , int flag = 0)
		{
			return send(client_socket , msg , strlen(msg) + 1 , flag);
		}

		int recv_msg(const SOCKET& client_socket , char* recvBuf , int len , int flag = 0)
		{
			return recv(client_socket , recvBuf , len , flag);
		}
};