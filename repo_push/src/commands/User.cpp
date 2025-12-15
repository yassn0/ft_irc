#include "../../inc/CommandHandler.hpp"
#include "../../inc/Client.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/IRCReplies.hpp"

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
