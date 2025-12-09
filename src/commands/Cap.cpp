#include "../../inc/CommandHandler.hpp"
#include "../../inc/Client.hpp"

void CommandHandler::handleCap(Client *client, const std::string &params)
{
	// CAP LS : lister les capabilities disponibles
	// CAP REQ : demander des capabilities
	// CAP END : finir la négociation
	//
	// Pour ce projet, on n'implémente aucune capability
	// On répond juste pour que les clients modernes fonctionnent

	(void)params; // pas utilisé

	// répondre que nous n'avons aucune capability disponible
	client->sendMessage(":server CAP * LS :\r\n");
}
