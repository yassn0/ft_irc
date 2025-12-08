#include "../inc/Channel.hpp"

Channel::Channel(const std::string &name, const std::string &key, Client *admin) : _name(name), _admin(admin), _k(key), _limit(0), _n(false)
{
}

Channel::~Channel()
{
}

// getters
std::string Channel::get_name() const
{
	return _name;
}
Client *Channel::get_admin() const
{
	return _admin;
}

std::string Channel::get_key() const
{
	return _k;
}
size_t Channel::get_limit() const
{
	return _limit;
}
bool Channel::ext_msg() const
{
	return _n;
}

size_t Channel::get_size() const
{
	return _clients.size();
}

std::vector<std::string> Channel::get_nicknames()
{
	std::vector<std::string> nicknames;

	for (size_t i = 0; i < _clients.size(); i++)
	{
		Client *client = _clients[i];

		std::string nick = (client == _admin ? "@" : "") + client->getNickname();
		nicknames.push_back(nick);
	}

	return nicknames;
}

// setters
void Channel::set_key(std::string key)
{
	_k = key;
}
void Channel::set_limit(size_t limit)
{
	_limit = limit;
}
void Channel::set_ext_msg(bool flag)
{
	_n = flag;
}

// channels actions
void Channel::add_client(Client *client)
{
	_clients.push_back(client);
}

void Channel::remove_client(Client *client)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i] == client)
		{
			_clients.erase(_clients.begin() + i);
			return;
		}
	}
}

void Channel::broadcast(const std::string &message, Client *exclude)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i] != exclude)
			_clients[i]->sendMessage(message);
	}
}

bool Channel::has_client(Client *client) const
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i] == client)
			return true;
	}
	return false;
}