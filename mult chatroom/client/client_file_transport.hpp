#pragma once

#include<winsock2.h>
#include<WS2tcpip.h>
#include<string>
#include<fstream>
#include<sys/stat.h>

using std::string;
using std::ifstream;
using std::ofstream;


class transport_base
{
	public:
		bool create_socket(SOCKET& recv_socket, sockaddr_in& recv_in)
		{
			recv_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (recv_socket == INVALID_SOCKET)
				return false;

			recv_in.sin_family = AF_INET;
			recv_in.sin_port = htons(8889);
			int ip_err = inet_pton(AF_INET, "127.0.0.1", &recv_in.sin_addr);

			if (ip_err != 1)
				return false;

			return connect(recv_socket, (sockaddr*)&recv_in, sizeof(sockaddr_in)) != -1;
		}
};


class file_transport : private transport_base
{
	private:
		string PATH = "D:/vs2019 project/Socket_ChatRoom";
		string recv_path;

	public:
		bool set_path(const string& path)
		{
			if (path_check(path))
			{
				PATH = path;
				return true;
			}

			return false;
		}

		int send_wrapper(const SOCKET& client_socket, const string& op)
		{
			string target_name = op.substr(0, op.find_last_of(" "));
			string path = op.substr(op.find_last_of(" ") + 1, op.size());

			string name = path.substr(path.find_last_of("/"), path.size());

			if (!file_check(path))
				return -1;

			if (!send_signal(client_socket, target_name))
				return 1;

			SOCKET send_socket = INVALID_SOCKET;
			sockaddr_in send_in;

			if (!create_socket(send_socket, send_in))
				return 2;

			if (!send_filename(send_socket, name))
				return 3;

			if (!recv_ack(send_socket))
				return 4;

			if (!send_file(send_socket, path))
				return 5;

			closesocket(send_socket);
			return 0;
		}

		int recv_wrapper()
		{
			SOCKET recv_socket = INVALID_SOCKET;
			sockaddr_in recv_in;

			if (!create_socket(recv_socket, recv_in))
				return 1;

			if (!recv_filename(recv_socket))
				return 2;

			if (!send_ack(recv_socket))
				return 3;

			if (!recv_file(recv_socket))
				return 4;

			closesocket(recv_socket);
			return 0;
		}

	private:
		bool send_signal(const SOCKET& client_socket, const string& target_name)
		{
			const string signal = "/filerc " + target_name;
			return send(client_socket, signal.c_str(), signal.size() + 1, 0) >= 0;
		}

		bool send_filename(const SOCKET& client_socket, const string& name)
		{
			return send(client_socket, name.c_str(), name.size() + 1, 0) > 0;
		}

		bool recv_filename(const SOCKET& client_socket)
		{
			char filename[256];
			int len = recv(client_socket, filename, 256, 0);

			if (len > 0)
			{
				string temp(filename, len - 1);
				recv_path = PATH + temp;
			}
			else
				return false;

			return true;
		}

		bool send_ack(const SOCKET& client_socket)
		{
			string ack = "/ack";
			return send(client_socket, ack.c_str(), ack.size() + 1, 0) > 0;
		}

		bool recv_ack(const SOCKET& client_socket)
		{
			char ack_buf[1024] = { 0 };
			int len = recv(client_socket, ack_buf, 1024, 0);
			if (len > 0)
			{
				string temp(ack_buf, len - 1);
				return temp == "/ack";
			}

			return false;
		}

		bool send_file(const SOCKET& client_socket, const string& path)
		{
			char file_buf[1024] = { 0 };
			ifstream file;

			file.open(path, std::ios::binary);
			if (!file)
				return false;

			while (!file.eof())
			{
				file.read(file_buf, 1024);

				int len = file.gcount();
				if (send(client_socket, file_buf, len, 0) < 0) //send file
					return false;
			}


			file.close();
			return true;
		}

		bool recv_file(const SOCKET& client_socket)
		{
			char file_buf[1024] = { 0 };
			ofstream file;

			file.open(recv_path, std::ios::binary);
			if (!file)
				return false;

			while (1)
			{
				int len = recv(client_socket, file_buf, 1024, 0);
				if (len > 0)
					file.write(file_buf, len);
				else
					break;
			}


			file.flush();
			file.close();
			return true;
		}


		bool path_check(const string& path)
		{
			struct _stat fileStat;
			if ((_stat(path.c_str(), &fileStat) == 0) && (fileStat.st_mode & _S_IFDIR))
				return true;

			return false;
		}

		bool file_check(const string& path)
		{
			ifstream fin(path);
			if (!fin)
				return false;

			return true;
		}
};