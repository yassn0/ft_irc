#include "../../inc/CommandHandler.hpp"
#include "../../inc/Client.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Channel.hpp"
#include "../../inc/IRCReplies.hpp"

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

	// construire le message PART avec le préfixe complet
	std::string prefix = client->getNickname();
	if (!client->getUsername().empty())
		prefix += "!" + client->getUsername() + "@localhost";

	std::string partMsg = ":" + prefix + " PART " + channelName;
	if (!reason.empty())
		partMsg += " :" + reason;
	partMsg += "\r\n";

	// broadcaster à tous les membres du channel (y compris celui qui part)
	channel->broadcast(partMsg, NULL);

	// si le client qui part est opérateur et qu'il reste d'autres membres
	if (channel->is_operator(client) && channel->get_size() > 1)
	{
		// donner le statut d'opérateur au premier autre membre
		std::vector<std::string> nicks = channel->get_nicknames();
		for (size_t i = 0; i < nicks.size(); i++)
		{
			Client* nextOp = findClientByNickname(nicks[i]);
			if (nextOp && nextOp != client)
			{
				channel->set_operator(nextOp);
				break;
			}
		}
	}

	// retirer le client du channel
	channel->remove_client(client);

	// supprimer le channel s'il est vide
	if (channel->get_size() == 0)
		_server->removeChannel(channel);
}
