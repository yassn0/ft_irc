#include "../inc/Channel.hpp"

Channel::Channel(const std::string &name, const std::string &key, Client *admin) : _name(name), _admin(admin), _k(key), _limit(0), _n(false), _topic(""), _i(false), _t(false)
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

// Topic
std::string Channel::get_topic() const
{
	return _topic;
}

void Channel::set_topic(const std::string& topic)
{
	_topic = topic;
}

// Modes
bool Channel::is_invite_only() const
{
	return _i;
}

void Channel::set_invite_only(bool flag)
{
	_i = flag;
}

bool Channel::is_topic_protected() const
{
	return _t;
}

void Channel::set_topic_protected(bool flag)
{
	_t = flag;
}

// Invitations
bool Channel::is_invited(const std::string& nickname) const
{
	for (size_t i = 0; i < _inviteList.size(); i++)
	{
		if (_inviteList[i] == nickname)
			return true;
	}
	return false;
}

void Channel::add_invite(const std::string& nickname)
{
	if (!is_invited(nickname))
		_inviteList.push_back(nickname);
}

void Channel::remove_invite(const std::string& nickname)
{
	for (size_t i = 0; i < _inviteList.size(); i++)
	{
		if (_inviteList[i] == nickname)
		{
			_inviteList.erase(_inviteList.begin() + i);
			return;
		}
	}
}

// Opérateurs
bool Channel::is_operator(Client* client) const
{
	return (client == _admin);
}

void Channel::set_operator(Client* client)
{
	_admin = client;
}