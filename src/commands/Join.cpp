#include "../../inc/CommandHandler.hpp"
#include "../../inc/Client.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Channel.hpp"
#include "../../inc/IRCReplies.hpp"
#include <vector>

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
