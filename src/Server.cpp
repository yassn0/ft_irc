#include "../inc/Server.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


Server::Server(const std::string &port, const std::string &pass)
	: _server_fd(-1), _port(0), _password(pass)
{
	_port = std::atoi(port.c_str());
	std::cout << "IRC Server started on port " << _port << std::endl;
	setupSocket();
}

Server::~Server()
{
	// Fermer tous les sockets clients
	for (size_t i = 1; i < _poll_fds.size(); i++)
		close(_poll_fds[i].fd);

	// Fermer le socket serveur
	if (_server_fd != -1)
		close(_server_fd);
}


void Server::setupSocket()
{

	// cree un socket (ipv4 et protocole tcp utilise)
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd < 0)
		throw std::runtime_error("Error: Failed to create socket");

	// option SO_REUSEADDR (évite "Address already in use"), on peut donc relancer immediatement
	int opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		close(_server_fd);
		throw std::runtime_error("Error: Failed to set SO_REUSEADDR");
	}

	// mettre le socket en mode NON-BLOQUANT pour que recv()/send() ne bloquent le serveur
	if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(_server_fd);
		throw std::runtime_error("Error: Failed to set non-blocking mode");
	}

	// preparer l'adresse du serveur
	struct sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(_port);

	// lier le socket à l'adresse
	if (bind(_server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		close(_server_fd);
		throw std::runtime_error("Error: Failed to bind socket (port already in use?)");
	}

	// mttre le socket en mode écoute
	// 10 = backlog (nombre max de connexions en attente)
	if (listen(_server_fd, 10) < 0)
	{
		close(_server_fd);
		throw std::runtime_error("Error: Failed to listen on socket");
	}

	// ajouter le socket serveur au tableau poll premier element de _poll_fds
	pollfd server_pollfd = {_server_fd, POLLIN, 0};
	// server_pollfd.fd = _server_fd;
	// server_pollfd.events = POLLIN; // On veut être notifié des nouvelles connexions
	// server_pollfd.revents = 0;	   // Sera rempli par poll()
	_poll_fds.push_back(server_pollfd);
}


void Server::start()
{
	while (true)
	{
		// poll() attend qu'il se passe quelque chose
		// &_poll_fds[0] = pointeur vers le premier élément du vector
		// _poll_fds.size() = nombre de FD à surveiller
		// -1 = timeout infini (attend pour toujours)
		if (poll(&_poll_fds[0], _poll_fds.size(), -1) < 0)
            throw std::runtime_error("Error while polling from fd!");
			
		// poll > 0 = activite
		// on parcourt tous les file descriptors
		for (size_t i = 0; i < _poll_fds.size(); i++)
		{
			// verifie si ce fd a de l'activite
			if (_poll_fds[i].revents & POLLIN)
			{
	
				if (_poll_fds[i].fd == _server_fd)
					acceptNewClient();
				else
					handleClientMessage(_poll_fds[i].fd);
			}

			if (_poll_fds[i].revents & POLLHUP)
			{
				removeClient(_poll_fds[i].fd);
				i--;
			}
		}
	}
}


// Accepte une nouvelle connexion
void Server::acceptNewClient()
{
	struct sockaddr_in client_addr = {};
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(_server_fd, (struct sockaddr *)&client_addr, &client_len);

	if (client_fd < 0)
		throw std::runtime_error("Error: Failed to accept client");

	// mettre le client en mode non-bloquant
	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(client_fd);
		throw std::runtime_error("Error: Failed to set client non-blocking");
	}

	// ajouter ce client au tableau poll
	struct pollfd client_pollfd = {client_fd, POLLIN, 0};
	_poll_fds.push_back(client_pollfd);

	// TODO: Créer un objet Client et l'ajouter à _clients
	// Client *new_client = new Client(client_fd);
	// _clients.push_back(new_client);
}

// Reçoit et traite un message
void Server::handleClientMessage(int client_fd)
{
	char buffer[512];

	// Recevoir les données
	ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes_read <= 0)
	{
		removeClient(client_fd);
		return;
	}

	buffer[bytes_read] = '\0';

	// TODO:
	// 1. Ajouter au buffer du Client (car peut être partiel!)
	// 2. Chercher \r\n pour détecter une commande complète
	// 3. Parser la commande (NICK, USER, JOIN, etc.)
	// 4. Exécuter la commande

	// Pour l'instant, on renvoie juste un echo
	std::string response = "Echo: ";
	response += buffer;
	send(client_fd, response.c_str(), response.length(), 0);
}

// Supprime un client déconnecté
void Server::removeClient(int client_fd)
{
	close(client_fd);

	for (size_t i = 0; i < _poll_fds.size(); i++)
	{
		if (_poll_fds[i].fd == client_fd)
		{
			_poll_fds.erase(_poll_fds.begin() + i);
			break;
		}
	}

	// TODO: Retirer de _clients et delete l'objet Client
}
