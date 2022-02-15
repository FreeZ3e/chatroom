#pragma once

#include<WinSock2.h>
#include<string>
#include<iostream>
#include"client_file_transport.hpp"

using std::string;
using std::cout;
using std::endl;

class client_command
{
	private:
		file_transport file_t;

	public:
		bool send_command(const SOCKET& client_socket, const char* input, bool& exit_tag)
		{
			string com, op;
			get_command(input, com, op);

			if (com == "/help")
			{
				help();
				return true;
			}
			else if (com == "/exit")
			{
				exit_tag = true;
				return true;
			}
			else if (com == "/filesd")
			{
				int err = file_t.send_wrapper(client_socket, op);
				if (err != 0)
					cout << "send error: " << err << endl;
				else
					cout << "file send" << endl;

				return true;
			}
			else if (com == "/filept")
			{
				if(!file_t.set_path(op))
					cout << "path error" << endl;
				else
					cout << "path reset" << endl;

				return true;
			}

			return false;
		}

		bool recv_command(const char* input, const string& name)
		{
			string com, op;
			get_command(input, com, op);

			if (com == "/filerc")
			{
				if (op == name)
				{
					int err = file_t.recv_wrapper();
					if (err != 0)
						cout << "receive error: " << err << endl;
					else
						cout << "file recv" << endl;
				}
				return true;
			}
			else if (com == "/p")
			{
				string target_name, buf;
				divide_op(op, target_name, buf, "#");
				if (target_name == name)
				{
					string user_name, msg;
					divide_op(buf, user_name, msg, "/NA/");
					cout << "\n		" << user_name << ": " << msg << endl;
				}
				return true;
			}

			return false;
		}

	private:
		void help()
		{
			cout << "---------------------------------" << endl;
			cout << "commands:" << endl;
			cout << "		1. /accdel (user name): delete account" << endl;
			cout << "		2. /passwd (new passwd) : reset password" << endl;
			cout << "		3. /filesd (target user name) (file path): send file" << endl;
			cout << "		4. /filept (path) : change save path" << endl;
			cout << "		5. /p (target name)#msg : send msg to target user" << endl;
			cout << "		6. /exit : exit chatroom" << endl;
			cout << "---------------------------------" << endl;
		}

		
		void divide_op(const string& buf, string& name, string& msg, const char* d_tag)
		{
			name = buf.substr(0, buf.find(d_tag));
			msg = buf.substr(buf.find(d_tag) + strlen(d_tag), buf.size());
		}

		void get_command(const char* input, string& com, string& op)
		{
			string str(input);

			com = str.substr(0, str.find(" "));
			op = str.substr(str.find(" ") + 1, str.size());
		}
};