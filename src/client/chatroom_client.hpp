#pragma once
#pragma comment(lib, "ws2_32.lib")

#include<winsock2.h>
#include<WS2tcpip.h>
#include<thread>
#include<iostream>
#include<string>
#include"client_account.hpp"
#include"client_command.hpp"

using std::cout;
using std::endl;
using std::thread;
using std::ref;
using std::string;


//string equal
bool aux_input_equal(const char* input , const char* tag)
{
	if (strlen(input) != strlen(tag))
		return false;

	for (size_t n = 0; n < strlen(input); ++n)
	{
		if (input[n] != tag[n])
			return false;
	}

	return true;
}


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
		SOCKET client_sock;

	protected:

		chatroom_base(int af , int type , int protocol , int port , const char* ip , unsigned int main_ver = 2 , unsigned int ver = 2)
		{
			startup_check(main_ver , ver , af , type , protocol , port , ip);
			if (err_tag != 0)
				cout << "the error number: " << err_tag << endl;
			else
			{
				cout << "client running" << endl;
				cout << "chatting with server: " << ip << endl;
				cout << "you can input msg anytime" << endl;
				cout << "-----------------------------------------------" << endl;
			}
		}

		~chatroom_base()
		{
			closesocket(client_sock);
			WSACleanup();
			cout << "client closed" << endl;
		}

	private:
		bool startup_ws2(unsigned int main_ver , unsigned int ver)
		{
			version = MAKEWORD(main_ver , ver);
			return WSAStartup(version , &dat) == 0;//false == error
		}

		bool create_socket(int af , int type , int protocol)
		{
			client_sock = socket(af , type , protocol);
			return client_sock != INVALID_SOCKET;
		}

		bool create_sockaddr(int af , int port , const char* ip)
		{
			server_in.sin_family = af;
			server_in.sin_port = htons(port);
			int ip_err = inet_pton(af , ip , &server_in.sin_addr);

			return ip_err == 1;
		}

		bool connect_to_server()
		{
			return connect(client_sock , (sockaddr*)&server_in , sizeof(sockaddr_in)) != -1;
		}

		void startup_check(unsigned int main_ver , unsigned int ver , int af , int type , int protocol , int port , const char* ip)
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

			err = create_sockaddr(af , port , ip);
			if (err == false)
			{
				err_tag = 3;
				return;
			}

			err = connect_to_server();
			if (err == false)
				err_tag = 4;
		}
};


class chatroom_client : private chatroom_base
{
	private:
		client_command com;
		client_account account;
		string name;

	public:
		chatroom_client(int af , int type , int protocol , int port , const char* ip , unsigned int main_ver = 2 , unsigned int ver = 2):
			chatroom_base(af , type , protocol , port , ip , main_ver , ver)
		{}

		void run()
		{
			int verify = 0;
			if (account.login(verify , client_sock , name))
			{
				bool connect_state = true;
				while (verify == 1)
				{
					cout << "password wrong, input again" << endl;

					name.clear();//passwd wrong, clear user name
					if (!account.login(verify , client_sock , name))
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
					chat();
				}
			}

			shutdown(client_sock , SD_BOTH);
			cout << "lost connect with server" << endl;
		}

	private:
		//chat server

		void chat()
		{
			while (1)
			{
				bool send_state = true;
				bool recv_state = true;

				bool send_tag = false;
				bool recv_tag = false;

				bool exit_tag = false;

				thread th_recv(&chatroom_client::recv_wrapper , this ,
							   ref(recv_state) , ref(send_tag) , ref(exit_tag));
				thread th_send(&chatroom_client::send_wrapper , this ,
							   ref(send_state) , ref(recv_tag) , ref(exit_tag));


				th_recv.detach();
				th_send.detach();

				if (connect_check(send_state , recv_state))
				{
					th_recv.~thread();
					th_send.~thread();
					break;
				}

				if (exit_tag)
				{
					th_recv.~thread();
					th_send.~thread();
					break;
				}

				while ((send_tag == false && recv_tag == false))
				{
				}

				if (connect_check(send_state , recv_state))
				{
					th_recv.~thread();
					th_send.~thread();
					break;
				}

				if (exit_tag)
				{
					th_recv.~thread();
					th_send.~thread();
					break;
				}

				th_send.~thread();
				th_recv.~thread();
			}
		}

		int send_msg(const char* msg , int flag = 0)
		{
			return send(client_sock , msg , strlen(msg) + 1 , flag);
		}

		int recv_msg(char* recvBuf , int len , int flag = 0)
		{
			return recv(client_sock , recvBuf , len , flag);
		}

		void send_wrapper(bool& state , bool& s_tag , bool& exit)
		{
			cout << endl << name << ": ";

			char msg[256];
			gets_s(msg , 255);

			if (com.send_command(client_sock , msg))
			{
				s_tag = true;
				return;
			}

			if (aux_input_equal(msg , "/exit"))
			{
				exit = true;
				s_tag = true;
				return;
			}

			int msg_len = send_msg(msg);
			state = (msg_len >= 0);

			s_tag = true;
		}

		void recv_wrapper(bool& state , bool& r_tag , bool& exit)
		{
			char recvBuf[256];	//1k
			int len = recv_msg(recvBuf , 256);

			if (com.recv_command(recvBuf))
			{
				r_tag = true;
				return;
			}

			if (len > 0)
				cout << "\n		server: " << recvBuf << endl;
			else
				state = false;

			if (aux_input_equal(recvBuf , "/exit"))
				exit = true;

			r_tag = true;
		}


		//haven't deployed
		void heart_beat(bool& state)
		{
			char beat[4];

			int beat_len = recv_msg(beat , 4);
			state = (beat_len > 0);
		}

		bool connect_check(bool send_state , bool recv_state)
		{
			return (!(send_state && recv_state) && errno != EINTR);
		}
};