#include "../../inc/CommandHandler.hpp"
#include "../../inc/Client.hpp"

void CommandHandler::handleCap(Client *client, const std::string &params)
{
	(void)params;

	// répondre qu'il y a aucune capability disponible
	client->sendMessage(":server CAP * LS :\r\n");
}
