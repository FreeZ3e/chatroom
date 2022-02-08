#pragma once

#include"client_file_transport.hpp"
#include<WinSock2.h>
#include<string>
#include<iostream>

using std::string;
using std::cout;
using std::endl;

class client_command
{
	private:
		file_transport file_t;

	public:
		bool send_command(const SOCKET& client_socket , const char* input)
		{
			string com , op;
			get_command(input , com , op);

			if (com == "/filesd")//send file
			{
				int err_num = file_t.send_wrapper(client_socket , op);
				if (err_num == 0)
					cout << "file send" << endl;
				else
					cout << "send failed, error num: " << err_num << endl;
				return true;
			}
			else if (com == "/filept")
			{
				if (file_t.set_path(op))
					cout << "path reset" << endl;
				else
					cout << "path error" << endl;
				return true;
			}
			else if (com == "/help")
			{
				help();
				return true;
			}

			return false;//not command
		}

		bool recv_command(const char* input)
		{
			string com , op;
			get_command(input , com , op);

			if (com == "/filerc")//recv file
			{
				int err_num = file_t.recv_wrapper();
				if (err_num == 0)
					cout << "file recv" << endl;
				else
					cout << "receive failed, error num: " << err_num << endl;
				return true;
			}

			return false;
		}

	private:
		void help()
		{
			cout << "---------------------------------" << endl;
			cout << "commands:" << endl;
			cout << "		1. /filesd (file path) : send file" << endl;
			cout << "		2. /filept (path) : change save path" << endl;
			cout << "		3. /passwd (new passwd) : reset password" << endl;
			cout << "		4. /exit : exit chatroom" << endl;
			cout << "---------------------------------" << endl;
		}
		
		void get_command(const char* input , string& com , string& op)
		{
			string str(input);

			com = str.substr(0 , str.find(" "));
			op = str.substr(str.find(" ") + 1 , str.size());
		}
};