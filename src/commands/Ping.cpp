#include "../../inc/CommandHandler.hpp"
#include "../../inc/Client.hpp"
#include "../../inc/Server.hpp"

void CommandHandler::handlePing(Client *client, const std::string &params)
{
	std::string response = ":server PONG server " + params + "\r\n";
	client->sendMessage(response);
}
