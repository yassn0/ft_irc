#include "../../inc/CommandHandler.hpp"
#include "../../inc/Client.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Channel.hpp"
#include "../../inc/IRCReplies.hpp"
#include <cstdlib>

void CommandHandler::handleMode(Client *client, const std::string &params)
{
	if (!client->isRegistered())
		return client->sendMessage(ERR_NOTREGISTERED(client->getNickname())), void();

	// parser: MODE #channel [+/-mode] [params]
	size_t space = params.find(' ');
	if (space == std::string::npos)
		return client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "MODE")), void();

	std::string channelName = params.substr(0, space);
	std::string modeString = params.substr(space + 1);

	// vérifier que le channel existe
	Channel *channel = getChannel(channelName);
	if (!channel)
		return client->sendMessage(ERR_NOSUCHCHANNEL(client->getNickname(), channelName)), void();

	// vérifier que le client est opérateur
	if (!channel->is_operator(client))
		return client->sendMessage(ERR_CHANOPRIVSNEEDED(client->getNickname(), channelName)), void();

	// parser le mode et les paramètres
	size_t spaceInMode = modeString.find(' ');
	std::string modes = (spaceInMode != std::string::npos) ? modeString.substr(0, spaceInMode) : modeString;
	std::string modeParams = (spaceInMode != std::string::npos) ? modeString.substr(spaceInMode + 1) : "";

	if (modes.empty() || (modes[0] != '+' && modes[0] != '-'))
		return client->sendMessage(ERR_UNKNOWNMODE(client->getNickname(), modes)), void();

	bool adding = (modes[0] == '+');
	std::string appliedModes;
	std::string appliedParams;

	for (size_t i = 1; i < modes.length(); i++)
	{
		char mode = modes[i];

		if (mode == 'i') // invite-only
		{
			channel->set_invite_only(adding);
			appliedModes += mode;
		}
		else if (mode == 't') // topic protected
		{
			channel->set_topic_protected(adding);
			appliedModes += mode;
		}
		else if (mode == 'k') // channel key
		{
			if (adding)
			{
				// +k: besoin d'un paramètre (le mot de passe)
				if (modeParams.empty())
				{
					client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "MODE"));
					continue;
				}

				// extraire le premier paramètre
				size_t paramSpace = modeParams.find(' ');
				std::string key = (paramSpace != std::string::npos) ? modeParams.substr(0, paramSpace) : modeParams;
				modeParams = (paramSpace != std::string::npos) ? modeParams.substr(paramSpace + 1) : "";

				// définir/changer la clé du channel
				channel->set_key(key);
				appliedModes += mode;
				appliedParams += " " + key;
			}
			else
			{
				// -k: supprimer la clé
				channel->set_key("");
				appliedModes += mode;
			}
		}
		else if (mode == 'o') // operator privilege
		{
			// besoin d'un paramètre (le nickname)
			if (modeParams.empty())
			{
				client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "MODE"));
				continue;
			}

			// extraire le premier paramètre
			size_t paramSpace = modeParams.find(' ');
			std::string targetNick = (paramSpace != std::string::npos) ? modeParams.substr(0, paramSpace) : modeParams;
			modeParams = (paramSpace != std::string::npos) ? modeParams.substr(paramSpace + 1) : "";

			// trouver la cible
			Client *target = findClientByNickname(targetNick);

			if (!target || !channel->has_client(target))
			{
				client->sendMessage(ERR_USERNOTINCHANNEL(client->getNickname(), targetNick, channelName));
				continue;
			}

			if (adding)
			{
				// +o: donner privilège opérateur (transférer l'admin)
				channel->set_operator(target);
				appliedModes += mode;
				appliedParams += " " + targetNick;
			}
			else
			{
				// -o: impossible de retirer le seul opérateur
				client->sendMessage(ERR_CHANOPRIVSNEEDED(client->getNickname(), channelName));
				continue;
			}
		}
		else if (mode == 'l') // user limit
		{
			if (adding)
			{
				// +l: besoin d'un paramètre (le nombre)
				if (modeParams.empty())
				{
					client->sendMessage(ERR_NEEDMOREPARAMS(client->getNickname(), "MODE"));
					continue;
				}

				// extraire le premier paramètre
				size_t paramSpace = modeParams.find(' ');
				std::string limitStr = (paramSpace != std::string::npos) ? modeParams.substr(0, paramSpace) : modeParams;
				modeParams = (paramSpace != std::string::npos) ? modeParams.substr(paramSpace + 1) : "";

				// convertir en nombre
				size_t limit = atoi(limitStr.c_str());
				channel->set_limit(limit);
				appliedModes += mode;
				appliedParams += " " + limitStr;
			}
			else
			{
				// -l: supprimer la limite
				channel->set_limit(0);
				appliedModes += mode;
			}
		}
		else
		{
			// mode inconnu
			std::string unknownMode(1, mode);
			client->sendMessage(ERR_UNKNOWNMODE(client->getNickname(), unknownMode));
		}
	}

	// broadcaster le changement si des modes ont été appliqués
	if (!appliedModes.empty())
	{
		std::string prefix = client->getNickname();
		if (!client->getUsername().empty())
			prefix += "!" + client->getUsername() + "@localhost";

		std::string modeMsg = ":" + prefix + " MODE " + channelName + " ";
		modeMsg += (adding ? "+" : "-") + appliedModes + appliedParams + "\r\n";
		channel->broadcast(modeMsg, NULL);
	}
}
