#pragma once
#pragma comment(lib, "ws2_32.lib")

#include<winsock2.h>
#include<WS2tcpip.h>
#include<thread>
#include<iostream>


using std::cout;
using std::endl;
using std::thread;
using std::ref;


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
	public:
		chatroom_client(int af , int type , int protocol , int port , const char* ip , unsigned int main_ver = 2 , unsigned int ver = 2):
			chatroom_base(af , type , protocol , port , ip , main_ver , ver)
		{}

		void run()
		{
			while (1)
			{
				bool send_staute = true;
				bool recv_staute = true;

				bool send_tag = false;
				bool recv_tag = false;

				bool exit_tag = false;

				thread th_recv(&chatroom_client::recv_wrapper , this ,
							   ref(recv_staute) , ref(send_tag) , ref(exit_tag));
				thread th_send(&chatroom_client::send_wrapper , this ,
							   ref(send_staute) , ref(recv_tag) , ref(exit_tag));

				th_recv.detach();
				th_send.detach();

				if (connect_check(send_staute , recv_staute))
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

				while (send_tag == false && recv_tag == false)
				{
				}

				if (connect_check(send_staute , recv_staute))
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
			}

			shutdown(client_sock , SD_BOTH);
			closesocket(client_sock);
			cout << "lost connect with server" << endl;
		}

	private:
		//chat server

		int send_msg(const char* msg , int flag = 0)
		{
			return send(client_sock , msg , strlen(msg) + 1 , flag);
		}

		int recv_msg(char* recvBuf , int len , int flag = 0)
		{
			return recv(client_sock , recvBuf , len , flag);
		}

		void send_wrapper(bool& staute , bool& s_tag , bool& exit)
		{
			char msg[256];
			gets_s(msg , 255);

			if (aux_input_equal(msg , "/exit"))
			{
				exit = true;
				s_tag = true;
				return;
			}

			int msg_len = send_msg(msg);
			staute = (msg_len >= 0);

			s_tag = true;
		}

		void recv_wrapper(bool& staute , bool& r_tag , bool& exit)
		{
			char recvBuf[256];	//1k
			int len = recv_msg(recvBuf , 256);
			if (len > 0)
				cout << "server: " << recvBuf << endl;
			else
				staute = false;

			if (aux_input_equal(recvBuf , "/exit"))
				exit = true;

			r_tag = true;
		}

		//haven't deployed
		void heart_beat(bool& staute)
		{
			char beat[4];

			int beat_len = recv_msg(beat , 4);
			staute = (beat_len > 0);
		}

		bool connect_check(bool send_staute , bool recv_staute)
		{
			return (!(send_staute && recv_staute) && errno != EINTR);
		}
};