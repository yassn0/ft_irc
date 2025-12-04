#pragma once

#include <string>
#include <vector>
#include <poll.h>
#include "CommandHandler.hpp"

class Client;

class Server
{
public:
	Server(const std::string &port, const std::string &pass);
	~Server();

	void start(); // lance la boucle principale

	const std::string& getPassword() const;

private:
	Server();
	Server(const Server &other);
	Server &operator=(const Server &other);

	void setupSocket();
	void acceptNewClient();
	void handleClientMessage(int client_fd);
	void removeClient(int client_fd);

	Client* getClientByFd(int fd);

	int _server_fd;                     // Socket serveur (le FD principal)
	int _port;
	std::string _password;
	std::vector<Client *> _clients;
	std::vector<struct pollfd> _poll_fds;  // Tableau pour poll()
	CommandHandler _commandHandler;
};
