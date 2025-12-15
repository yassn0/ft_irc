#include "../../inc/CommandHandler.hpp"
#include "../../inc/Client.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Channel.hpp"
#include "../../inc/IRCReplies.hpp"

void CommandHandler::handleInvite(Client *client, const std::string &params)
{
	if (!client->isRegistered())
		return client->sendMessage(ERR_NOTREGISTERED(client->getNickname())), void();

	size_t space = params.find(' ');
	if (space == std::string::npos)
		return client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "INVITE")), void();

	std::string targetNick = params.substr(0, space);
	std::string channelName = params.substr(space + 1);

	Channel *channel = getChannel(channelName);
	if (!channel)
		return client->sendMessage(ERR_NOSUCHCHANNEL(client->getNickname(), channelName)), void();

	if (!channel->has_client(client))
		return client->sendMessage(ERR_NOTONCHANNEL(client->getNickname(), channelName)), void();

	if (channel->is_invite_only() && !channel->is_operator(client))
		return client->sendMessage(ERR_CHANOPRIVSNEEDED(client->getNickname(), channelName)), void();

	Client *target = findClientByNickname(targetNick);
	if (!target)
		return client->sendMessage(ERR_NOSUCHNICK(client->getNickname(), targetNick)), void();

	if (channel->has_client(target))
		return client->sendMessage(ERR_USERONCHANNEL(client->getNickname(), targetNick, channelName)), void();

	channel->add_invite(targetNick);
	client->sendMessage(RPL_INVITING(client->getNickname(), targetNick, channelName));

	std::string inviteMsg = ":" + client->getNickname() + " INVITE " + targetNick + " " + channelName + "\r\n";
	target->sendMessage(inviteMsg);
}
