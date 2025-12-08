#include "../inc/CommandHandler.hpp"
#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"
#include <iostream>
#include <cstdlib>

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
#define ERR_NOTONCHANNEL(client, chan) (":server 442 " + std::string(client) + " " + std::string(chan) + " :You're not on that channel\r\n")
#define ERR_USERNOTINCHANNEL(client, nick, chan) (":server 441 " + std::string(client) + " " + std::string(nick) + " " + std::string(chan) + " :They aren't on that channel\r\n")
#define ERR_USERONCHANNEL(client, nick, chan) (":server 443 " + std::string(client) + " " + std::string(nick) + " " + std::string(chan) + " :is already on channel\r\n")
#define ERR_KEYSET(client, chan) (":server 467 " + std::string(client) + " " + std::string(chan) + " :Channel key already set\r\n")
#define ERR_UNKNOWNMODE(client, mode) (":server 472 " + std::string(client) + " " + std::string(mode) + " :is unknown mode char to me\r\n")
#define ERR_INVITEONLYCHAN(client, chan) (":server 473 " + std::string(client) + " " + std::string(chan) + " :Cannot join channel (+i)\r\n")
#define ERR_CHANOPRIVSNEEDED(client, chan) (":server 482 " + std::string(client) + " " + std::string(chan) + " :You're not channel operator\r\n")
#define RPL_TOPIC(client, chan, topic) (":server 332 " + std::string(client) + " " + std::string(chan) + " :" + std::string(topic) + "\r\n")
#define RPL_INVITING(client, nick, chan) (":server 341 " + std::string(client) + " " + std::string(nick) + " " + std::string(chan) + "\r\n")

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
	std::string commands[13] = {"PING", "PASS", "NICK", "USER", "JOIN", "PRIVMSG", "PART", "QUIT", "KICK", "INVITE", "TOPIC", "MODE", "NOTICE"};

	// tableau des pointeurs de fonctions membres
	void (CommandHandler::*handlers[13])(Client *, const std::string &) = {
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

	for (int i = 0; i < 13; i++)
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
		return client->sendMessage(ERR_NEEDMOREPARAMS("*", "PASS")), void();

	if (client->isRegistered())
		return client->sendMessage(ERR_ALREADYREGISTRED(client->getNickname())), void();

	if (params == _server->getPassword())
		client->setAuthenticated(true);
	else
		client->sendMessage(ERR_PASSWDMISMATCH("*"));
}

void CommandHandler::handleNick(Client *client, const std::string &params)
{
	if (params.empty())
		return client->sendMessage(ERR_NONICKNAMEGIVEN("*")), void();

	std::string nickname = params.substr(0, params.find(' '));

	// Vérifier si le nickname est déjà utilisé (on pourra implémenter ça plus tard avec une liste)
	// Pour l'instant, on accepte tous les nicknames non vides

	client->setNickname(nickname);
}

