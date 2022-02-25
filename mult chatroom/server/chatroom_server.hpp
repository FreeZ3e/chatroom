#pragma once
#pragma comment(lib, "ws2_32.lib")

#include<winsock2.h>
#include<WS2tcpip.h>
#include<thread>
#include<iostream>
#include<vector>
#include"server_account.hpp"
#include"server_command.hpp"

using std::cout;
using std::endl;
using std::thread;
using std::vector;


class chatroom_base
{
	private:
		//WSA
		WORD version;
		WSADATA dat;

		//sockaddr
		sockaddr_in server_in;

		//startup error tag
		unsigned int err_tag = 0;

	protected:
		//socket
		SOCKET server_sock;

	protected:

		chatroom_base(int af, int type, int protocol, int port, int backlog = 5, unsigned int main_ver = 2, unsigned int ver = 2)
		{
			startup_check(main_ver, ver, af, type, protocol, port, backlog);
			if (err_tag != 0)
				cout << "the error number: " << err_tag << endl;
			else
			{
				cout << "server running" << endl;
				cout << "-----------------------------------------------" << endl;
			}
		}

		~chatroom_base()
		{
			closesocket(server_sock);
			WSACleanup();
			cout << "server closed" << endl;
		}

	private:

		bool startup_ws2(unsigned int main_ver, unsigned int ver)
		{
			version = MAKEWORD(main_ver, ver);
			return WSAStartup(version, &dat) == 0;
		}

		bool create_socket(int af, int type, int protocol)
		{
			server_sock = socket(af, type, protocol);
			return server_sock != INVALID_SOCKET;
		}

		void create_sockaddr(int af, int port)
		{
			server_in.sin_family = af;
			server_in.sin_port = htons(port);
			server_in.sin_addr.s_addr = INADDR_ANY;
		}

		bool socket_bind()
		{
			return bind(server_sock, (sockaddr*)&server_in, sizeof(server_in)) != -1;
		}

		bool socket_listen(int backlog)
		{
			return listen(server_sock, backlog) != -1;
		}

		void startup_check(unsigned int main_ver, unsigned int ver, int af, int type, int protocol, int port, int backlog)
		{
			bool err = startup_ws2(main_ver, ver);
			if (err == false)
			{
				err_tag = 1;
				return;
			}

			err = create_socket(af, type, protocol);
			if (err == false)
			{
				err_tag = 2;
				return;
			}

			create_sockaddr(af, port);
			err = socket_bind();
			if (err == false)
			{
				err_tag = 3;
				return;
			}

			err = socket_listen(backlog);
			if (err == false)
				err_tag = 4;
		}
};


class chatroom_server : private chatroom_base
{
	private:
		vector<SOCKET> socket_arr;
		vector<string> user_name;
		vector<string> history_msg;

		int client_num = 0;

		server_account account;
		server_command com;

	public:
		chatroom_server(int af, int type, int protocol, int port,
			int backlog = 5, unsigned int main_ver = 2, unsigned int ver = 2) :
			chatroom_base(af, type, protocol, port, backlog, main_ver, ver)
		{}

		void run()
		{
			while (1)
			{
				SOCKET client_socket = accpet_client();

				if (client_socket != INVALID_SOCKET)
				{
					thread th_channel(&chatroom_server::client_channel, this, client_socket);
					th_channel.detach();
				}
			}
		}

	private:
		//chat server

		SOCKET accpet_client()
		{
			sockaddr_in client_in;
			int addr_len = sizeof(sockaddr_in);
			SOCKET client_socket = INVALID_SOCKET;

			return accept(server_sock, (sockaddr*)&client_in, &addr_len);
		}

		void client_channel(const SOCKET client_socket)
		{
			string name;

			if (login_wrapper(client_socket ,name))//login
			{
				while (1)//chat
				{
					char msg_buf[1024];
					if (!recv_wrapper(client_socket, msg_buf))
						break;

					if (!com.command(client_socket, account, msg_buf, name, 
									socket_arr, user_name))
					{
						cout << name << ": " << msg_buf << endl;

						string msg = msg_wrapper(name, msg_buf).c_str();
						history_msg.push_back(msg);
						send_wrapper(client_socket, msg.c_str());
					}
				}

				send_notice(client_socket, name, " is disconnected");
				logout_wrapper(name);
			}
		}

		bool login_wrapper(const SOCKET& client_socket, string& name)
		{
			int verify = 0;
			if (account.login(client_socket, name, verify))
			{
				bool connect_state = true;
				while (verify == 1 || verify == 3)
				{
					cout << "password wrong or user are login" << endl;
					if (!account.login(client_socket, name, verify))
					{
						connect_state = false;
						break;
					}
				}

				if (connect_state)
				{
					if (verify == 0)
						cout << "verify success" << endl;
					else
						cout << "create a new account" << endl;

					user_name.push_back(name);
					socket_arr.push_back(client_socket);

					send_notice(client_socket, name, " is online");
					send_history_msg(client_socket);
					cout << "client num: " << ++client_num << endl;

					return true;
				}
			}

			return false;
		}

		void logout_wrapper(const string& name)
		{
			if (account.logout(name))
				cout << name << " logout" << endl;
			arr_clear();
			name_clear(name);

			client_num--;
		}

		void arr_clear()
		{
			auto iter = socket_arr.begin();
			for (; iter != socket_arr.end();)
			{
				if ((*iter) == INVALID_SOCKET)
					iter = socket_arr.erase(iter);
				else
					++iter;
			}

			cout << "invaliad socket clear" << endl;
		}

		void name_clear(const string& name)
		{
			auto iter = user_name.begin();
			for (; iter != user_name.end();)
			{
				if ((*iter) == name)
					iter = user_name.erase(iter);
				else
					++iter;
			}

			cout << "invaliad user name clear" << endl;
		}


		int send_msg(const SOCKET& client_socket, const char* msg, int flag = 0)
		{
			return send(client_socket, msg, strlen(msg) + 1, flag);
		}

		int recv_msg(const SOCKET& client_socket, char* recvBuf, int len, int flag = 0)
		{
			return recv(client_socket, recvBuf, len, flag);
		}

		string msg_wrapper(const string& name, const string& msg)
		{
			return name + "/NA/" + msg;
		}

		void send_wrapper(const SOCKET& client_socket, const char* msg_buf)
		{
			for (size_t n = 0; n < socket_arr.size(); ++n)
			{
				if (socket_arr[n] != INVALID_SOCKET && 
					client_socket != socket_arr[n])
				{
					int send_len = send_msg(socket_arr[n], msg_buf);
					if (send_len <= 0)
					{
						closesocket(socket_arr[n]);
						socket_arr[n] = INVALID_SOCKET;
					}
				}
			}
		}

		bool recv_wrapper(const SOCKET& client_socket, char* msg_buf)
		{
			return recv_msg(client_socket, msg_buf, 1024) > 0;
		}


		void send_notice(const SOCKET& client_socket, const string& name, const string& tip)
		{
			string notice = name + tip;
			notice = msg_wrapper("server", notice);
			send_wrapper(client_socket, notice.c_str());
		}

		void send_history_msg(const SOCKET& client_socket)
		{
			size_t size = history_msg.size();
			for (size_t n = 0; n < size; n++)
			{
				send_msg(client_socket, history_msg[n].c_str());
				Sleep(10);
			}

			send_msg(client_socket, "/hisend");
		}
};