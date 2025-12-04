#include "../inc/Client.hpp"
#include <sys/socket.h>
#include <unistd.h>

Client::Client(int fd) : _fd(fd), _nickname(""), _username(""), _buffer(""), _authenticated(false), _registered(false)
{
}

Client::~Client()
{
}

bool Client::sendMessage(const std::string &message)
{
	ssize_t bytes_sent = send(_fd, message.c_str(), message.length(), 0);
	if (bytes_sent < 0 || (size_t)bytes_sent != message.length())
		return false;
	return true;
}

// Getters
int Client::getFd() const
{
	return _fd;
}

const std::string &Client::getNickname() const
{
	return _nickname;
}

const std::string &Client::getUsername() const
{
	return _username;
}

const std::string &Client::getBuffer() const
{
	return _buffer;
}

bool Client::isAuthenticated() const
{
	return _authenticated;
}

bool Client::isRegistered() const
{
	return _registered;
}

// Setters
void Client::setNickname(const std::string &nickname)
{
	_nickname = nickname;
}

void Client::setUsername(const std::string &username)
{
	_username = username;
}

void Client::setAuthenticated(bool auth)
{
	_authenticated = auth;
}

void Client::setRegistered(bool reg)
{
	_registered = reg;
}

void Client::appendToBuffer(const std::string &data)
{
	_buffer += data;
}

void Client::clearBuffer()
{
	_buffer.clear();
}
