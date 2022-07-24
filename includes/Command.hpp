#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <vector>
#include <iostream>

std::vector<std::string> split(std::string s, std::string delimiter);

namespace irc
{
	class Command
	{
		private:
			std::string _message;
			std::string _prefix;
			std::vector<std::string> _params;
			std::string _trailer;
		public:
			Command(std::string message);
			~Command();

			std::string getPrefix();
			std::string getTrailer();
			std::vector<std::string> getParams();
	};
}
#endif