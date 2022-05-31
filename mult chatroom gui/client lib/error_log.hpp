#pragma once

#include<fstream>
using std::ofstream;

class error_log
{
	public:
		static void log(unsigned int err_tag)
		{
			ofstream writer("./error_log.txt");
			writer << "the error number: " << err_tag;

			writer.flush();
			writer.close();
		}
};