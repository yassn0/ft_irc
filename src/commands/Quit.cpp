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

	// construire le message QUIT avec le préfixe complet
	std::string prefix = client->getNickname();
	if (!client->getUsername().empty())
		prefix += "!" + client->getUsername() + "@localhost";

	std::string quitMsg = ":" + prefix + " QUIT :";
	if (!reason.empty())
		quitMsg += reason;
	else
		quitMsg += "Client exited";
	quitMsg += "\r\n";

	// envoyer le message QUIT au client lui-même
	client->sendMessage(quitMsg);

	// broadcaster le QUIT à tous les channels où le client est présent
	std::vector<Channel *> channels = _server->getAllChannels();
	std::vector<Channel *> emptyChannels;

	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i]->has_client(client))
		{
			channels[i]->broadcast(quitMsg, client);

			// si le client qui quitte est opérateur et qu'il reste d'autres membres
			if (channels[i]->is_operator(client) && channels[i]->get_size() > 1)
			{
				// donner le statut d'opérateur au premier autre membre
				std::vector<std::string> nicks = channels[i]->get_nicknames();
				for (size_t j = 0; j < nicks.size(); j++)
				{
					Client* nextOp = findClientByNickname(nicks[j]);
					if (nextOp && nextOp != client)
					{
						channels[i]->set_operator(nextOp);
						break;
					}
				}
			}

			channels[i]->remove_client(client);

			// marquer le channel pour suppression s'il est vide
			if (channels[i]->get_size() == 0)
				emptyChannels.push_back(channels[i]);
		}
	}

	// supprimer les channels vides
	for (size_t i = 0; i < emptyChannels.size(); i++)
		_server->removeChannel(emptyChannels[i]);

	// marquer le client pour déconnexion
	client->setShouldDisconnect(true);
}
