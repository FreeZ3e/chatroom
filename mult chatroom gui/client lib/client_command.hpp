#pragma once

#include<WinSock2.h>
#include<string>
#include"client_file_transport.hpp"

using std::string;

class client_command
{
	private:
		file_transport file_t;

	public:
		void set_ip(const char* ip)
		{
			file_t.set_ip(ip);
		}

		bool send_command(const SOCKET& client_socket, const char* input)
		{
			string com, op;
			get_command(input, com, op);

			if (com == "/filesd")
			{
				int err = file_t.send_wrapper(client_socket, op);
				if (err != 0)
					return false;

				return true;
			}
			else if (com == "/filept")
			{
				if (!file_t.set_path(op))
					return false;

				return true;
			}

			return false;
		}

		bool recv_command(const char* input, const string& name, string& out_msg)
		{
			string com, op;
			get_command(input, com, op);

			if (com == "/filerc")
			{
				if (op == name)
				{
					int err = file_t.recv_wrapper();
					if (err != 0)
						out_msg = "\n		receive error: " + (err + 48);
					else
						out_msg = "\n		file recv";
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
					out_msg = "\n		" + user_name + ": " + msg;
				}

				return true;
			}

			return false;
		}

	private:		
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