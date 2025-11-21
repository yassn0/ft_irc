#pragma once

#include <string>
#include <vector>
#include <poll.h>

class Client;

class Server
{
public:
	Server(const std::string &port, const std::string &pass);
	~Server();

	void start(); // lance la boucle principale

private:
	// interdire la copie
	Server();
	Server(const std::string &port);
	Server(const Server &other);
	Server &operator=(const Server &other);

	int _server_fd;					// Socket serveur
	int _port;						// Port d'écoute
	std::string _password;			// Mot de passe du serveur
	std::vector<Client *> _clients; 		// Liste des clients connectés
	std::vector<struct pollfd> _poll_fds;	// Pour poll()
};
