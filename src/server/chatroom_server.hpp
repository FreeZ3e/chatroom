#pragma once
#pragma comment(lib, "ws2_32.lib")


#include<winsock2.h>
#include<WS2tcpip.h>
#include<thread>
#include<iostream>
#include"server_account.hpp"

using std::cout;
using std::endl;
using std::thread;
using std::ref;


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

		chatroom_base(int af , int type , int protocol , int port , int backlog = 5 , unsigned int main_ver = 2 , unsigned int ver = 2)
		{
			startup_check(main_ver , ver , af , type , protocol , port , backlog);
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

		bool startup_ws2(unsigned int main_ver , unsigned int ver)
		{
			version = MAKEWORD(main_ver , ver);
			return WSAStartup(version , &dat) == 0;
		}

		bool create_socket(int af , int type , int protocol)
		{
			server_sock = socket(af , type , protocol);
			return server_sock != INVALID_SOCKET;
		}

		void create_sockaddr(int af , int port)
		{
			server_in.sin_family = af;
			server_in.sin_port = htons(port);
			server_in.sin_addr.s_addr = INADDR_ANY;
		}

		bool socket_bind()
		{
			return bind(server_sock , (sockaddr*)&server_in , sizeof(server_in)) != -1;
		}

		bool socket_listen(int backlog)
		{
			return listen(server_sock , backlog) != -1;
		}

		void startup_check(unsigned int main_ver , unsigned int ver , int af , int type , int protocol , int port , int backlog)
		{
			bool err = startup_ws2(main_ver , ver);
			if (err == false)
			{
				err_tag = 1;
				return;
			}

			err = create_socket(af , type , protocol);
			if (err == false)
			{
				err_tag = 2;
				return;
			}

			create_sockaddr(af , port);
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
		server_account account;
		string name;

	public:
		chatroom_server(int af , int type , int protocol , int port , int backlog = 5 , unsigned int main_ver = 2 , unsigned int ver = 2):
			chatroom_base(af , type , protocol , port , backlog , main_ver , ver)
		{}

		void run()
		{
			while (1)
			{
				sockaddr_in client_in;
				int addr_len = sizeof(sockaddr_in);
				SOCKET client_socket = INVALID_SOCKET;

				client_socket = accept(server_sock , (sockaddr*)&client_in , &addr_len);
				if (client_socket == INVALID_SOCKET)
				{
					cout << "invalid client socket" << endl;
				}
				else
				{
					char client_ip[INET_ADDRSTRLEN];
					inet_ntop(AF_INET , &client_in.sin_addr , client_ip , sizeof(client_ip));
					cout << "chatting with client: " << client_ip << endl;
					cout << "you can input msg anytime" << endl;
					cout << "---------------------------------------------------" << endl;

					int verify = 0;
					if (account.login(client_socket , name , verify))
					{
						bool connect_state = true;
						while (verify == 1)//passwd wrong
						{
							cout << "password wrong" << endl;
							if (!account.login(client_socket , name , verify))
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

							cout << "-----------------------chatroom---------------------" << endl;
							chat(client_socket);
						}
					}

					cout << "lost connect with client" << endl;
				}

				closesocket(client_socket);
			}
		}

	private:
		//chat server

		void chat(const SOCKET& client_socket)
		{
			while (1)
			{
				bool send_state = true;
				bool recv_state = true;

				bool send_tag = false;
				bool recv_tag = false;

				thread th_send(&chatroom_server::send_wrapper , this ,
							   ref(client_socket) , ref(send_state) , ref(send_tag));
				thread th_recv(&chatroom_server::recv_wrapper , this ,
							   ref(client_socket) , ref(recv_state) , ref(recv_tag));

				th_recv.detach();
				th_send.detach();


				if (connect_check(send_state , recv_state))
				{
					th_recv.~thread();
					th_send.~thread();
					break;
				}

				while (send_tag == false && recv_tag == false)
				{
				}

				if (connect_check(send_state , recv_state))
				{
					th_recv.~thread();
					th_send.~thread();
					break;
				}
			}
		}

		int send_msg(const SOCKET& client_socket , const char* msg , int flag = 0)
		{
			return send(client_socket , msg , strlen(msg) + 1 , flag);
		}

		int recv_msg(const SOCKET& client_socket , char* recvBuf , int len , int flag = 0)
		{
			return recv(client_socket , recvBuf , len , flag);
		}

		void send_wrapper(const SOCKET& client_socket , bool& state , bool& s_tag)
		{
			cout << "\nserver: ";

			char input[256];	//1kbit
			gets_s(input , 255);

			int msg_len = send_msg(client_socket , input);
			state = (msg_len >= 0);

			s_tag = true;
		}

		void recv_wrapper(const SOCKET& client_socket , bool& state , bool& r_tag)
		{
			char recvBuf[256];
			int len = recv_msg(client_socket , recvBuf , 256);

			if (len > 0)
				cout << "\n		" << name << ": " << recvBuf << endl;
			else
				state = false;

			r_tag = true;
		}


		//haven't deployed
		void heart_beat(const SOCKET& client_socket , bool& state)
		{
			char beat[4] = "/hb";

			int beat_len = send_msg(client_socket , beat);
			state = (beat_len >= 0);
		}

		bool connect_check(bool send_state , bool recv_state)
		{
			return (!(send_state && recv_state) && errno != EINTR);
		}
};