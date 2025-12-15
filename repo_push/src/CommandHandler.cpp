#include "../inc/CommandHandler.hpp"
#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"
#include "../inc/IRCReplies.hpp"
#include <iostream>

CommandHandler::CommandHandler(Server *server) : _server(server)
{
}

CommandHandler::~CommandHandler()
{
}

void CommandHandler::execute(Client *client, const std::string &command)
{
	std::cout << "Command from " << client->getUsername() << ": " << command << std::endl;

	// extraire le nom de la commande et les paramètres
	size_t pos = command.find(' ');
	std::string cmd = (pos != std::string::npos) ? command.substr(0, pos) : command;
	std::string params = (pos != std::string::npos) ? command.substr(pos + 1) : "";

	std::string commands[14] = {"CAP", "PING", "PASS", "NICK", "USER", "JOIN", "PRIVMSG", "PART", "QUIT", "KICK", "INVITE", "TOPIC", "MODE", "NOTICE"};

	void (CommandHandler::*handlers[14])(Client *, const std::string &) = {
		&CommandHandler::handleCap,
		&CommandHandler::handlePing,
		&CommandHandler::handlePass,
		&CommandHandler::handleNick,
		&CommandHandler::handleUser,
		&CommandHandler::handleJoin,
		&CommandHandler::handlePrivmsg,
		&CommandHandler::handlePart,
		&CommandHandler::handleQuit,
		&CommandHandler::handleKick,
		&CommandHandler::handleInvite,
		&CommandHandler::handleTopic,
		&CommandHandler::handleMode,
		&CommandHandler::handleNotice
	};

	for (int i = 0; i < 14; i++)
	{
		if (cmd == commands[i])
		{
			(this->*handlers[i])(client, params);
			return;
		}
	}

	std::cout << "Unknown command: " << cmd << std::endl;
}

void CommandHandler::sendWelcomeMessages(Client *client)
{
	std::string nick = client->getNickname();
	client->sendMessage(":server 001 " + nick + " :Welcome to the IRC Network " + nick + "\r\n");
	client->sendMessage(":server 002 " + nick + " :Your host is server, running version 1.0\r\n");
	client->sendMessage(":server 003 " + nick + " :This server was created 2025-01-01\r\n");
	client->sendMessage(":server 004 " + nick + " server 1.0 io itkol\r\n");
}

// Helper functions
Client* CommandHandler::findClientByNickname(const std::string& nickname)
{
	std::vector<Client*> allClients = _server->getAllClients();
	for (size_t i = 0; i < allClients.size(); i++)
	{
		if (allClients[i]->getNickname() == nickname)
			return allClients[i];
	}
	return NULL;
}

Channel* CommandHandler::getChannel(const std::string& channelName)
{
	return _server->getChannelByName(channelName);
}
