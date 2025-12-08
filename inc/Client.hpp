#pragma once

#include <string>

class Client
{
public:
	Client(int fd);
	~Client();

	// envoyer message (retourne false si échec)
	bool sendMessage(const std::string &message);

	int getFd() const;
	const std::string &getNickname() const;
	const std::string &getUsername() const;
	const std::string &getBuffer() const;
	bool isAuthenticated() const;
	bool isRegistered() const;
	bool shouldDisconnect() const;

	void setNickname(const std::string &nickname);
	void setShouldDisconnect(bool disconnect);
	void setUsername(const std::string &username);
	void setAuthenticated(bool auth);
	void setRegistered(bool reg);

	// gestion du buffer
	void appendToBuffer(const std::string &data);
	void clearBuffer();

private:
	// interdire copie
	Client();
	Client(const Client &other);
	Client &operator=(const Client &other);

	int _fd;			   // File descriptor du socket
	std::string _nickname; // IRC nickname (ex: "alice")
	std::string _username; // IRC username (ex: "alice")
	std::string _buffer;   // Buffer pour messages partiels
	bool _authenticated;   // A envoyePASS avec bon password
	bool _registered;	   // A envoye NICK + USER
	bool _shouldDisconnect; // Le client doit être déconnecté
};
