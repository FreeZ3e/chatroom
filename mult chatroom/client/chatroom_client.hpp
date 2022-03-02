#pragma once
#pragma comment(lib, "ws2_32.lib")

#include<winsock2.h>
#include<WS2tcpip.h>
#include<thread>
#include<iostream>
#include<string>
#include<mutex>
#include<condition_variable>
#include"client_account.hpp"
#include"client_command.hpp"

using std::cout;
using std::endl;
using std::thread;
using std::ref;
using std::string;
using std::mutex;
using std::unique_lock;
using std::condition_variable;


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
		client_account account;
		client_command com;

		mutex MUTE; 
		condition_variable cv;

		string name;

	public:
		chatroom_client(int af , int type , int protocol , int port , 
						const char* ip , unsigned int main_ver = 2 , unsigned int ver = 2) :
			chatroom_base(af , type , protocol , port , ip , main_ver , ver)
		{
			com.set_ip(ip);
		}

		void run()
		{
			int verify = 0;
			if (account.login(verify, client_sock, name))
			{
				bool connect_state = true;
				while (verify == 1 || verify == 3)
				{
					cout << "password wrong or user are login" << endl;

					name.clear();
					if (!account.login(verify, client_sock, name))
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

					if (recv_history())
						cout << "---------------------history here-------------------" << endl;

					chat();
				}
			}

			shutdown(client_sock, SD_BOTH);
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

				bool exit_tag = false;

				thread th_recv(&chatroom_client::recv_wrapper, this, ref(send_state));
				thread th_send(&chatroom_client::send_wrapper, this, ref(recv_state), ref(exit_tag));

				th_recv.detach();
				th_send.detach();

				std::unique_lock<std::mutex> lock(MUTE);
				cv.wait(lock);

				if (connect_check(send_state, recv_state) || exit_tag)
				{
					th_recv.~thread();
					th_send.~thread();
					break;
				}
			}
		}

		void send_wrapper(bool& state , bool& exit_tag)
		{
			cout << endl << name << ": ";

			char msg[1024];
			gets_s(msg , 1023);

			if (com.send_command(client_sock, msg, exit_tag))
			{
				cv.notify_one();
				return;
			}

			int msg_len = send_msg(msg);
			state = (msg_len >= 0);

			cv.notify_one();
		}

		void recv_wrapper(bool& state)
		{
			char recvBuf[1024];
			int len = recv_msg(recvBuf , 1024);

			if (com.recv_command(recvBuf, name))
			{
				cv.notify_one();
				return;
			}

			if (len > 0)
			{
				string name, msg;
				divide_name(recvBuf, name, msg);
				cout << "\n		" << name << ": " << msg << endl;
			}
			else
				state = false;

			cv.notify_one();
		}

		bool recv_history()
		{
			bool history = false;

			while (1)
			{
				char recvBuf[1024];
				if(recv_msg(recvBuf,1024) < 0)
					break;

				string name, msg;
				divide_name(recvBuf, name, msg);
				if(name == "/hisend")
					break;
				else
					history = true;

				cout << "\n		" << name << ": " << msg << endl;
			}

			return history;
		}

		int send_msg(const char* msg , int flag = 0)
		{
			return send(client_sock , msg , strlen(msg) + 1 , flag);
		}

		int recv_msg(char* recvBuf , int len , int flag = 0)
		{
			return recv(client_sock , recvBuf , len , flag);
		}


		bool connect_check(bool send_state , bool recv_state)
		{
			return (!(send_state && recv_state) && errno != EINTR);
		}

		void divide_name(const char* recvBuf , string& name , string& msg)
		{
			string buf = recvBuf;

			name = buf.substr(0, buf.find("/NA/"));
			msg = buf.substr(buf.find("/NA/") + 4, buf.size());
		}
};


void aux_set_ip(string& ip_buf, int& port)
{
	cout<<"input ip address of server: ";
	std::getline(std::cin, ip_buf);

	cout<<"input the port of server: ";
	std::cin>>port;
	std::cin.ignore();
}