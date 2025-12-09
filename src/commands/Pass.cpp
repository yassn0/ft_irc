#include "../../inc/CommandHandler.hpp"
#include "../../inc/Client.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/IRCReplies.hpp"

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
