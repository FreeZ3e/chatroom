#pragma once

#include<WinSock2.h>
#include<WS2tcpip.h>
#include<vector>

using std::vector;

class transport_base
{
	public:
		bool create_send_socket(SOCKET& send_socket, sockaddr_in& send_in)
		{
			send_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (send_socket == INVALID_SOCKET)
				return false;

			send_in.sin_family = AF_INET;
			send_in.sin_port = htons(8889);
			send_in.sin_addr.s_addr = INADDR_ANY;

			if (bind(send_socket, (sockaddr*)&send_in, sizeof(send_in)) == -1)
				return false;

			if (listen(send_socket, 5) == -1)
				return false;

			return true;
		}

		bool send_accpet(SOCKET& send_socket, SOCKET& recv_socket, sockaddr_in& recv_in)
		{
			int len = sizeof(recv_in);
			recv_socket = accept(send_socket, (sockaddr*)&recv_in, &len);

			if (recv_socket == INVALID_SOCKET)
				return false;
			return true;
		}

		void send_socket_close(SOCKET& tran_socket, SOCKET& send_socket, SOCKET& recv_socket)
		{
			closesocket(tran_socket);
			closesocket(send_socket);
			closesocket(recv_socket);
		}
};


class file_transport : private transport_base
{
	public:
		int transport(const SOCKET& client_socket, const vector<SOCKET>& user,
						const char* input)
		{
			SOCKET tran_socket;
			sockaddr_in tran_in;

			if(!create_send_socket(tran_socket, tran_in))
				return 1;

			SOCKET send_socket;
			SOCKET recv_socket;
			sockaddr_in send_in;
			sockaddr_in recv_in;

			if (!send_accpet(tran_socket, send_socket, send_in))
				return 2;

			send_all(client_socket, user, input);

			if (!send_accpet(tran_socket, recv_socket, recv_in))
				return 3;

			if (!filename_tran(send_socket, recv_socket))
				return 4;
			if (!ack_tran(send_socket, recv_socket))
				return 5;

			file_tran(send_socket, recv_socket);

			send_socket_close(tran_socket, send_socket, recv_socket);
			return 0;
		}

		
	private:
		bool filename_tran(const SOCKET& send_socket, const SOCKET& recv_socket)
		{
			char name_buf[1024] = { 0 };
			int len = recv(send_socket, name_buf, 1024, 0);
			if (len > 0)
			{
				if (send(recv_socket, name_buf, len, 0) > 0)
					return true;
			}

			return false;
		}

		bool ack_tran(const SOCKET& send_socket, const SOCKET& recv_socket)
		{
			char ack_buf[1024] = { 0 };
			int len = recv(recv_socket, ack_buf, 1024, 0);
			if (len > 0)
			{
				if (send(send_socket, ack_buf, len, 0) > 0)
					return true;
			}

			return false;
		}

		void file_tran(const SOCKET& send_socket, const SOCKET& recv_socket)
		{
			while (1)
			{
				char file_buf[1024] = { 0 };
				int len = recv(send_socket, file_buf, 1024, 0);
				if (len <= 0)
					break;

				len = send(recv_socket, file_buf, len, 0);
				if (len <= 0)
					break;
			}
		}


		void send_all(const SOCKET& client_socket, const vector<SOCKET>& user, 
						const char* msg)
		{
			for (size_t n = 0; n < user.size(); ++n)
			{
				if (user[n] != INVALID_SOCKET && client_socket != user[n])
					send_msg(user[n], msg);
			}
		}

		int send_msg(const SOCKET& client_socket, const char* msg, int flag = 0)
		{
			return send(client_socket, msg, strlen(msg) + 1, flag);
		}
};