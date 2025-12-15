#include "../../inc/CommandHandler.hpp"
#include "../../inc/Client.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Channel.hpp"
#include "../../inc/IRCReplies.hpp"

void CommandHandler::handleKick(Client *client, const std::string &params)
{
	if (!client->isRegistered())
		return client->sendMessage(ERR_NOTREGISTERED(client->getNickname())), void();

	// parser: KICK #channel nickname [:reason]
	size_t space1 = params.find(' ');
	if (space1 == std::string::npos)
		return client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "KICK")), void();

	std::string channelName = params.substr(0, space1);
	std::string rest = params.substr(space1 + 1);

	size_t space2 = rest.find(' ');
	std::string targetNick = rest.substr(0, space2);
	std::string reason = (space2 != std::string::npos) ? rest.substr(space2 + 1) : "No reason";

	if (!reason.empty() && reason[0] == ':')
		reason = reason.substr(1);

	Channel *channel = getChannel(channelName);
	if (!channel)
		return client->sendMessage(ERR_NOSUCHCHANNEL(client->getNickname(), channelName)), void();

	if (!channel->has_client(client))
		return client->sendMessage(ERR_NOTONCHANNEL(client->getNickname(), channelName)), void();

	if (!channel->is_operator(client))
		return client->sendMessage(ERR_CHANOPRIVSNEEDED(client->getNickname(), channelName)), void();

	Client *target = findClientByNickname(targetNick);
	if (!target)
		return client->sendMessage(ERR_NOSUCHNICK(client->getNickname(), targetNick)), void();

	if (!channel->has_client(target))
		return client->sendMessage(ERR_USERNOTINCHANNEL(client->getNickname(), targetNick, channelName)), void();

	std::string kickMsg = ":" + client->getNickname() + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
	channel->broadcast(kickMsg, NULL);
	channel->remove_client(target);
}
