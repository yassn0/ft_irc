#pragma once

#include <string>

class Client;
class Server;

class CommandHandler
{
public:
	CommandHandler(Server* server);
	~CommandHandler();

	void execute(Client* client, const std::string& command);

private:
	CommandHandler();
	CommandHandler(const CommandHandler& other);
	CommandHandler& operator=(const CommandHandler& other);

	Server* _server;

	void handlePing(Client* client, const std::string& params);
	void handlePass(Client* client, const std::string& params);
	void handleNick(Client* client, const std::string& params);
	void handleUser(Client* client, const std::string& params);
	void handleCap(Client* client, const std::string& params);
	void handleJoin(Client* client, const std::string& params);
	void handlePrivmsg(Client* client, const std::string& params);

	void sendWelcomeMessages(Client* client);
};
