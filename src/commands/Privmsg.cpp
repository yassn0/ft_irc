#include "../../inc/CommandHandler.hpp"
#include "../../inc/Client.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Channel.hpp"
#include "../../inc/IRCReplies.hpp"

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

	std::string prefix = client->getNickname();
	if (!client->getUsername().empty())
		prefix += "!" + client->getUsername() + "@localhost";

	// 1) message vers un channel (#channel)
	if (target[0] == '#')
	{
		Channel *channel = _server->getChannelByName(target);

		if (!channel)
			return client->sendMessage(ERR_NOSUCHCHANNEL(client->getNickname(), target)), void();

		if (!channel->has_client(client))
			return client->sendMessage(ERR_CANNOTSENDTOCHAN(client->getNickname(), target)), void();

		std::string fullMsg = ":" + prefix + " PRIVMSG " + target + " :" + message + "\r\n";
		channel->broadcast(fullMsg, client);
	}
	// 2) message privé vers un user
	else
	{
		Client *targetClient = findClientByNickname(target);

		if (!targetClient)
			return client->sendMessage(ERR_NOSUCHNICK(client->getNickname(), target)), void();

		std::string fullMsg = ":" + prefix + " PRIVMSG " + target + " :" + message + "\r\n";
		targetClient->sendMessage(fullMsg);
	}
}
