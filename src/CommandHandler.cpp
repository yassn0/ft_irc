#include "../inc/CommandHandler.hpp"
#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include <iostream>

#define ERR_NONICKNAMEGIVEN(client) (":server 431 " + std::string(client) + " :No nickname given\r\n")
#define ERR_NEEDMOREPARAMS(client, cmd) (":server 461 " + std::string(client) + " " + std::string(cmd) + " :Not enough parameters\r\n")
#define ERR_ALREADYREGISTRED(client) (":server 462 " + std::string(client) + " :You may not reregister\r\n")
#define ERR_PASSWDMISMATCH(client) (":server 464 " + std::string(client) + " :Password incorrect\r\n")

CommandHandler::CommandHandler(Server *server) : _server(server)
{
}

CommandHandler::~CommandHandler()
{
}

void CommandHandler::execute(Client *client, const std::string &command)
{
	std::cout << "Command from fd " << client->getFd() << ": " << command << std::endl;

	// extraire le nom de la commande et les paramètres
	size_t pos = command.find(' ');
	std::string cmd = (pos != std::string::npos) ? command.substr(0, pos) : command;
	std::string params = (pos != std::string::npos) ? command.substr(pos + 1) : "";

	if (cmd == "PING")
		handlePing(client, params);
	else if (cmd == "PASS")
		handlePass(client, params);
	else if (cmd == "NICK")
		handleNick(client, params);
	else if (cmd == "USER")
		handleUser(client, params);
	else if (cmd == "JOIN")
		handleJoin(client, params);
	else if (cmd == "PRIVMSG")
		handlePrivmsg(client, params);
	else
		std::cout << "Unknown command: " << cmd << std::endl;
}

void CommandHandler::handlePing(Client *client, const std::string &params)
{
	std::string response = ":server PONG server " + params + "\r\n";
	client->sendMessage(response);
}

void CommandHandler::handlePass(Client *client, const std::string &params)
{
	if (params.empty())
	{
		client->sendMessage(ERR_NEEDMOREPARAMS("*", "PASS"));
		return;
	}

	if (client->isRegistered())
	{
		client->sendMessage(ERR_ALREADYREGISTRED(client->getNickname()));
		return;
	}

	if (params == _server->getPassword())
		client->setAuthenticated(true);
	else
		client->sendMessage(ERR_PASSWDMISMATCH("*"));
}

void CommandHandler::handleNick(Client *client, const std::string &params)
{
	if (params.empty())
	{
		client->sendMessage(ERR_NONICKNAMEGIVEN("*"));
		return;
	}

	std::string nickname = params.substr(0, params.find(' '));

	// Vérifier si le nickname est déjà utilisé (on pourra implémenter ça plus tard avec une liste)
	// Pour l'instant, on accepte tous les nicknames non vides

	client->setNickname(nickname);
}

void CommandHandler::handleUser(Client *client, const std::string &params)
{
	if (params.empty())
	{
		client->sendMessage(ERR_NEEDMOREPARAMS("*", "USER"));
		return;
	}

	if (client->isRegistered())
	{
		client->sendMessage(ERR_ALREADYREGISTRED(client->getNickname()));
		return;
	}

	// format: USER username 0 * :realname
	size_t space = params.find(' ');
	if (space != std::string::npos)
	{
		std::string username = params.substr(0, space);
		client->setUsername(username);

		if (client->isAuthenticated() && !client->getNickname().empty())
		{
			client->setRegistered(true);
			sendWelcomeMessages(client);
		}
	}
	else
		client->sendMessage(ERR_NEEDMOREPARAMS("*", "USER"));
}

// JOIN - Rejoindre un channel (à implémenter)
void CommandHandler::handleJoin(Client *client, const std::string &params)
{
	(void)client;
	(void)params;
	std::cout << "JOIN not implemented yet" << std::endl;
	
	// if (params.empty())
    // {
	// 	client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "USER"));
    //     return;
    // }

}

// PRIVMSG - Envoyer un message (à implémenter)
void CommandHandler::handlePrivmsg(Client *client, const std::string &params)
{
	(void)client;
	(void)params;
	std::cout << "PRIVMSG not implemented yet" << std::endl;
}

// Envoie les messages de bienvenue (codes 001-004)
void CommandHandler::sendWelcomeMessages(Client *client)
{
	std::string nick = client->getNickname();
	client->sendMessage(":server 001 " + nick + " :Welcome to the IRC Network " + nick + "\r\n");
	client->sendMessage(":server 002 " + nick + " :Your host is server, running version 1.0\r\n");
	client->sendMessage(":server 003 " + nick + " :This server was created 2025-01-01\r\n");
	client->sendMessage(":server 004 " + nick + " server 1.0 io itkol\r\n");
}
