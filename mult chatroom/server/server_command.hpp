#pragma once

#include<WinSock2.h>
#include<string>
#include<iostream>
#include"file_transport.hpp"
#include"server_account.hpp"

using std::string;
using std::cout;
using std::endl;

class server_command
{
	private:
		file_transport file_t;

	public:
		bool command(const SOCKET& client_socket, server_account& account, 
					const char* input, const string& name, 
					const vector<SOCKET>& user, const vector<string>& user_name)
		{
			string com, op;
			get_command(input, com, op);

			if (com == "/accdel")
			{
				string msg = "user not exist";

				if(account.account_del(name))
					msg = "account deleted";

				send_wrapper(client_socket, msg);
				cout << msg << endl;

				return true;
			}
			else if (com == "/passwd")
			{
				string msg = "reset failed, check the user name";

				if (account.reset_passwd(name, op))
					msg = "password reset";

				send_wrapper(client_socket, msg);
				cout << msg << endl;

				return true;
			}
			else if (com == "/filerc")
			{
				if (target_name_check(user_name, op, name))
				{
					int err = file_t.transport(client_socket, user, input);
					if (err != 0)
						cout << "file transport err: " << err << endl;
					else
						cout << "transport success" << endl;
				}
				else
					cout << "user name error" << endl;

				return true;
			}
			else if (com == "/p")
			{
				string msg = insert_user_name(input, name);
				send_all(client_socket, user, msg.c_str());
				return true;
			}

			return false;
		}

	private:
		void get_command(const char* input, string& com, string& op)
		{
			string str(input);

			com = str.substr(0, str.find(" "));
			op = str.substr(str.find(" ") + 1, str.size());
		}

		bool target_name_check(const vector<string>& user_name, const string& target_name,
								const string& self_name)
		{
			for (auto p : user_name)
			{
				if (p == target_name && p != self_name)
					return true;
			}

			return false;
		}

		string insert_user_name(const char* input, const string& name)
		{
			string temp = input;
			
			string msg = temp.substr(0, temp.find("#") + 1);
			msg = msg + name + "/NA/";
			msg += temp.substr(temp.find("#") + 1, temp.size());

			return msg;
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

		void send_wrapper(const SOCKET& client_socket, const string& msg_buf)
		{
			string msg = "server/NA/" + msg_buf;
			send_msg(client_socket, msg.c_str());
		}

		int send_msg(const SOCKET& client_socket, const char* msg, int flag = 0)
		{
			return send(client_socket, msg, strlen(msg) + 1, flag);
		}
};