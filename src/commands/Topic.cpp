#include "../../inc/CommandHandler.hpp"
#include "../../inc/Client.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Channel.hpp"
#include "../../inc/IRCReplies.hpp"

void CommandHandler::handleTopic(Client *client, const std::string &params)
{
	if (!client->isRegistered())
		return client->sendMessage(ERR_NOTREGISTERED(client->getNickname())), void();

	// parser: TOPIC #channel [:new topic]
	size_t space = params.find(' ');
	std::string channelName = (space != std::string::npos) ? params.substr(0, space) : params;
	std::string newTopic = (space != std::string::npos) ? params.substr(space + 1) : "";

	// vérifier que le channel existe
	Channel *channel = getChannel(channelName);
	if (!channel)
		return client->sendMessage(ERR_NOSUCHCHANNEL(client->getNickname(), channelName)), void();

	// vérifier que le client est dans le channel
	if (!channel->has_client(client))
		return client->sendMessage(ERR_NOTONCHANNEL(client->getNickname(), channelName)), void();

	// si pas de nouveau topic, afficher le topic actuel
	if (newTopic.empty())
	{
		std::string topic = channel->get_topic();
		if (topic.empty())
			client->sendMessage(":server 331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n");
		else
			client->sendMessage(RPL_TOPIC(client->getNickname(), channelName, topic));
		return;
	}

	// enlever le : du début du topic si présent
	if (newTopic[0] == ':')
		newTopic = newTopic.substr(1);

	// si channel en mode +t (topic protected), vérifier que le client est opérateur
	if (channel->is_topic_protected() && !channel->is_operator(client))
		return client->sendMessage(ERR_CHANOPRIVSNEEDED(client->getNickname(), channelName)), void();

	// changer le topic
	channel->set_topic(newTopic);

	// broadcaster le changement à tous les membres
	std::string topicMsg = ":" + client->getNickname() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
	channel->broadcast(topicMsg, NULL);
}
