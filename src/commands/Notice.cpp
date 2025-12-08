#include "../../inc/CommandHandler.hpp"
#include "../../inc/Client.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Channel.hpp"
#include "../../inc/IRCReplies.hpp"

void CommandHandler::handleNotice(Client *client, const std::string &params)
{
	// NOTICE ne génère jamais de réponse d'erreur
	if (!client->isRegistered())
		return;

	// parser NOTICE <target> :<message>
	size_t space = params.find(' ');
	if (space == std::string::npos)
		return;

	std::string target = params.substr(0, space);
	std::string message = params.substr(space + 1);

	if (message.empty() || message[0] != ':')
		return;

	message = message.substr(1);

	// 1) message vers un channel (#channel)
	if (target[0] == '#')
	{
		Channel *channel = getChannel(target);
		if (!channel || !channel->has_client(client))
			return;

		std::string fullMsg = ":" + client->getNickname() + " NOTICE " + target + " :" + message + "\r\n";
		channel->broadcast(fullMsg, client);
	}
	// 2) message privé vers un user
	else
	{
		Client *targetClient = findClientByNickname(target);

		if (!targetClient)
			return;

		std::string fullMsg = ":" + client->getNickname() + " NOTICE " + target + " :" + message + "\r\n";
		targetClient->sendMessage(fullMsg);
	}
}
