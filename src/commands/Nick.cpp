#include "../../inc/CommandHandler.hpp"
#include "../../inc/Client.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Channel.hpp"
#include "../../inc/IRCReplies.hpp"

void CommandHandler::handleNick(Client *client, const std::string &params)
{
	if (params.empty())
		return client->sendMessage(ERR_NONICKNAMEGIVEN("*")), void();

	std::string nickname = params.substr(0, params.find(' '));

	// vérifier si le nickname est déjà utilisé
	Client* existing = findClientByNickname(nickname);
	if (existing && existing != client)
		return client->sendMessage(ERR_NICKNAMEINUSE("*", nickname)), void();

	std::string oldnick = client->getNickname();
	client->setNickname(nickname);

	// confirmer le changement de nickname au client
	if (!oldnick.empty())
	{
		// broadcaster le changement aux channels (format: :nick!user@host NICK :newnick)
		std::string prefix = oldnick;
		if (!client->getUsername().empty())
			prefix += "!" + client->getUsername() + "@localhost";

		std::string nickMsg = ":" + prefix + " NICK :" + nickname + "\r\n";
		client->sendMessage(nickMsg);

		// broadcaster aux autres membres des channels
		std::vector<Channel*> channels = _server->getAllChannels();
		for (size_t i = 0; i < channels.size(); i++)
		{
			if (channels[i]->has_client(client))
				channels[i]->broadcast(nickMsg, client);
		}
	}
}
