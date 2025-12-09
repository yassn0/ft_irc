#include "../../inc/CommandHandler.hpp"
#include "../../inc/Client.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/IRCReplies.hpp"

void CommandHandler::handleNick(Client *client, const std::string &params)
{
	if (params.empty())
		return client->sendMessage(ERR_NONICKNAMEGIVEN("*")), void();

	std::string nickname = params.substr(0, params.find(' '));

	// Vérifier si le nickname est déjà utilisé (on pourra implémenter ça plus tard avec une liste)
	// Pour l'instant, on accepte tous les nicknames non vides

	client->setNickname(nickname);
}
