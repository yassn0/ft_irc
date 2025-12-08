#include "../inc/Server.hpp"
#include "../inc/Client.hpp"
#include "../inc/CommandHandler.hpp"
#include "../inc/Channel.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <csignal>

bool g_shutdown = false;

void signalHandler(int signal)
{
	(void)signal;
	g_shutdown = true;
}

Server::Server(const std::string &port, const std::string &pass) : _server_fd(-1), _port(0), _password(pass), _commandHandler(this)
{
	_port = std::atoi(port.c_str());
	std::cout << "IRC Server started on port " << _port << std::endl;

	std::signal(SIGINT, signalHandler);
	std::signal(SIGTERM, signalHandler);

	setupSocket();
}

Server::~Server()
{
	// supprimer tous les channels
	for (size_t i = 0; i < _channels.size(); i++)
		delete _channels[i];
	_channels.clear();

	// fermer et supprimer tous les clients restants
	// On fait une copie du vecteur pour éviter les problèmes d'itération
	std::vector<int> client_fds;
	for (size_t i = 0; i < _clients.size(); i++)
		client_fds.push_back(_clients[i]->getFd());

	for (size_t i = 0; i < client_fds.size(); i++)
		removeClient(client_fds[i]);

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
	if (listen(_server_fd, 999) < 0)
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
	std::cout << "Server running. Press Ctrl+C to stop." << std::endl;

	while (!g_shutdown)
	{
		// poll() attend qu'il se passe quelque chose
		// &_poll_fds[0] = pointeur vers le socket serveur
		// _poll_fds.size() = nombre de FD à surveiller
		// -1 = timeout infini
		int poll_count = poll(&_poll_fds[0], _poll_fds.size(), -1);

		// si poll() est interrompu par un signal, on continue (ou on sort si g_shutdown est mis)
		if (poll_count < 0)
		{
			if (g_shutdown)
				break;
			continue;
		}

		// poll > 0 = activite
		// on parcourt tous les file descriptors en sens inverse pour éviter les problèmes de suppression
		for (size_t i = _poll_fds.size(); i > 0; i--)
		{
			size_t idx = i - 1;

			// verifie si ce fd a de l'activite
			if (_poll_fds[idx].revents & POLLIN)
			{
				if (_poll_fds[idx].fd == _server_fd)
					acceptNewClient();
				else
					handleClientMessage(_poll_fds[idx].fd);
			}

			if (_poll_fds[idx].revents & POLLHUP)
				removeClient(_poll_fds[idx].fd);
		}

		// vérifier si des clients doivent être déconnectés (après QUIT par exemple)
		for (size_t i = _clients.size(); i > 0; i--)
		{
			size_t idx = i - 1;
			if (_clients[idx]->shouldDisconnect())
				removeClient(_clients[idx]->getFd());
		}
	}

	std::cout << "Server shutting down..." << std::endl;
}

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

	struct pollfd client_pollfd = {client_fd, POLLIN, 0};
	_poll_fds.push_back(client_pollfd);

	Client *new_client = new Client(client_fd);
	_clients.push_back(new_client);
}

void Server::handleClientMessage(int client_fd)
{
	char buffer[1024];

	memset(buffer, 0, sizeof(buffer));

	// recevoir les données
	ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes_read <= 0)
	{
		std::cout << "Client < " << client_fd << "> Disconnected" << std::endl;
		removeClient(client_fd);
		return;
	}

	buffer[bytes_read] = '\0';

	Client *client = getClientByFd(client_fd);
	if (!client)
		return;

	// ajoute les données au buffer du client
	client->appendToBuffer(std::string(buffer, bytes_read));

	// chercher des commandes complètes (terminées par \r\n)
	std::string buf = client->getBuffer();
	size_t pos;

	while ((pos = buf.find("\r\n")) != std::string::npos)
	{
		// extraire la commande
		std::string command = buf.substr(0, pos);
		buf.erase(0, pos + 2); // Enlever la commande + \r\n

		// traiter la commande via CommandHandler
		if (!command.empty())
			_commandHandler.execute(client, command);
	}

	// mettre à jour le buffer du client
	client->clearBuffer();
	client->appendToBuffer(buf);
}

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

	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i]->getFd() == client_fd)
		{
			delete _clients[i];
			_clients.erase(_clients.begin() + i);
			break;
		}
	}
}

Client *Server::getClientByFd(int fd)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i]->getFd() == fd)
			return _clients[i];
	}
	return NULL;
}

const std::string &Server::getPassword() const
{
	return _password;
}

Channel *Server::getChannelByName(const std::string &name)
{
	for (size_t i = 0; i < _channels.size(); i++)
	{
		if (_channels[i]->get_name() == name)
			return _channels[i];
	}
	return NULL;
}

Channel *Server::createChannel(const std::string &name, const std::string &key, Client *creator)
{
	Channel *channel = new Channel(name, key, creator);
	_channels.push_back(channel);
	channel->add_client(creator);
	return channel;
}

std::vector<Client *> Server::getAllClients() const
{
	return _clients;
}

std::vector<Channel *> Server::getAllChannels() const
{
	return _channels;
}
