#include "../inc/CommandHandler.hpp"
#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include <iostream>

CommandHandler::CommandHandler(Server* server) : _server(server)
{
}

CommandHandler::~CommandHandler()
{
}

void CommandHandler::execute(Client* client, const std::string& command)
{
	std::cout << "Command from fd " << client->getFd() << ": " << command << std::endl;

	// extraire le nom de la commande et les paramètres
	size_t pos = command.find(' ');
	std::string cmd = (pos != std::string::npos) ? command.substr(0, pos) : command;
	std::string params = (pos != std::string::npos) ? command.substr(pos + 1) : "";

	// Dispatcher vers le bon handler
	if (cmd == "PING")
		handlePing(client, params);
	else if (cmd == "PASS")
		handlePass(client, params);
	else if (cmd == "NICK")
		handleNick(client, params);
	else if (cmd == "USER")
		handleUser(client, params);
	else if (cmd == "CAP")
		handleCap(client, params);
	else if (cmd == "JOIN")
		handleJoin(client, params);
	else if (cmd == "PRIVMSG")
		handlePrivmsg(client, params);
	else
		std::cout << "Unknown command: " << cmd << std::endl;
}


// PING - Critique pour irssi! Répond PONG pour éviter le timeout
void CommandHandler::handlePing(Client* client, const std::string& params)
{
	std::string response = ":server PONG server " + params + "\r\n";
	client->sendMessage(response);
}

void CommandHandler::handlePass(Client* client, const std::string& params)
{
	if (params == _server->getPassword())
		client->setAuthenticated(true);
}

void CommandHandler::handleNick(Client* client, const std::string& params)
{
	client->setNickname(params);
}

void CommandHandler::handleUser(Client* client, const std::string& params)
{
	// Format: USER username 0 * :realname
	size_t space = params.find(' ');
	if (space != std::string::npos)
	{
		std::string username = params.substr(0, space);
		client->setUsername(username);

		// Si authentifié et a un nickname → client registered
		if (client->isAuthenticated() && !client->getNickname().empty())
		{
			client->setRegistered(true);
			sendWelcomeMessages(client);
		}
	}
}

// CAP - Réponse aux capacités (irssi l'envoie automatiquement)
void CommandHandler::handleCap(Client* client, const std::string& params)
{
	(void)params;
	client->sendMessage(":server CAP * LS :\r\n");
}

// JOIN - Rejoindre un channel (à implémenter)
void CommandHandler::handleJoin(Client* client, const std::string& params)
{
	(void)client;
	(void)params;
	std::cout << "JOIN not implemented yet" << std::endl;
}

// PRIVMSG - Envoyer un message (à implémenter)
void CommandHandler::handlePrivmsg(Client* client, const std::string& params)
{
	(void)client;
	(void)params;
	std::cout << "PRIVMSG not implemented yet" << std::endl;
}


// Envoie les messages de bienvenue (codes 001-004)
void CommandHandler::sendWelcomeMessages(Client* client)
{
	std::string nick = client->getNickname();
	client->sendMessage(":server 001 " + nick + " :Welcome to the IRC Network " + nick + "\r\n");
	client->sendMessage(":server 002 " + nick + " :Your host is server, running version 1.0\r\n");
	client->sendMessage(":server 003 " + nick + " :This server was created 2025-01-01\r\n");
	client->sendMessage(":server 004 " + nick + " server 1.0 io itkol\r\n");
}
