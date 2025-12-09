#pragma once

#include <string>

class Client;
class Server;
class Channel;

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

	void handleCap(Client* client, const std::string& params);
	void handlePing(Client* client, const std::string& params);
	void handlePass(Client* client, const std::string& params);
	void handleNick(Client* client, const std::string& params);
	void handleUser(Client* client, const std::string& params);
	void handleJoin(Client* client, const std::string& params);
	void handlePrivmsg(Client* client, const std::string& params);
	void handlePart(Client* client, const std::string& params);
	void handleQuit(Client* client, const std::string& params);
	void handleKick(Client* client, const std::string& params);
	void handleInvite(Client* client, const std::string& params);
	void handleTopic(Client* client, const std::string& params);
	void handleMode(Client* client, const std::string& params);
	void handleNotice(Client* client, const std::string& params);

	void sendWelcomeMessages(Client* client);

	// Helper functions
	Client* findClientByNickname(const std::string& nickname);
	Channel* getChannel(const std::string& channelName);
};
