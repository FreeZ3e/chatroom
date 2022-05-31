#pragma once
#pragma comment(lib, "ws2_32.lib")

#include<winsock2.h>
#include<WS2tcpip.h>
#include<string>
#include"client_account.hpp"
#include"client_command.hpp"
#include"error_log.hpp"

using std::string;


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
				error_log::log(err_tag);
		}

		~chatroom_base()
		{
			closesocket(client_sock);
			WSACleanup();
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

		string name;

	public:
		chatroom_client(int af , int type , int protocol , int port , 
						const char* ip , unsigned int main_ver = 2 , 
						unsigned int ver = 2) :
			chatroom_base(af , type , protocol , port , ip , main_ver , ver)
		{
			com.set_ip(ip);
		}

		int login(const string& user_name, const string& passwd)
		{
			int res = account._login(client_sock, user_name, passwd);
			if (res == 0 || res == 2)
				name = user_name;

			return res;
		}

		bool send_wrapper(const string& msg)
		{
			return send_msg(msg.c_str()) > 0;
		}

		string recv_wrapper()
		{
			char recvBuf[1024];
			int len = recv_msg(recvBuf, 1024);

			string out_msg;
			if (com.recv_command(recvBuf, name, out_msg))
				return out_msg;

			if (len > 0)
			{
				string name, msg;
				divide_name(recvBuf, name, msg);
				string res = name + ": " + msg;

				return res;
			}
			
			return "error";
		}

		bool call_command(const string& msg)
		{
			return com.send_command(client_sock, msg.c_str());
		}

		string recv_history()
		{
			char recvBuf[1024];
			if (recv_msg(recvBuf, 1024) < 0)
				return "/hisend";

			string msg(recvBuf);
			return msg;
		}

		string recv_online_user()
		{
			char recvBuf[1024];
			if (recv_msg(recvBuf, 1024) < 0)
				return "/urend";

			string msg(recvBuf);
			return msg;
		}

	private:
		int send_msg(const char* msg , int flag = 0)
		{
			return send(client_sock , msg , strlen(msg) + 1 , flag);
		}

		int recv_msg(char* recvBuf , int len , int flag = 0)
		{
			return recv(client_sock , recvBuf , len , flag);
		}

		void divide_name(const char* recvBuf , string& name , string& msg)
		{
			string buf = recvBuf;

			name = buf.substr(0, buf.find("/NA/"));
			msg = buf.substr(buf.find("/NA/") + 4, buf.size());
		}
};