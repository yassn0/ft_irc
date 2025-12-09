#include "../../inc/CommandHandler.hpp"
#include "../../inc/Client.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/IRCReplies.hpp"

void CommandHandler::handleNick(Client *client, const std::string &params)
{
	if (params.empty())
		return client->sendMessage(ERR_NONICKNAMEGIVEN("*")), void();

	std::string nickname = params.substr(0, params.find(' '));

	// doit être authentifié avant de choisir un nickname
	if (!client->isAuthenticated())
		return client->sendMessage(ERR_NOTREGISTERED("*")), void();

	// vérifier si le nickname est déjà utilisé
	Client* existing = findClientByNickname(nickname);
	if (existing && existing != client)
		return client->sendMessage(ERR_NICKNAMEINUSE("*", nickname)), void();

	client->setNickname(nickname);
}
