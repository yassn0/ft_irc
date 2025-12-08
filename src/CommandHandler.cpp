#include "../inc/CommandHandler.hpp"
#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"
#include <iostream>

#define ERR_NONICKNAMEGIVEN(client) (":server 431 " + std::string(client) + " :No nickname given\r\n")
#define ERR_NEEDMOREPARAMS(client, cmd) (":server 461 " + std::string(client) + " " + std::string(cmd) + " :Not enough parameters\r\n")
#define ERR_ALREADYREGISTRED(client) (":server 462 " + std::string(client) + " :You may not reregister\r\n")
#define ERR_PASSWDMISMATCH(client) (":server 464 " + std::string(client) + " :Password incorrect\r\n")
#define ERR_NOTREGISTERED(client) (":server 451 " + std::string(client) + " :You have not registered\r\n")
#define ERR_NOSUCHCHANNEL(client, chan) (":server 403 " + std::string(client) + " " + std::string(chan) + " :No such channel\r\n")
#define ERR_BADCHANNELKEY(client, chan) (":server 475 " + std::string(client) + " " + std::string(chan) + " :Cannot join channel (+k)\r\n")
#define ERR_CHANNELISFULL(client, chan) (":server 471 " + std::string(client) + " " + std::string(chan) + " :Cannot join channel (+l)\r\n")
#define ERR_NOTEXTTOSEND(client) (":server 412 " + std::string(client) + " :No text to send\r\n")
#define ERR_NOSUCHNICK(client, nick) (":server 401 " + std::string(client) + " " + std::string(nick) + " :No such nick/channel\r\n")
#define ERR_CANNOTSENDTOCHAN(client, chan) (":server 404 " + std::string(client) + " " + std::string(chan) + " :Cannot send to channel\r\n")

CommandHandler::CommandHandler(Server *server) : _server(server)
{
}

CommandHandler::~CommandHandler()
{
}

void CommandHandler::execute(Client *client, const std::string &command)
{
	std::cout << "Command from fd " << client->getUsername() << ": " << command << std::endl;

	// extraire le nom de la commande et les paramètres
	size_t pos = command.find(' ');
	std::string cmd = (pos != std::string::npos) ? command.substr(0, pos) : command;
	std::string params = (pos != std::string::npos) ? command.substr(pos + 1) : "";

	// tableau de commandes
	std::string commands[6] = {"PING", "PASS", "NICK", "USER", "JOIN", "PRIVMSG"};

	// tableau des pointeurs de fonctions membres
	void (CommandHandler::*handlers[6])(Client *, const std::string &) = {
		&CommandHandler::handlePing,
		&CommandHandler::handlePass,
		&CommandHandler::handleNick,
		&CommandHandler::handleUser,
		&CommandHandler::handleJoin,
		&CommandHandler::handlePrivmsg
	};

	for (int i = 0; i < 6; i++)
	{
		if (cmd == commands[i])
		{
			(this->*handlers[i])(client, params);
			return;
		}
	}

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

void CommandHandler::handleJoin(Client *client, const std::string &params)
{
	if (!client->isRegistered())
	{
		client->sendMessage(ERR_NOTREGISTERED(client->getNickname()));
		return;
	}

	if (params.empty())
	{
		client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "JOIN"));
		return;
	}

	// parser JOIN #channel [key]
	size_t space = params.find(' ');
	std::string channelName = params.substr(0, space);
	std::string key = (space != std::string::npos) ? params.substr(space + 1) : "";

	if (channelName[0] != '#')
	{
		client->sendMessage(ERR_NOSUCHCHANNEL(client->getNickname(), channelName));
		return;
	}

	Channel *channel = _server->getChannelByName(channelName);

	if (!channel)
	{
		channel = _server->createChannel(channelName, key, client);

		std::string joinMsg = ":" + client->getNickname() + " JOIN " + channelName + "\r\n";
		client->sendMessage(joinMsg);

		client->sendMessage(":server 331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n");

		std::vector<std::string> nicks = channel->get_nicknames();
		std::string namelist;
		for (size_t i = 0; i < nicks.size(); i++)
			namelist += nicks[i] + " ";

		client->sendMessage(":server 353 " + client->getNickname() + " = " + channelName + " :" + namelist + "\r\n");
		client->sendMessage(":server 366 " + client->getNickname() + " " + channelName + " :End of /NAMES list\r\n");

		return;
	}

	if (channel->has_client(client))
		return;

	if (!channel->get_key().empty() && channel->get_key() != key)
	{
		client->sendMessage(ERR_BADCHANNELKEY(client->getNickname(), channelName));
		return;
	}

	if (channel->get_limit() > 0 && channel->get_size() >= channel->get_limit())
	{
		client->sendMessage(ERR_CHANNELISFULL(client->getNickname(), channelName));
		return;
	}

	channel->add_client(client);

	std::string joinMsg = ":" + client->getNickname() + " JOIN " + channelName + "\r\n";
	channel->broadcast(joinMsg, NULL);

	client->sendMessage(":server 331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n");

	std::vector<std::string> nicks = channel->get_nicknames();
	std::string namelist;
	for (size_t i = 0; i < nicks.size(); i++)
		namelist += nicks[i] + " ";

	client->sendMessage(":server 353 " + client->getNickname() + " = " + channelName + " :" + namelist + "\r\n");
	client->sendMessage(":server 366 " + client->getNickname() + " " + channelName + " :End of /NAMES list\r\n");
}

void CommandHandler::handlePrivmsg(Client *client, const std::string &params)
{
	if (!client->isRegistered())
	{
		client->sendMessage(ERR_NOTREGISTERED(client->getNickname()));
		return;
	}

	// parser PRIVMSG <target> :<message>
	size_t space = params.find(' ');
	if (space == std::string::npos)
	{
		client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "PRIVMSG"));
		return;
	}

	std::string target = params.substr(0, space);
	std::string message = params.substr(space + 1);

	if (message.empty() || message[0] != ':')
	{
		client->sendMessage(ERR_NOTEXTTOSEND(client->getNickname()));
		return;
	}

	message = message.substr(1);

	// 1) message vers un channel (#channel)
	if (target[0] == '#')
	{
		Channel *channel = _server->getChannelByName(target);

		if (!channel)
		{
			client->sendMessage(ERR_NOSUCHCHANNEL(client->getNickname(), target));
			return;
		}

		if (!channel->has_client(client))
		{
			client->sendMessage(ERR_CANNOTSENDTOCHAN(client->getNickname(), target));
			return;
		}

		std::string fullMsg = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
		channel->broadcast(fullMsg, client);
	}
	// 2) message privé vers un user
	else
	{
		Client *targetClient = NULL;
		std::vector<Client *> allClients = _server->getAllClients();

		for (size_t i = 0; i < allClients.size(); i++)
		{
			if (allClients[i]->getNickname() == target)
			{
				targetClient = allClients[i];
				break;
			}
		}

		if (!targetClient)
		{
			client->sendMessage(ERR_NOSUCHNICK(client->getNickname(), target));
			return;
		}

		std::string fullMsg = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
		targetClient->sendMessage(fullMsg);
	}
}

void CommandHandler::sendWelcomeMessages(Client *client)
{
	std::string nick = client->getNickname();
	client->sendMessage(":server 001 " + nick + " :Welcome to the IRC Network " + nick + "\r\n");
	client->sendMessage(":server 002 " + nick + " :Your host is server, running version 1.0\r\n");
	client->sendMessage(":server 003 " + nick + " :This server was created 2025-01-01\r\n");
	client->sendMessage(":server 004 " + nick + " server 1.0 io itkol\r\n");
}
