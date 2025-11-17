#pragma once

#include <string>

class Server
{
public:
	Server(const std::string &port, const std::string &pass);
	~Server();

private:
	Server();
	
};
