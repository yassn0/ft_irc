#include "../../inc/CommandHandler.hpp"
#include "../../inc/Client.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Channel.hpp"
#include <vector>

void CommandHandler::handleQuit(Client *client, const std::string &params)
{
	// parser QUIT [:reason]
	std::string reason = params;
	if (!reason.empty() && reason[0] == ':')
		reason = reason.substr(1);

	// construire le message QUIT
	std::string quitMsg = ":" + client->getNickname() + " QUIT :";
	if (!reason.empty())
		quitMsg += reason;
	else
		quitMsg += "Client exited";
	quitMsg += "\r\n";

	// envoyer le message QUIT au client lui-même
	client->sendMessage(quitMsg);

	// broadcaster le QUIT à tous les channels où le client est présent
	std::vector<Channel *> channels = _server->getAllChannels();
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i]->has_client(client))
		{
			channels[i]->broadcast(quitMsg, client);
			channels[i]->remove_client(client);
		}
	}

	// marquer le client pour déconnexion
	client->setShouldDisconnect(true);
}
