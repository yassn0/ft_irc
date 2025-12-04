#pragma once

#include <string>
#include <vector>
#include <poll.h>

class Client;
class CommandHandler;

class Server
{
public:
	Server(const std::string &port, const std::string &pass);
	~Server();

	void start(); // lance la boucle principale

	const std::string& getPassword() const;

private:
	// Interdire la copie (pattern Coplien C++98)
	Server();
	Server(const Server &other);
	Server &operator=(const Server &other);

	// Méthodes privées pour gérer le serveur
	void setupSocket();                       // Crée et configure le socket serveur
	void acceptNewClient();                   // Accepte une nouvelle connexion
	void handleClientMessage(int client_fd);  // Traite un message reçu
	void removeClient(int client_fd);         // Supprime un client déconnecté

	// Utilitaires
	Client* getClientByFd(int fd);            // Trouve un client par son FD

	// Attributs privés
	int _server_fd;                     // Socket serveur (le FD principal)
	int _port;                          // Port d'écoute (ex: 6667)
	std::string _password;              // Mot de passe du serveur
	std::vector<Client *> _clients;     // Liste des clients connectés
	std::vector<struct pollfd> _poll_fds;  // Tableau pour poll()
	CommandHandler* _commandHandler;    // Gestionnaire de commandes IRC
};
