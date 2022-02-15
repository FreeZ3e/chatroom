#pragma once

#include<fstream>
#include<string>
#include<vector>

using std::vector;
using std::string;
using std::ifstream;
using std::ofstream;
using std::getline;

class chatroom_account
{
	struct account
	{
		string user;
		string password;
		bool login_tag = 0;

		account(const string& u, const string& p) :user(u), password(p)
		{ }
	};

	public:
		vector<account> arr;

		ofstream writer;
		ifstream reader;

		string PATH = "D:/vs2019 project/Socket_ChatRoom/account.txt";

	public:
		chatroom_account()
		{
			load_account();
		}

		int account_server(const string& user, const string& passwd)
		{
			int staute = verify(user, passwd);
			if (staute == 2)
				create_account(user, passwd);

			return staute;
		}

		bool delete_account(const string& user)
		{
			auto pos = arr.begin();
			for (; pos != arr.end();)
			{
				if ((*pos).user == user)
				{
					pos = arr.erase(pos);

					save_account();
					return true;
				}
				else
					++pos;
			}

			return false;
		}

		bool reset_passwd(const string& user, const string& passwd)
		{
			for (auto& p : arr)
			{
				if (p.user == user)
				{
					p.password = passwd;

					save_account();
					return true;
				}
			}

			return false;
		}

		bool logout(const string& user)
		{
			for (auto& p : arr)
			{
				if (p.user == user)
				{
					p.login_tag = 0;
					return true;
				}
			}

			return false;
		}

	private:
		//verify success = 0
		//passwd wrong = 1
		//user not exist = 2
		//user are login = 3
		int verify(const string& user, const string& passwd)
		{
			for (auto& p : arr)
			{
				if (p.user == user)
				{
					if (p.password == passwd)
					{
						if (p.login_tag == 0)
						{
							p.login_tag = 1;
							return 0;
						}
						else
							return 3;
					}
					else
						return 1;
				}
			}

			return 2;
		}

		//save account each create
		void create_account(const string& user, const string& passwd)
		{
			arr.push_back(account(user, passwd));
			save_account();
		}

		void save_account()
		{
			writer.open(PATH);

			for (auto p : arr)
			{
				writer << p.user << ":" << p.password << "\n";
			}

			writer.flush();
			writer.close();
		}

		void load_account()
		{
			reader.open(PATH);

			string line;
			string user;
			string passwd;

			while (getline(reader, line))
			{
				get_account(line, user, passwd);
				if (user.empty() || passwd.empty())
					continue;

				arr.push_back(account(user, passwd));
			}

			reader.close();
		}

		void get_account(const string& line, string& user, string& wd)
		{
			if (line.empty() || line.find(":") == string::npos)
				return;

			user = line.substr(0, line.find(":"));
			wd = line.substr(line.find(":") + 1, line.size());
		}
};