void CommandHandler::handleUser(Client *client, const std::string &params)
{
	if (params.empty())
		return client->sendMessage(ERR_NEEDMOREPARAMS("*", "USER")), void();

	if (client->isRegistered())
		return client->sendMessage(ERR_ALREADYREGISTRED(client->getNickname())), void();

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
		return client->sendMessage(ERR_NOTREGISTERED(client->getNickname())), void();

	if (params.empty())
		return client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "JOIN")), void();

	// parser JOIN #channel [key]
	size_t space = params.find(' ');
	std::string channelName = params.substr(0, space);
	std::string key = (space != std::string::npos) ? params.substr(space + 1) : "";

	if (channelName[0] != '#')
		return client->sendMessage(ERR_NOSUCHCHANNEL(client->getNickname(), channelName)), void();

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

	// vérifier si le channel est en mode invite-only (+i)
	if (channel->is_invite_only() && !channel->is_invited(client->getNickname()))
		return client->sendMessage(ERR_INVITEONLYCHAN(client->getNickname(), channelName)), void();

	if (!channel->get_key().empty() && channel->get_key() != key)
		return client->sendMessage(ERR_BADCHANNELKEY(client->getNickname(), channelName)), void();

	if (channel->get_limit() > 0 && channel->get_size() >= channel->get_limit())
		return client->sendMessage(ERR_CHANNELISFULL(client->getNickname(), channelName)), void();

	channel->add_client(client);

	// retirer le client de la liste d'invitations s'il y était
	channel->remove_invite(client->getNickname());

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
		return client->sendMessage(ERR_NOTREGISTERED(client->getNickname())), void();

	// parser PRIVMSG <target> :<message>
	size_t space = params.find(' ');
	if (space == std::string::npos)
		return client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "PRIVMSG")), void();

	std::string target = params.substr(0, space);
	std::string message = params.substr(space + 1);

	if (message.empty() || message[0] != ':')
		return client->sendMessage(ERR_NOTEXTTOSEND(client->getNickname())), void();

	message = message.substr(1);

	// 1) message vers un channel (#channel)
	if (target[0] == '#')
	{
		Channel *channel = _server->getChannelByName(target);

		if (!channel)
			return client->sendMessage(ERR_NOSUCHCHANNEL(client->getNickname(), target)), void();

		if (!channel->has_client(client))
			return client->sendMessage(ERR_CANNOTSENDTOCHAN(client->getNickname(), target)), void();

		std::string fullMsg = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
		channel->broadcast(fullMsg, client);
	}
	// 2) message privé vers un user
	else
	{
		Client *targetClient = findClientByNickname(target);

		if (!targetClient)
			return client->sendMessage(ERR_NOSUCHNICK(client->getNickname(), target)), void();

		std::string fullMsg = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
		targetClient->sendMessage(fullMsg);
	}
}

void CommandHandler::handlePart(Client *client, const std::string &params)
{
	if (!client->isRegistered())
		return client->sendMessage(ERR_NOTREGISTERED(client->getNickname())), void();

	if (params.empty())
		return client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "PART")), void();

	// parser PART #channel [:reason]
	size_t space = params.find(' ');
	std::string channelName = params.substr(0, space);
	std::string reason = (space != std::string::npos) ? params.substr(space + 1) : "";

	// enlever le : au début de la raison si présent
	if (!reason.empty() && reason[0] == ':')
		reason = reason.substr(1);

	Channel *channel = _server->getChannelByName(channelName);

	if (!channel)
		return client->sendMessage(ERR_NOSUCHCHANNEL(client->getNickname(), channelName)), void();

	if (!channel->has_client(client))
		return client->sendMessage(ERR_NOTONCHANNEL(client->getNickname(), channelName)), void();

	// construire le message PART
	std::string partMsg = ":" + client->getNickname() + " PART " + channelName;
	if (!reason.empty())
		partMsg += " :" + reason;
	partMsg += "\r\n";

	// broadcaster à tous les membres du channel (y compris celui qui part)
	channel->broadcast(partMsg, NULL);

	// retirer le client du channel
	channel->remove_client(client);

	// TODO: si le channel est vide, le supprimer (on peut implémenter ça plus tard)
}

void CommandHandler::handleQuit(Client *client, const std::string &params)
{
	// parser QUIT [:reason]
	std::string reason = params;
	if (!reason.empty() && reason[0] == ':')
		reason = reason.substr(1);

	// construire le message QUIT
	std::string quitMsg = ":" + client->getNickname() + " QUIT :";
	if (!reason.empty())
		quitMsg += reason;
	else
		quitMsg += "Client exited";
	quitMsg += "\r\n";

	// envoyer le message QUIT au client lui-même
	client->sendMessage(quitMsg);

	// broadcaster le QUIT à tous les channels où le client est présent
	std::vector<Channel *> channels = _server->getAllChannels();
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i]->has_client(client))
		{
			channels[i]->broadcast(quitMsg, client);
			channels[i]->remove_client(client);
		}
	}

	// marquer le client pour déconnexion
	client->setShouldDisconnect(true);
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
